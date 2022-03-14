#ifndef _PARSE_XML_H_
#define _PARSE_XML_H_

#include <string>
#include <thread>
#include "tinyxml.h"
#include "pkg_info.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */

class ParseXML
{
public:
    ParseXML();
    ~ParseXML() = default;
    void xml_parser(PKG_INFO *pck_info, const char *file = "/opt/app/update/xml/setting.xml");
    uint8_t GetPkgNum();

private:
    uint8_t pkg_num;
};

#endif
