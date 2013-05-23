#ifndef MCX_UTILITY_H_
#define MCX_UTILITY_H_

#include <stdio.h>
#include <string>

namespace mcx
{
namespace detail
{

inline bool writeToFile( const char* filepath, const void* content, const size_t len )
{
    FILE* fp = ::fopen( filepath, "w+");

    if ( fp == NULL )
    {
        fprintf( stderr, "%s : could not open file \"%s\" for write\n", __func__, filepath);
        return false;
    }

    ::fwrite(content, sizeof(char), len, fp);
    ::fflush(fp);
    ::fclose(fp);

    return true;
}

inline bool writeToFile( const char* filepath, const std::string& content)
{
    return writeToFile(filepath, content.data(), content.size());
}

#ifndef H_CASE_STRING_BIGIN
    #define H_CASE_STRING_BIGIN(state) switch(state){
    #define H_CASE_STRING(state) case state:return #state;break;
    #define H_CASE_STRING_END()  default:return "Unknown";break;}
#endif

inline const char* statusToString(int status) {
    H_CASE_STRING_BIGIN(status);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_SUCCESS);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_KEY_ENOENT);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_KEY_EEXISTS);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_E2BIG);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_EINVAL);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_NOT_STORED);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_DELTA_BADVAL);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_NOT_MY_VBUCKET);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_AUTH_ERROR);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_AUTH_CONTINUE);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_ERANGE);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_UNKNOWN_COMMAND);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_ENOMEM);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_NOT_SUPPORTED);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_EINTERNAL);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_EBUSY);
    H_CASE_STRING(PROTOCOL_BINARY_RESPONSE_ETMPFAIL);
    H_CASE_STRING_END();
}

}
}

#endif

