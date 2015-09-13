#ifndef LIBS3_H
#define LIBS3_H

#include <stdint.h>
#include <sys/select.h>


#ifdef __cplusplus
extern "C" {
#endif

/** **************************************************************************
 * Constants
 ************************************************************************** **/

/**
 * S3_MAX_HOSTNAME_SIZE is the maximum size we allow for a host name
 **/
#define S3_MAX_HOSTNAME_SIZE               255

/**
 * This is the default hostname that is being used for the S3 requests
 **/
#define S3_DEFAULT_HOSTNAME                "mtmss.com"


/**
 * S3_MAX_BUCKET_NAME_SIZE is the maximum size of a bucket name.
 **/

#define S3_MAX_BUCKET_NAME_SIZE            255

/**
 * S3_MAX_KEY_SIZE is the maximum size of keys that Amazon S3 supports.
 **/
#define S3_MAX_KEY_SIZE                    1024


/**
 * S3_MAX_METADATA_SIZE is the maximum number of bytes allowed for
 * x-amz-meta header names and values in any request passed to Amazon S3
 **/
#define S3_MAX_METADATA_SIZE               2048


/**
 * S3_METADATA_HEADER_NAME_PREFIX is the prefix of an S3 "meta header"
 **/
#define S3_METADATA_HEADER_NAME_PREFIX     "x-amz-meta-"


/**
 * S3_MAX_METADATA_COUNT is the maximum number of x-amz-meta- headers that
 * could be included in a request to S3.  The smallest meta header is
 * "x-amz-meta-n: v".  Since S3 doesn't count the ": " against the total, the
 * smallest amount of data to count for a header would be the length of
 * "x-amz-meta-nv".
 **/
#define S3_MAX_METADATA_COUNT \
    (S3_MAX_METADATA_SIZE / (sizeof(S3_METADATA_HEADER_NAME_PREFIX "nv") - 1))


/**
 * S3_MAX_ACL_GRANT_COUNT is the maximum number of ACL grants that may be
 * set on a bucket or object at one time.  It is also the maximum number of
 * ACL grants that the XML ACL parsing routine will parse.
 **/
#define S3_MAX_ACL_GRANT_COUNT             100


/**
 * This is the maximum number of characters (including terminating \0) that
 * libs3 supports in an ACL grantee email address.
 **/
#define S3_MAX_GRANTEE_EMAIL_ADDRESS_SIZE  128


/**
 * This is the maximum number of characters (including terminating \0) that
 * libs3 supports in an ACL grantee user id.
 **/
#define S3_MAX_GRANTEE_USER_ID_SIZE        128


/**
 * This is the maximum number of characters (including terminating \0) that
 * libs3 supports in an ACL grantee user display name.
 **/
#define S3_MAX_GRANTEE_DISPLAY_NAME_SIZE   128


/**
 * This is the maximum number of characters that will be stored in the
 * return buffer for the utility function which computes an HTTP authenticated
 * query string
 **/
#define S3_MAX_AUTHENTICATED_QUERY_STRING_SIZE \
    (sizeof("https:///") + S3_MAX_HOSTNAME_SIZE + (S3_MAX_KEY_SIZE * 3) + \
     sizeof("?AWSAccessKeyId=") + 32 + sizeof("&Expires=") + 32 + \
     sizeof("&Signature=") + 28 + 1)


/**
 * This constant is used by the S3_initialize() function, to specify that
 * the winsock library should be initialized by libs3; only relevent on 
 * Microsoft Windows platforms.
 **/
#define S3_INIT_WINSOCK                    1


/**
 * This convenience constant is used by the S3_initialize() function to
 * indicate that all libraries required by libs3 should be initialized.
 **/
#define S3_INIT_ALL                        (S3_INIT_WINSOCK)


/** **************************************************************************
 * Enumerations
 ************************************************************************** **/

/**
 * S3Status is a status code as returned by a libs3 function.  The meaning of
 * each status code is defined in the comments for each function which returns
 * that status.
 **/
typedef enum
{
    S3StatusOK                                              ,

    /**
     * Errors that prevent the S3 request from being issued or response from
     * being read
     **/
    S3StatusInternalError                                   ,
    S3StatusOutOfMemory                                     ,
    S3StatusInterrupted                                     ,
    S3StatusInvalidBucketNameTooLong                        ,
    S3StatusInvalidBucketNameFirstCharacter                 ,
    S3StatusInvalidBucketNameCharacter                      ,
    S3StatusInvalidBucketNameCharacterSequence              ,
    S3StatusInvalidBucketNameTooShort                       ,
    S3StatusInvalidBucketNameDotQuadNotation                ,
    S3StatusQueryParamsTooLong                              ,
    S3StatusFailedToInitializeRequest                       ,
    S3StatusMetaDataHeadersTooLong                          ,
    S3StatusBadMetaData                                     ,
    S3StatusBadContentType                                  ,
    S3StatusContentTypeTooLong                              ,
    S3StatusBadMD5                                          ,
    S3StatusMD5TooLong                                      ,
    S3StatusBadCacheControl                                 ,
    S3StatusCacheControlTooLong                             ,
    S3StatusBadContentDispositionFilename                   ,
    S3StatusContentDispositionFilenameTooLong               ,
    S3StatusBadContentEncoding                              ,
    S3StatusContentEncodingTooLong                          ,
    S3StatusBadIfMatchETag                                  ,
    S3StatusIfMatchETagTooLong                              ,
    S3StatusBadIfNotMatchETag                               ,
    S3StatusIfNotMatchETagTooLong                           ,
    S3StatusHeadersTooLong                                  ,
    S3StatusKeyTooLong                                      ,
    S3StatusUriTooLong                                      ,
    S3StatusXmlParseFailure                                 ,
    S3StatusEmailAddressTooLong                             ,
    S3StatusUserIdTooLong                                   ,
    S3StatusUserDisplayNameTooLong                          ,
    S3StatusGroupUriTooLong                                 ,
    S3StatusPermissionTooLong                               ,
    S3StatusTargetBucketTooLong                             ,
    S3StatusTargetPrefixTooLong                             ,
    S3StatusTooManyGrants                                   ,
    S3StatusBadGrantee                                      ,
    S3StatusBadPermission                                   ,
    S3StatusXmlDocumentTooLarge                             ,
    S3StatusNameLookupError                                 ,
    S3StatusFailedToConnect                                 ,
    S3StatusServerFailedVerification                        ,
    S3StatusConnectionFailed                                ,
    S3StatusAbortedByCallback                               ,
    
    /**
     * Errors from the S3 service
     **/
    S3StatusErrorAccessDenied                               ,
    S3StatusErrorAccountProblem                             ,
    S3StatusErrorAmbiguousGrantByEmailAddress               ,
    S3StatusErrorBadDigest                                  ,
    S3StatusErrorBucketAlreadyExists                        ,
    S3StatusErrorBucketAlreadyOwnedByYou                    ,
    S3StatusErrorBucketNotEmpty                             ,
    S3StatusErrorCredentialsNotSupported                    ,
    S3StatusErrorCrossLocationLoggingProhibited             ,
    S3StatusErrorEntityTooSmall                             ,
    S3StatusErrorEntityTooLarge                             ,
    S3StatusErrorExpiredToken                               ,
    S3StatusErrorIllegalVersioningConfigurationException    ,
    S3StatusErrorIncompleteBody                             ,
    S3StatusErrorIncorrectNumberOfFilesInPostRequest        ,
    S3StatusErrorInlineDataTooLarge                         ,
    S3StatusErrorInternalError                              ,
    S3StatusErrorInvalidAccessKeyId                         ,
    S3StatusErrorInvalidAddressingHeader                    ,
    S3StatusErrorInvalidArgument                            ,
    S3StatusErrorInvalidBucketName                          ,
    S3StatusErrorInvalidBucketState                         ,
    S3StatusErrorInvalidDigest                              ,
    S3StatusErrorInvalidLocationConstraint                  ,
    S3StatusErrorInvalidObjectState                         ,
    S3StatusErrorInvalidPart                                ,
    S3StatusErrorInvalidPartOrder                           ,
    S3StatusErrorInvalidPayer                               ,
    S3StatusErrorInvalidPolicyDocument                      ,
    S3StatusErrorInvalidRange                               ,
    S3StatusErrorInvalidRequest                             ,
    S3StatusErrorInvalidSecurity                            ,
    S3StatusErrorInvalidSOAPRequest                         ,
    S3StatusErrorInvalidStorageClass                        ,
    S3StatusErrorInvalidTargetBucketForLogging              ,
    S3StatusErrorInvalidToken                               ,
    S3StatusErrorInvalidURI                                 ,
    S3StatusErrorKeyTooLong                                 ,
    S3StatusErrorMalformedACLError                          ,
    S3StatusErrorMalformedPOSTRequest                       ,
    S3StatusErrorMalformedXML                               ,
    S3StatusErrorMaxMessageLengthExceeded                   ,
    S3StatusErrorMaxPostPreDataLengthExceededError          ,
    S3StatusErrorMetadataTooLarge                           ,
    S3StatusErrorMethodNotAllowed                           ,
    S3StatusErrorMissingAttachment                          ,
    S3StatusErrorMissingContentLength                       ,
    S3StatusErrorMissingRequestBodyError                    ,
    S3StatusErrorMissingSecurityElement                     ,
    S3StatusErrorMissingSecurityHeader                      ,
    S3StatusErrorNoLoggingStatusForKey                      ,
    S3StatusErrorNoSuchBucket                               ,
    S3StatusErrorNoSuchKey                                  ,
    S3StatusErrorNoSuchLifecycleConfiguration               ,
    S3StatusErrorNoSuchUpload                               ,
    S3StatusErrorNoSuchVersion                              ,
    S3StatusErrorNotImplemented                             ,
    S3StatusErrorNotSignedUp                                ,
    S3StatusErrorNotSuchBucketPolicy                        ,
    S3StatusErrorOperationAborted                           ,
    S3StatusErrorPermanentRedirect                          ,
    S3StatusErrorPreconditionFailed                         ,
    S3StatusErrorRedirect                                   ,
    S3StatusErrorRestoreAlreadyInProgress                   ,
    S3StatusErrorRequestIsNotMultiPartContent               ,
    S3StatusErrorRequestTimeout                             ,
    S3StatusErrorRequestTimeTooSkewed                       ,
    S3StatusErrorRequestTorrentOfBucketError                ,
    S3StatusErrorSignatureDoesNotMatch                      ,
    S3StatusErrorServiceUnavailable                         ,
    S3StatusErrorSlowDown                                   ,
    S3StatusErrorTemporaryRedirect                          ,
    S3StatusErrorTokenRefreshRequired                       ,
    S3StatusErrorTooManyBuckets                             ,
    S3StatusErrorUnexpectedContent                          ,
    S3StatusErrorUnresolvableGrantByEmailAddress            ,
    S3StatusErrorUserKeyMustBeSpecified                     ,
    S3StatusErrorUnknown                                    ,

    /**
     * The following are HTTP errors returned by S3 without enough detail to
     * distinguish any of the above S3StatusError conditions
     **/
    S3StatusHttpErrorMovedTemporarily                       ,
    S3StatusHttpErrorBadRequest                             ,
    S3StatusHttpErrorForbidden                              ,
    S3StatusHttpErrorNotFound                               ,
    S3StatusHttpErrorConflict                               ,
    S3StatusHttpErrorUnknown
} S3Status;


/**
 * S3Protocol represents a protocol that may be used for communicating a
 * request to the Amazon S3 service.
 *
 * In general, HTTPS is greatly preferred (and should be the default of any
 * application using libs3) because it protects any data being sent to or
 * from S3 using strong encryption.  However, HTTPS is much more CPU intensive
 * than HTTP, and if the caller is absolutely certain that it is OK for the
 * data to be viewable by anyone in transit, then HTTP can be used.
 **/
typedef enum
{
    S3ProtocolHTTPS                     = 0,
    S3ProtocolHTTP                      = 1
} S3Protocol;


/**
 * S3UriStyle defines the form that an Amazon S3 URI identifying a bucket or
 * object can take.  They are of these forms:
 *
 * Virtual Host: ${protocol}://${bucket}.s3.amazonaws.com/[${key}]
 * Path: ${protocol}://s3.amazonaws.com/${bucket}/[${key}]
 *
 * It is generally better to use the Virual Host URI form, because it ensures
 * that the bucket name used is compatible with normal HTTP GETs and POSTs of
 * data to/from the bucket.  However, if DNS lookups for the bucket are too
 * slow or unreliable for some reason, Path URI form may be used.
 **/
typedef enum
{
    S3UriStylePath                      = 1
} S3UriStyle;


/**
 * S3CannedAcl is an ACL that can be specified when an object is created or
 * updated.  Each canned ACL has a predefined value when expanded to a full
 * set of S3 ACL Grants.
 * Private canned ACL gives the owner FULL_CONTROL and no other permissions
 *     are issued
 * Public Read canned ACL gives the owner FULL_CONTROL and all users Read
 *     permission 
 * Public Read Write canned ACL gives the owner FULL_CONTROL and all users
 *     Read and Write permission
 * AuthenticatedRead canned ACL gives the owner FULL_CONTROL and authenticated
 *     S3 users Read permission
 **/
typedef enum
{
    S3CannedAclPrivate                  = 0, /* private */
    S3CannedAclPublicRead               = 1, /* public-read */
} S3CannedAcl;
/** **************************************************************************
 * Data Types
 ************************************************************************** **/

/**
 * An S3RequestContext manages multiple S3 requests simultaneously; see the
 * S3_XXX_request_context functions below for details
 **/
typedef struct S3RequestContext S3RequestContext;


/**
 * S3NameValue represents a single Name - Value pair, used to represent either
 * S3 metadata associated with a key, or S3 error details.
 **/
typedef struct S3NameValue
{
    /**
     * The name part of the Name - Value pair
     **/
    const char *name;

    /**
     * The value part of the Name - Value pair
     **/
    const char *value;
} S3NameValue;


/**
 * S3ResponseProperties is passed to the properties callback function which is
 * called when the complete response properties have been received.  Some of
 * the fields of this structure are optional and may not be provided in the
 * response, and some will always be provided in the response.
 **/
typedef struct S3ResponseProperties
{
    /**
     * This optional field identifies the request ID and may be used when
     * reporting problems to Amazon.
     **/
    const char *requestId;

    /**
     * This optional field identifies the request ID and may be used when
     * reporting problems to Amazon.
     **/
    const char *requestId2;

    /**
     * This optional field is the content type of the data which is returned
     * by the request.  If not provided, the default can be assumed to be
     * "binary/octet-stream".
     **/
    const char *contentType;

    /**
     * This optional field is the content length of the data which is returned
     * in the response.  A negative value means that this value was not
     * provided in the response.  A value of 0 means that there is no content
     * provided.  A positive value gives the number of bytes in the content of
     * the response.
     **/
    uint64_t contentLength;

    /**
     * This optional field names the server which serviced the request.
     **/
    const char *server;

    /**
     * This optional field provides a string identifying the unique contents
     * of the resource identified by the request, such that the contents can
     * be assumed not to be changed if the same eTag is returned at a later
     * time decribing the same resource.  This is an MD5 sum of the contents.
     **/
    const char *eTag;

    /**
     * This optional field provides the last modified time, relative to the
     * Unix epoch, of the contents.  If this value is < 0, then the last
     * modified time was not provided in the response.  If this value is >= 0,
     * then the last modified date of the contents are available as a number
     * of seconds since the UNIX epoch.
     * 
     **/
    int64_t lastModified;

    /**
     * This is the number of user-provided meta data associated with the
     * resource.
     **/
    int metaDataCount;

    /**
     * These are the meta data associated with the resource.  In each case,
     * the name will not include any S3-specific header prefixes
     * (i.e. x-amz-meta- will have been removed from the beginning), and
     * leading and trailing whitespace will have been stripped from the value.
     **/
    const S3NameValue *metaData;

    /**
     * This optional field provides an indication of whether or not
     * server-side encryption was used for the object.  This field is only
     * meaningful if the request was an object put, copy, get, or head
     * request.
     * If this value is 0, then server-side encryption is not in effect for
     * the object (or the request was one for which server-side encryption is
     * not a meaningful value); if this value is non-zero, then server-side
     * encryption is in effect for the object.
     **/
    char usesServerSideEncryption;
} S3ResponseProperties;


/**
 * A context for working with objects within a bucket.  A bucket context holds
 * all information necessary for working with a bucket, and may be used
 * repeatedly over many consecutive (or simultaneous) calls into libs3 bucket
 * operation functions.
 **/
typedef struct S3BucketContext
{
    /**
     * The name of the host to connect to when making S3 requests.  If set to
     * NULL, the default S3 hostname passed in to S3_initialize will be used.
     **/
    const char *hostName;

    /**
     * The name of the bucket to use in the bucket context
     **/
    const char *bucketName;

    /**
     * The protocol to use when accessing the bucket
     **/
    S3Protocol protocol;

    /**
     * The URI style to use for all URIs sent to Amazon S3 while working with
     * this bucket context
     **/
    S3UriStyle uriStyle;

    /**
     * The Amazon Access Key ID to use for access to the bucket
     **/
    const char *accessKeyId;

    /**
     *  The Amazon Secret Access Key to use for access to the bucket
     **/
    const char *secretAccessKey;

    /**
     *  The Amazon Security Token used to generate Temporary Security Credentials
     **/
    const char *securityToken;
} S3BucketContext;


/**
 * This is a single entry supplied to the list bucket callback by a call to
 * S3_list_bucket.  It identifies a single matching key from the list
 * operation.
 **/
typedef struct S3ListMultipartUpload
{
    /**
     * This is the next key in the list bucket results.
     **/
    const char *key;

    const char *uploadId;
    const char *initiatorId;
    const char *initiatorDisplayName;

    /**
     * This is the ID of the owner of the key; it is present only if access
     * permissions allow it to be viewed.
     **/
    const char *ownerId;

    /**
     * This is the display name of the owner of the key; it is present only if
     * access permissions allow it to be viewed.
     **/
    const char *ownerDisplayName;
    
    const char *storageClass;

    /**
     * This is the number of seconds since UNIX epoch of the last modified
     * date of the object identified by the key. 
     **/
    int64_t initiated;
    
} S3ListMultipartUpload;


typedef struct S3ListPart
{   
    const char *eTag;

    /**
     * This is the number of seconds since UNIX epoch of the last modified
     * date of the object identified by the key. 
     **/
    int64_t lastModified;
    uint64_t partNumber;
    uint64_t size;
} S3ListPart;


/**
 * S3PutProperties is the set of properties that may optionally be set by the
 * user when putting objects to S3.  Each field of this structure is optional
 * and may or may not be present.
 **/
typedef struct S3PutProperties
{
    /**
     * If present, this is the Content-Type that should be associated with the
     * object.  If not provided, S3 defaults to "binary/octet-stream".
     **/
    const char *contentType;

    /**
     * If present, this provides the MD5 signature of the contents, and is
     * used to validate the contents.  This is highly recommended by Amazon
     * but not required.  Its format is as a base64-encoded MD5 sum.
     **/
    const char *md5;

    /**
     * If present, this gives a Cache-Control header string to be supplied to
     * HTTP clients which download this
     **/
    const char *cacheControl;

    /**
     * If present, this gives the filename to save the downloaded file to,
     * whenever the object is downloaded via a web browser.  This is only
     * relevent for objects which are intended to be shared to users via web
     * browsers and which is additionally intended to be downloaded rather
     * than viewed.
     **/
    const char *contentDispositionFilename;

    /**
     * If present, this identifies the content encoding of the object.  This
     * is only applicable to encoded (usually, compressed) content, and only
     * relevent if the object is intended to be downloaded via a browser.
     **/
    const char *contentEncoding;

    /**
     * If >= 0, this gives an expiration date for the content.  This
     * information is typically only delivered to users who download the
     * content via a web browser.
     **/
    int64_t expires;

    /**
     * This identifies the "canned ACL" that should be used for this object.
     * The default (0) gives only the owner of the object access to it.
     **/
    S3CannedAcl cannedAcl;

    /**
     * This is the number of values in the metaData field.
     **/
    int metaDataCount;

    /**
     * These are the meta data to pass to S3.  In each case, the name part of
     * the Name - Value pair should not include any special S3 HTTP header
     * prefix (i.e., should be of the form 'foo', NOT 'x-amz-meta-foo').
     **/
    const S3NameValue *metaData;

    /**
     * This a boolean value indicating whether or not the object should be
     * stored by Amazon S3 using server-side encryption, wherein the data is
     * encrypted by Amazon before being stored on permanent medium.
     * Server-side encryption does not affect the data as it is sent to or
     * received by Amazon, the encryption is applied by Amazon when objects
     * are put and then de-encryption is applied when the objects are read by
     * clients.
     * If this value is 0, then server-side encryption is not used; if this
     * value is non-zero, then server-side encryption is used.  Note that the
     * encryption status of the object can be checked by ensuring that the put
     * response has the usesServerSideEncryption flag set.
     **/
    char useServerSideEncryption;
} S3PutProperties;


/**
 * S3GetConditions is used for the get_object operation, and specifies
 * conditions which the object must meet in order to be successfully returned.
 **/
typedef struct S3GetConditions
{
    /**
     * The request will be processed if the Last-Modification header of the
     * object is greater than or equal to this value, specified as a number of
     * seconds since Unix epoch.  If this value is less than zero, it will not
     * be used in the conditional.
     **/
    int64_t ifModifiedSince;

    /**
     * The request will be processed if the Last-Modification header of the
     * object is less than this value, specified as a number of seconds since
     * Unix epoch.  If this value is less than zero, it will not be used in
     * the conditional.
     **/
    int64_t ifNotModifiedSince;

    /**
     * If non-NULL, this gives an eTag header value which the object must
     * match in order to be returned.  Note that altough the eTag is simply an
     * MD5, this must be presented in the S3 eTag form, which typically
     * includes double-quotes.
     **/
    const char *ifMatchETag;

    /**
     * If non-NULL, this gives an eTag header value which the object must not
     * match in order to be returned.  Note that altough the eTag is simply an
     * MD5, this must be presented in the S3 eTag form, which typically
     * includes double-quotes.
     **/
    const char *ifNotMatchETag;
} S3GetConditions;


/**
 * S3ErrorDetails provides detailed information describing an S3 error.  This
 * is only presented when the error is an S3-generated error (i.e. one of the
 * S3StatusErrorXXX values).
 **/
typedef struct S3ErrorDetails
{
    /**
     * This is the human-readable message that Amazon supplied describing the
     * error
     **/
    const char *message;

    /**
     * This identifies the resource for which the error occurred
     **/
    const char *resource;

    /**
     * This gives human-readable further details describing the specifics of
     * this error
     **/
    const char *furtherDetails;

    /**
     * This gives the number of S3NameValue pairs present in the extraDetails
     * array
     **/
    int extraDetailsCount;

    /**
     * S3 can provide extra details in a freeform Name - Value pair format.
     * Each error can have any number of these, and this array provides these
     * additional extra details.
     **/
    S3NameValue *extraDetails;
} S3ErrorDetails;


/** **************************************************************************
 * Callback Signatures
 ************************************************************************** **/

/**
 * This callback is made whenever the response properties become available for
 * any request.
 *
 * @param properties are the properties that are available from the response
 * @param callbackData is the callback data as specified when the request
 *        was issued.
 * @return S3StatusOK to continue processing the request, anything else to
 *         immediately abort the request with a status which will be
 *         passed to the S3ResponseCompleteCallback for this request.
 *         Typically, this will return either S3StatusOK or
 *         S3StatusAbortedByCallback.
 **/
typedef S3Status (S3ResponsePropertiesCallback)
    (const S3ResponseProperties *properties, void *callbackData);


/**
 * This callback is made when the response has been completely received, or an
 * error has occurred which has prematurely aborted the request, or one of the
 * other user-supplied callbacks returned a value intended to abort the
 * request.  This callback is always made for every request, as the very last
 * callback made for that request.
 *
 * @param status gives the overall status of the response, indicating success
 *        or failure; use S3_status_is_retryable() as a simple way to detect
 *        whether or not the status indicates that the request failed but may
 *        be retried.
 * @param errorDetails if non-NULL, gives details as returned by the S3
 *        service, describing the error
 * @param callbackData is the callback data as specified when the request
 *        was issued.
 **/
typedef void (S3ResponseCompleteCallback)(S3Status status,
                                          const S3ErrorDetails *errorDetails,
                                          void *callbackData);

                                       
/**
 * This callback is made during a put object operation, to obtain the next
 * chunk of data to put to the S3 service as the contents of the object.  This
 * callback is made repeatedly, each time acquiring the next chunk of data to
 * write to the service, until a negative or 0 value is returned.
 *
 * @param bufferSize gives the maximum number of bytes that may be written
 *        into the buffer parameter by this callback
 * @param buffer gives the buffer to fill with at most bufferSize bytes of
 *        data as the next chunk of data to send to S3 as the contents of this
 *        object
 * @param callbackData is the callback data as specified when the request
 *        was issued.
 * @return < 0 to abort the request with the S3StatusAbortedByCallback, which
 *        will be pased to the response complete callback for this request, or
 *        0 to indicate the end of data, or > 0 to identify the number of
 *        bytes that were written into the buffer by this callback
 **/
typedef int (S3PutObjectDataCallback)(int bufferSize, char *buffer,
                                      void *callbackData);


/**
 * This callback is made during a get object operation, to provide the next
 * chunk of data available from the S3 service constituting the contents of
 * the object being fetched.  This callback is made repeatedly, each time
 * providing the next chunk of data read, until the complete object contents
 * have been passed through the callback in this way, or the callback
 * returns an error status.
 *
 * @param bufferSize gives the number of bytes in buffer
 * @param buffer is the data being passed into the callback
 * @param callbackData is the callback data as specified when the request
 *        was issued.
 * @return S3StatusOK to continue processing the request, anything else to
 *         immediately abort the request with a status which will be
 *         passed to the S3ResponseCompleteCallback for this request.
 *         Typically, this will return either S3StatusOK or
 *         S3StatusAbortedByCallback.
 **/
typedef S3Status (S3GetObjectDataCallback)(int bufferSize, const char *buffer,
                                           void *callbackData);
                                       


typedef S3Status (S3MultipartCommitResponseCallback)(const char * location, const char * etag, void * callbackData);



typedef S3Status (S3MultipartInitialResponseCallback)(const char * upload_id, void * callbackData);


typedef S3Status (S3ListMultipartUploadsResponseCallback)(int isTruncated,
                                        const char *nextKeyMarker,
                                        const char *nextUploadIdMarker,
                                        int uploadsCount, 
                                        const S3ListMultipartUpload *uploads,
                                        int commonPrefixesCount,
                                        const char **commonPrefixes,
                                        void *callbackData);


typedef S3Status (S3ListPartsResponseCallback)(int isTruncated,
                                        const char *nextPartNumberMarker,
                                        const char *initiatorId,
                                        const char *initiatorDisplayName,
                                        const char *ownerId,
                                        const char *ownerDisplayName,
                                        const char *storageClass,
                                        int partsCount,
                                        int lastPartNumber,
                                        const S3ListPart *parts,
                                        void *callbackData);

/** **************************************************************************
 * Callback Structures
 ************************************************************************** **/


/**
 * An S3ResponseHandler defines the callbacks which are made for any
 * request.
 **/
typedef struct S3ResponseHandler
{
    /**
     * The propertiesCallback is made when the response properties have
     * successfully been returned from S3.  This function may not be called
     * if the response properties were not successfully returned from S3.
     **/
    S3ResponsePropertiesCallback *propertiesCallback;
    
    /**
     * The completeCallback is always called for every request made to S3,
     * regardless of the outcome of the request.  It provides the status of
     * the request upon its completion, as well as extra error details in the
     * event of an S3 error.
     **/
    S3ResponseCompleteCallback *completeCallback;
} S3ResponseHandler;


/**
 * An S3PutObjectHandler defines the callbacks which are made for
 * put_object requests.
 **/
typedef struct S3PutObjectHandler
{
    /**
     * responseHandler provides the properties and complete callback
     **/
    S3ResponseHandler responseHandler;

    /**
     * The putObjectDataCallback is called to acquire data to send to S3 as
     * the contents of the put_object request.  It is made repeatedly until it
     * returns a negative number (indicating that the request should be
     * aborted), or 0 (indicating that all data has been supplied).
     **/
    S3PutObjectDataCallback *putObjectDataCallback;
} S3PutObjectHandler;


/**
 * An S3GetObjectHandler defines the callbacks which are made for
 * get_object requests.
 **/
typedef struct S3GetObjectHandler
{
    /**
     * responseHandler provides the properties and complete callback
     **/
    S3ResponseHandler responseHandler;

    /**
     * The getObjectDataCallback is called as data is read from S3 as the
     * contents of the object being read in the get_object request.  It is
     * called repeatedly until there is no more data provided in the request,
     * or until the callback returns an error status indicating that the
     * request should be aborted.
     **/
    S3GetObjectDataCallback *getObjectDataCallback;
} S3GetObjectHandler;


typedef struct S3MultipartInitialHander {
    /**
     * responseHandler provides the properties and complete callback
     **/
    S3ResponseHandler responseHandler;

    S3MultipartInitialResponseCallback *responseXmlCallback;
} S3MultipartInitialHander;

typedef struct S3MultipartCommitHandler
{
    /**
     * responseHandler provides the properties and complete callback
     **/
    S3ResponseHandler responseHandler;

    /**
     * The putObjectDataCallback is called to acquire data to send to S3 as
     * the contents of the put_object request.  It is made repeatedly until it
     * returns a negative number (indicating that the request should be
     * aborted), or 0 (indicating that all data has been supplied).
     **/
    S3PutObjectDataCallback *putObjectDataCallback;
    S3MultipartCommitResponseCallback *responseXmlCallback;
} S3MultipartCommitHandler;

typedef struct S3ListMultipartUploadsHandler
{
    /**
     * responseHandler provides the properties and complete callback
     **/
    S3ResponseHandler responseHandler;
    
    S3ListMultipartUploadsResponseCallback *responseXmlCallback;
} S3ListMultipartUploadsHandler;

typedef struct S3ListPartsHandler
{
    /**
     * responseHandler provides the properties and complete callback
     **/
    S3ResponseHandler responseHandler;
    
    S3ListPartsResponseCallback *responseXmlCallback;
} S3ListPartsHandler;

typedef struct S3AbortMultipartUploadHandler
{
    /**
     * responseHandler provides the properties and complete callback
     **/
    S3ResponseHandler responseHandler;
    
} S3AbortMultipartUploadHandler;

/** **************************************************************************
 * General Library Functions
 ************************************************************************** **/

/**
 * Initializes libs3 for use.  This function must be called before any other
 * libs3 function is called.  It may be called multiple times, with the same
 * effect as calling it once, as long as S3_deinitialize() is called an
 * equal number of times when the program has finished.  This function is NOT
 * thread-safe and must only be called by one thread at a time.
 *
 * @param userAgentInfo is a string that will be included in the User-Agent
 *        header of every request made to the S3 service.  You may provide
 *        NULL or the empty string if you don't care about this.  The value
 *        will not be copied by this function and must remain unaltered by the
 *        caller until S3_deinitialize() is called.
 * @param flags is a bitmask of some combination of S3_INIT_XXX flag, or
 *        S3_INIT_ALL, indicating which of the libraries that libs3 depends
 *        upon should be initialized by S3_initialize().  Only if your program
 *        initializes one of these dependency libraries itself should anything
 *        other than S3_INIT_ALL be passed in for this bitmask.
 *
 *        You should pass S3_INIT_WINSOCK if and only if your application does
 *        not initialize winsock elsewhere.  On non-Microsoft Windows
 *        platforms it has no effect.
 *
 *        As a convenience, the macro S3_INIT_ALL is provided, which will do
 *        all necessary initialization; however, be warned that things may
 *        break if your application re-initializes the dependent libraries
 *        later.
 * @param defaultS3Hostname is a string the specifies the default S3 server
 *        hostname to use when making S3 requests; this value is used
 *        whenever the hostName of an S3BucketContext is NULL.  If NULL is
 *        passed here then the default of S3_DEFAULT_HOSTNAME will be used.
 * @return One of:
 *         S3StatusOK on success
 *         S3StatusUriTooLong if the defaultS3HostName is longer than
 *             S3_MAX_HOSTNAME_SIZE
 *         S3StatusInternalError if dependent libraries could not be
 *             initialized
 *         S3StatusOutOfMemory on failure due to out of memory
 **/
S3Status S3_initialize(const char *userAgentInfo, int flags,
                       const char *defaultS3HostName);


/**
 * Must be called once per program for each call to libs3_initialize().  After
 * this call is complete, no libs3 function may be called except
 * S3_initialize().
 **/
void S3_deinitialize();


/**
 * Returns a string with the textual name of an S3Status code
 *
 * @param status is S3Status code for which the textual name will be returned
 * @return a string with the textual name of an S3Status code
 **/
const char *S3_get_status_name(S3Status status);


/**
 * Returns nonzero if the status indicates that the request should be
 * immediately retried, because the status indicates an error of a nature that
 * is likely due to transient conditions on the local system or S3, such as
 * network failures, or internal retryable errors reported by S3.  Returns
 * zero otherwise.
 *
 * @param status is the status to evaluate
 * @return nonzero if the status indicates a retryable error, 0 otherwise
 **/
int S3_status_is_retryable(S3Status status);


/** **************************************************************************
 * Object Functions
 ************************************************************************** **/

/**
 * Puts object data to S3.  This overwrites any existing object at that key;
 * note that S3 currently only supports full-object upload.  The data to
 * upload will be acquired by calling the handler's putObjectDataCallback.
 *
 * @param bucketContext gives the bucket and associated parameters for this
 *        request
 * @param key is the key of the object to put to
 * @param contentLength is required and gives the total number of bytes that
 *        will be put
 * @param putProperties optionally provides additional properties to apply to
 *        the object that is being put to
 * @param requestContext if non-NULL, gives the S3RequestContext to add this
 *        request to, and does not perform the request immediately.  If NULL,
 *        performs the request immediately and synchronously.
 * @param handler gives the callbacks to call as the request is processed and
 *        completed 
 * @param callbackData will be passed in as the callbackData parameter to
 *        all callbacks for this request
 **/
void S3_put_object(const S3BucketContext *bucketContext, const char *key,
                   uint64_t contentLength,
                   const S3PutProperties *putProperties,
                   S3RequestContext *requestContext,
                   const S3PutObjectHandler *handler, void *callbackData);


/**
 * Gets an object from S3.  The contents of the object are returned in the
 * handler's getObjectDataCallback.
 *
 * @param bucketContext gives the bucket and associated parameters for this
 *        request
 * @param key is the key of the object to get
 * @param getConditions if non-NULL, gives a set of conditions which must be
 *        met in order for the request to succeed
 * @param startByte gives the start byte for the byte range of the contents
 *        to be returned
 * @param byteCount gives the number of bytes to return; a value of 0
 *        indicates that the contents up to the end should be returned
 * @param requestContext if non-NULL, gives the S3RequestContext to add this
 *        request to, and does not perform the request immediately.  If NULL,
 *        performs the request immediately and synchronously.
 * @param handler gives the callbacks to call as the request is processed and
 *        completed 
 * @param callbackData will be passed in as the callbackData parameter to
 *        all callbacks for this request
 **/
void S3_get_object(const S3BucketContext *bucketContext, const char *key,
                   const S3GetConditions *getConditions,
                   uint64_t startByte, uint64_t byteCount,
                   S3RequestContext *requestContext,
                   const S3GetObjectHandler *handler, void *callbackData);


/**
 * This operation initiates a multipart upload and returns an upload ID. 
 * This upload ID is used to associate all the parts in the specific 
 * multipart upload. You specify this upload ID in each of your subsequent 
 * upload part requests
 *
 * @param bucketContext gives the bucket and associated parameters for this
 *        request; this is the bucket for which service access logging is
 *        being set
 * @param key is the source key
 * @param putProperties optionally provides additional properties to apply to
 *        the object that is being put to
 * @param handler gives the callbacks to call as the request is processed and
 *        completed 
 * @param requestContext if non-NULL, gives the S3RequestContext to add this
 *        request to, and does not perform the request immediately.  If NULL,
 *        performs the request immediately and synchronously.
 * @param callbackData will be passed in as the callbackData parameter to
 *        all callbacks for this request
 **/
void S3_initiate_multipart(S3BucketContext *bucketContext, const char *key, 
                                    S3PutProperties *putProperties,
                                    S3MultipartInitialHander *handler,
                                    S3RequestContext *requestContext,
                                    void *callbackData); 


/**
 * This operation uploads a part in a multipart upload.
 *
 * @param bucketContext gives the bucket and associated parameters for this
 *        request; this is the bucket for which service access logging is
 *        being set
 * @param key is the source key
 * @param putProperties optionally provides additional properties to apply to
 *        the object that is being put to
 * @param handler gives the callbacks to call as the request is processed and
 *        completed 
 * @param seq is a part number uniquely identifies a part and also 
 *        defines its position within the object being created. 
 * @param upload_id get from S3_initiate_multipart return 
 * @param partContentLength gives the size of the part, in bytes 
 * @param requestContext if non-NULL, gives the S3RequestContext to add this
 *        request to, and does not perform the request immediately.  If NULL,
 *        performs the request immediately and synchronously.
 * @param callbackData will be passed in as the callbackData parameter to
 *        all callbacks for this request
 **/
void S3_upload_part(S3BucketContext *bucketContext, const char *key, 
                            S3PutProperties * putProperties, S3PutObjectHandler *handler, 
                            int seq, const char *upload_id, int partContentLength, 
                            S3RequestContext *requestContext,
                            void *callbackData);


/**
 * This operation completes a multipart upload by assembling previously 
 * uploaded parts.
 *
 * @param bucketContext gives the bucket and associated parameters for this
 *        request; this is the bucket for which service access logging is
 *        being set
 * @param key is the source key
 * @param handler gives the callbacks to call as the request is processed and
 *        completed
 * @param upload_id get from S3_initiate_multipart return 
 * @param ContentLength gives the total size of the commit message, in bytes 
 * @param requestContext if non-NULL, gives the S3RequestContext to add this
 *        request to, and does not perform the request immediately.  If NULL,
 *        performs the request immediately and synchronously.
 * @param callbackData will be passed in as the callbackData parameter to
 *        all callbacks for this request
 **/
void S3_complete_multipart_upload(S3BucketContext *bucketContext, 
                                    const char *key, 
                                    S3MultipartCommitHandler *handler, 
                                    const char *upload_id, 
                                    int contentLength, 
                                    S3RequestContext *requestContext,
                                    void *callbackData);


/**
 * This operation lists the parts that have been uploaded for a specific 
 * multipart upload.
 *
 * @param bucketContext gives the bucket and associated parameters for this
 *        request; this is the bucket for which service access logging is
 *        being set
 * @param key is the source key
 * @param partnumbermarker Specifies the part after which listing should begin. 
 *        Only parts with higher part numbers will be listed.
 * @param uploadid identifying the multipart upload whose parts are being listed. 
 * @param upload_id get from S3_initiate_multipart return 
 * @param encodingtype Requests Amazon S3 to encode the response and specifies 
 *        the encoding method to use.
 * @param maxparts Sets the maximum number of parts to return in the response 
 *        body. Default: 1,000
 * @param requestContext if non-NULL, gives the S3RequestContext to add this
 *        request to, and does not perform the request immediately.  If NULL,
 *        performs the request immediately and synchronously.
 * @param handler gives the callbacks to call as the request is processed and
 *        completed
 * @param callbackData will be passed in as the callbackData parameter to
 *        all callbacks for this request
 **/
void S3_list_parts(S3BucketContext *bucketContext, const char *key,
                               const char *partnumbermarker, const char *uploadid,
                               const char *encodingtype, 
                               int maxparts,
                               S3RequestContext *requestContext,
                               const S3ListPartsHandler *handler,
                               void *callbackData);


/**
 * This operation aborts a multipart upload. After a multipart upload is 
 * aborted, no additional parts can be uploaded using that upload ID. 
 *
 * @param bucketContext gives the bucket and associated parameters for this
 *        request; this is the bucket for which service access logging is
 *        being set
 * @param key is the source key
 * @param uploadid identifying the multipart upload whose parts are being listed. 
 * @param handler gives the callbacks to call as the request is processed and
 *        completed
 **/
void S3_abort_multipart_upload(S3BucketContext *bucketContext, const char* key,
                                              const char *uploadId,
                                              S3AbortMultipartUploadHandler* handler) ;


/**
 * This operation lists in-progress multipart uploads. An in-progress 
 * multipart upload is a multipart upload that has been initiated, 
 * using the Initiate Multipart Upload request, but has not yet been 
 * completed or aborted.
 *
 * @param bucketContext gives the bucket and associated parameters for this
 *        request; this is the bucket for which service access logging is
 *        being set
 * @param Lists in-progress uploads only for those keys that begin with 
 *        the specified prefix. 
 * @param Together with upload-id-marker, this parameter specifies the 
 *        multipart upload after which listing should begin.
 * @param Together with key-marker, specifies the multipart upload 
 *        after which listing should begin. 
 * @param encodingtype Requests Amazon S3 to encode the response and specifies 
 *        the encoding method to use.
 * @param delimiter Character you use to group keys.  
 * @param maxuploads Sets the maximum number of multipart uploads, 
 *        from 1 to 1,000, to return in the response body. 
 * @param requestContext if non-NULL, gives the S3RequestContext to add this
 *        request to, and does not perform the request immediately.  If NULL,
 *        performs the request immediately and synchronously.
 * @param handler gives the callbacks to call as the request is processed and
 *        completed
 * @param callbackData will be passed in as the callbackData parameter to
 *        all callbacks for this request
 **/                          
void S3_list_multipart_uploads(S3BucketContext *bucketContext, 
                               const char *prefix,
                               const char *keymarker, const char *uploadidmarker,
                               const char *encodingtype,
                               const char *delimiter, 
                               int maxuploads,
                               S3RequestContext *requestContext,
                               const S3ListMultipartUploadsHandler *handler,
                               void *callbackData);

#ifdef __cplusplus
}
#endif

#endif /* LIBS3_H */
