#define _XOPEN_SOURCE 500
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "libmss.h"

// Some Windows stuff
#ifndef FOPEN_EXTRA_FLAGS
#define FOPEN_EXTRA_FLAGS ""
#endif

#define MULTIPART_CHUNK_SIZE (5<<20) //must larger than or equal to 5MB

static S3Protocol protocolG = S3ProtocolHTTP;
static S3UriStyle uriStyleG = S3UriStylePath;

static const char *hostNameG = "mtmss.com";
static const char *accessKeyIdG = "d9f1ead3e90c42af89841bcfa78fe628";
static const char *secretAccessKeyG = "474b801062b34061aaf4f9c3af0ae4df";

static int statusG = 0;
static char errorDetailsG[4096] = { 0 };

typedef struct growbuffer
{
  // The total number of bytes, and the start byte
  int size;
  // The start byte
  int start;
  // The blocks
  char data[64 * 1024];
  struct growbuffer *prev, *next;
} growbuffer;

typedef struct UploadManager{
  //used for initial multipart
  char * upload_id;

  //used for upload part object
  char **etags;
  int next_etags_pos;

  //used for commit Upload
  growbuffer *gb;
  int remaining;
} UploadManager;

// returns nonzero on success, zero on out of memory
static int growbuffer_append(growbuffer **gb, const char *data, int dataLen)
{
  int toCopy = 0 ;
  while (dataLen) {
    growbuffer *buf = *gb ? (*gb)->prev : 0;
    if (!buf || (buf->size == sizeof(buf->data))) {
      buf = (growbuffer *) malloc(sizeof(growbuffer));
      if (!buf) {
        return 0;
      }
      buf->size = 0;
      buf->start = 0;
      if (*gb && (*gb)->prev) {
        buf->prev = (*gb)->prev;
        buf->next = *gb;
        (*gb)->prev->next = buf;
        (*gb)->prev = buf;
      }
      else {
        buf->prev = buf->next = buf;
        *gb = buf;
      }
    }

    toCopy = (sizeof(buf->data) - buf->size);
    if (toCopy > dataLen) {
      toCopy = dataLen;
    }

    memcpy(&(buf->data[buf->size]), data, toCopy);

    buf->size += toCopy, data += toCopy, dataLen -= toCopy;
  }

  return toCopy;
}

static void growbuffer_read(growbuffer **gb, int amt, int *amtReturn, 
    char *buffer)
{
  *amtReturn = 0;

  growbuffer *buf = *gb;

  if (!buf) {
    return;
  }

  *amtReturn = (buf->size > amt) ? amt : buf->size;

  memcpy(buffer, &(buf->data[buf->start]), *amtReturn);

  buf->start += *amtReturn, buf->size -= *amtReturn;

  if (buf->size == 0) {
    if (buf->next == buf) {
      *gb = 0;
    }
    else {
      *gb = buf->next;
      buf->prev->next = buf->next;
      buf->next->prev = buf->prev;
    }
    free(buf);
  }
}

static void growbuffer_destroy(growbuffer *gb)
{
  growbuffer *start = gb;

  while (gb) {
    growbuffer *next = gb->next;
    free(gb);
    gb = (next == start) ? 0 : next;
  }
}


static void S3_init()
{
  S3Status status;

  if ((status = S3_initialize("s3", S3_INIT_ALL, hostNameG))
      != S3StatusOK) {
    fprintf(stderr, "Failed to initialize libs3: %s\n", 
        S3_get_status_name(status));
    exit(-1);
  }
}

static void printError()
{
  if (statusG < S3StatusErrorAccessDenied) {
    fprintf(stderr, "\nERROR: %s\n", S3_get_status_name(statusG));
  }
  else {
    fprintf(stderr, "\nERROR: %s\n", S3_get_status_name(statusG));
    fprintf(stderr, "%s\n", errorDetailsG);
  }
}

