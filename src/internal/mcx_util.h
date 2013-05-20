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


}
}

#endif

