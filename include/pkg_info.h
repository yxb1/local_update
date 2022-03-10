#ifndef _PACKAGE_INFO_HEAD_
#define _PACKAGE_INFO_HEAD_

#include <iostream>
#include <string>
#include <stdlib.h>

typedef struct {
    std::string pkg_name;
    std::string cache_path;
    std::string dst_path;
    std::string pkg_type;
}PKG_INFO;

#endif