// This callback does the same thing for every request type: prints out the
// properties if the user has requested them to be so
static S3Status responsePropertiesCallback(const S3ResponseProperties *properties, void *callbackData)
{
  (void) callbackData;

#define print_nonnull(name, field)                                 \
  do {                                                           \
    if (properties-> field) {                                  \
      printf("%s: %s\n", name, properties-> field);          \
    }                                                          \
  } while (0)

  print_nonnull("Content-Type", contentType);
  print_nonnull("Request-Id", requestId);
  print_nonnull("Request-Id-2", requestId2);
  if (properties->contentLength > 0) {
    printf("Content-Length: %lld\n", 
        (unsigned long long) properties->contentLength);
  }
  print_nonnull("Server", server);
  print_nonnull("ETag", eTag);
  if (properties->lastModified > 0) {
    char timebuf[256];
    time_t t = (time_t) properties->lastModified;
    // gmtime is not thread-safe but we don't care here.
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&t));
    printf("Last-Modified: %s\n", timebuf);
  }
  int i;
  for (i = 0; i < properties->metaDataCount; i++) {
    printf("x-amz-meta-%s: %s\n", properties->metaData[i].name,
        properties->metaData[i].value);
  }
  if (properties->usesServerSideEncryption) {
    printf("UsesServerSideEncryption: true\n");
  }

  return S3StatusOK;
}


// This callback does the same thing for every request type: saves the status
// and error stuff in global variables
static void responseCompleteCallback(S3Status status,
    const S3ErrorDetails *error, 
    void *callbackData)
{
  (void) callbackData;

  statusG = status;
  // Compose the error details message now, although we might not use it.
  // Can't just save a pointer to [error] since it's not guaranteed to last
  // beyond this callback
  int len = 0;
  if (error && error->message) {
    len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
        "  Message: %s\n", error->message);
  }
  if (error && error->resource) {
    len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
        "  Resource: %s\n", error->resource);
  }
  if (error && error->furtherDetails) {
    len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
        "  Further Details: %s\n", error->furtherDetails);
  }
  if (error && error->extraDetailsCount) {
    len += snprintf(&(errorDetailsG[len]), sizeof(errorDetailsG) - len,
        "%s", "  Extra Details:\n");
    int i;
    for (i = 0; i < error->extraDetailsCount; i++) {
      len += snprintf(&(errorDetailsG[len]), 
          sizeof(errorDetailsG) - len, "    %s: %s\n", 
          error->extraDetails[i].name,
          error->extraDetails[i].value);
    }
  }
}

typedef struct put_object_callback_data
{
  FILE *infile;
  uint64_t contentLength, originalContentLength;
} put_object_callback_data;

static int putObjectDataCallback(int bufferSize, char *buffer,
    void *callbackData)
{
  put_object_callback_data *data = 
    (put_object_callback_data *) callbackData;

  int ret = 0;

  if (data->contentLength) {
    int toRead = ((data->contentLength > (unsigned) bufferSize) ?
        (unsigned) bufferSize : data->contentLength);
    if (data->infile) {
      ret = fread(buffer, 1, toRead, data->infile);
    }
  }

  data->contentLength -= ret;

  if (data->contentLength) {
    // Avoid a weird bug in MingW, which won't print the second integer
    // value properly when it's in the same call, so print separately
    printf("%llu bytes remaining ", 
        (unsigned long long) data->contentLength);
    printf("(%d%% complete) ...\n",
        (int) (((data->originalContentLength - 
              data->contentLength) * 100) /
          data->originalContentLength));
  }

  return ret;
}

typedef struct MultipartPartData {
  put_object_callback_data put_object_data;
  int seq;
  UploadManager * manager;
} MultipartPartData;

static S3Status initial_multipart_callback(const char * upload_id, void * callbackData) {
  UploadManager *manager = (UploadManager *) callbackData;
  manager->upload_id = strdup(upload_id);
  return S3StatusOK;
}

static S3Status MultipartResponseProperiesCallback(const S3ResponseProperties *properties, void *callbackData) {

  responsePropertiesCallback(properties, callbackData);
  MultipartPartData * data = (MultipartPartData*) callbackData;
  int seq = data->seq;
  const char *etag = properties->eTag;
  data->manager->etags[seq - 1] = strdup(etag);
  data->manager->next_etags_pos = seq;
  return S3StatusOK;
}

static int multipartPutXmlCallback(int bufferSize, char *buffer, void *callbackData) {
  UploadManager * manager = (UploadManager*)callbackData;
  int ret = 0;
  if (manager->remaining) {
    int toRead = ((manager->remaining > bufferSize) ?
        bufferSize : manager->remaining);
    growbuffer_read(&(manager->gb), toRead, &ret, buffer);
  }
  manager->remaining -= ret;
  return ret;
}

static void put_object(const char *filename, const char *bucketName, const char *key)
{
  uint64_t contentLength = 0;
  const char *cacheControl = 0, *contentType = 0, *md5 = 0;
  const char *contentDispositionFilename = 0, *contentEncoding = 0;
  int64_t expires = -1;
  S3CannedAcl cannedAcl = S3CannedAclPublicRead;
  int metaPropertiesCount = 0;
  S3NameValue metaProperties[S3_MAX_METADATA_COUNT];
  char useServerSideEncryption = 0;

  put_object_callback_data data;
  data.infile = -1;
  if (filename) {
    if (!contentLength) {
      struct stat statbuf;
      // Stat the file to get its length
      if (stat(filename, &statbuf) == -1) {
        fprintf(stderr, "\nERROR: Failed to stat file %s: ",
            filename);
        perror(0);
        exit(-1);
      }
      contentLength = statbuf.st_size;
    }
    // Open the file
    if (!(data.infile = fopen(filename, "r" FOPEN_EXTRA_FLAGS))) {
      fprintf(stderr, "\nERROR: Failed to open input file %s: ",
          filename);
      perror(0);
      exit(-1);
    }
  }
  data.contentLength = data.originalContentLength = contentLength;

  S3_init();

  S3BucketContext bucketContext =
  {
    0,
    bucketName,
    protocolG,
    uriStyleG,
    accessKeyIdG,
    secretAccessKeyG,
    0
  };

  S3PutProperties putProperties =
  {
    contentType,
    md5,
    cacheControl,
    contentDispositionFilename,
    contentEncoding,
    expires,
    cannedAcl,
    metaPropertiesCount,
    metaProperties,
    useServerSideEncryption
  };

  if (contentLength <= MULTIPART_CHUNK_SIZE) {
    S3PutObjectHandler putObjectHandler =
    {
      { &responsePropertiesCallback, &responseCompleteCallback },
      &putObjectDataCallback
    };

    S3_put_object(&bucketContext, key, contentLength, &putProperties, 0,
        &putObjectHandler, &data);

    if (data.infile) {
      fclose(data.infile);
      data.infile = -1;
    }

    if (statusG != S3StatusOK) {
      printError();
    }
    else if (data.contentLength) {
      fprintf(stderr, "\nERROR: Failed to read remaining %llu bytes from "
          "input\n", (unsigned long long) data.contentLength);
    }
  }
  else {
    UploadManager manager;
    manager.upload_id = 0;
    manager.gb = 0;

    //div round up
    int seq;
    int totalSeq = (contentLength + MULTIPART_CHUNK_SIZE- 1)/ MULTIPART_CHUNK_SIZE;

    MultipartPartData partData;
    int partContentLength = 0;

    S3MultipartInitialHander handler = {
      {
        &responsePropertiesCallback,
        &responseCompleteCallback
      },
      &initial_multipart_callback    
    };

    S3PutObjectHandler putObjectHandler = {
      {
        &MultipartResponseProperiesCallback,
        &responseCompleteCallback
      },
      &putObjectDataCallback
    };

    S3MultipartCommitHandler commit_handler = {
      {
        &responsePropertiesCallback,
        &responseCompleteCallback
      },
      &multipartPutXmlCallback,
      0
    };

    manager.etags = (char**)malloc(sizeof(char*) * totalSeq);
    manager.next_etags_pos = 0;

    S3_initiate_multipart(&bucketContext,key,0, &handler,0, &manager);
    if (manager.upload_id == 0 || statusG != S3StatusOK) {
      printError();
      goto clean;
    }

    for(seq = manager.next_etags_pos+1 ; seq <= totalSeq ; seq ++) {
      memset(&partData, 0, sizeof(MultipartPartData));
      partData.manager = &manager;
      partData.seq = seq;
      partData.put_object_data = data;
      partContentLength = (contentLength > MULTIPART_CHUNK_SIZE)?MULTIPART_CHUNK_SIZE:contentLength;
      printf("Sending Part Seq %d, length=%d\n", seq, partContentLength);
      partData.put_object_data.contentLength = partContentLength;
      putProperties.md5 = 0;
      S3_upload_part(&bucketContext, key, &putProperties, &putObjectHandler, seq, manager.upload_id, partContentLength,0, &partData);
      if (statusG != S3StatusOK) {
        printError();
        goto clean;
      }
      contentLength -= MULTIPART_CHUNK_SIZE;
    }

    int i;
    int size = 0;
    size += growbuffer_append(&(manager.gb), "<CompleteMultipartUpload>", strlen("<CompleteMultipartUpload>"));
    char buf[256];
    int n;
    for(i=0;i<totalSeq;i++) {
      n = snprintf(buf,256,"<Part><PartNumber>%d</PartNumber><ETag>%s</ETag></Part>",
          i + 1,manager.etags[i]);
      size += growbuffer_append(&(manager.gb), buf, n);
    }
    size += growbuffer_append(&(manager.gb), "</CompleteMultipartUpload>",strlen("</CompleteMultipartUpload>"));
    manager.remaining = size;

    S3_complete_multipart_upload(&bucketContext, key, &commit_handler, manager.upload_id, manager.remaining, 0,  &manager); 
    if (statusG != S3StatusOK) {
      printError();
      goto clean;
    }

clean:
    if(manager.upload_id)
      free(manager.upload_id);
    for(i=0;i<manager.next_etags_pos;i++) {
      free(manager.etags[i]);
    }
    growbuffer_destroy(manager.gb);
    free(manager.etags);
  }

  S3_deinitialize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static S3Status getObjectDataCallback(int bufferSize, const char *buffer,
    void *callbackData)
{
  FILE *outfile = (FILE *) callbackData;

  size_t wrote = fwrite(buffer, 1, bufferSize, outfile);

  return ((wrote < (size_t) bufferSize) ? 
      S3StatusAbortedByCallback : S3StatusOK);
}


static void get_object(const char *filename, const char *bucketName, const char *key)
{
  int64_t ifModifiedSince = -1, ifNotModifiedSince = -1;
  const char *ifMatch = 0, *ifNotMatch = 0;
  uint64_t startByte = 0, byteCount = 0;

  FILE *outfile = 0;
  if (filename) {
    // Stat the file, and if it doesn't exist, open it in w mode
    struct stat buf;
    if (stat(filename, &buf) == -1) {
      outfile = fopen(filename, "w" FOPEN_EXTRA_FLAGS);
    }
    else {
      // Open in r+ so that we don't truncate the file, just in case
      // there is an error and we write no bytes, we leave the file
      // unmodified
      outfile = fopen(filename, "r+" FOPEN_EXTRA_FLAGS);
    }

    if (!outfile) {
      fprintf(stderr, "\nERROR: Failed to open output file %s: ",
          filename);
      perror(0);
      exit(-1);
    }
  }
  else {
    outfile = stdout;
  }

  S3_init();

  S3BucketContext bucketContext =
  {
    0,
    bucketName,
    protocolG,
    uriStyleG,
    accessKeyIdG,
    secretAccessKeyG,
    0
  };

  S3GetConditions getConditions =
  {
    ifModifiedSince,
    ifNotModifiedSince,
    ifMatch,
    ifNotMatch
  };

  S3GetObjectHandler getObjectHandler =
  {
    { &responsePropertiesCallback, &responseCompleteCallback },
    &getObjectDataCallback
  };

  S3_get_object(&bucketContext, key, &getConditions, startByte,
      byteCount, 0, &getObjectHandler, outfile);

  if (stdout != outfile && NULL != outfile) {
    fclose(outfile);
    outfile = NULL;
  }

  if (statusG != S3StatusOK) {
    printError();
  }

  S3_deinitialize();
}

int main(int argc, char **argv)
{
  const char *localFile = argv[1];
  const char *bucketName = argv[2];
  const char *key = argv[3];
  put_object(localFile, bucketName, key);

  const char *localReplica = argv[4];
  get_object(localReplica, bucketName, key);
}

