#include <iostream>
#include "parse_xml.h"

ParseXML::ParseXML() {
    std::cout << "Debug: ParseXML constructor." << std::endl;
}

void ParseXML::xml_parser(PKG_INFO *pck_info, const char *file) {
    TiXmlDocument doc;
    std::cout << GREEN << "Info : file path in xml_parser - " << file << RESET << std::endl;
    if (!doc.LoadFile(file)) {
        std::cout << RED << "Error: load xml file failed - " << doc.ErrorDesc() << RESET << std::endl;
        exit(-1);
    }
    TiXmlElement *root = doc.FirstChildElement();
    if (root->NoChildren()) {
        std::cout << RED << "Error: xml file format error, please check it" << RESET << std::endl;
        exit(-1);
    }
    TiXmlElement *msg = root->FirstChildElement();
    while(msg) {
        if(!msg->NoChildren()) {
            pkg_num = 0;
            TiXmlElement *next = msg->FirstChildElement();
            while(next) {
                if(!strcmp("pkg", next->Value())) {
                    pck_info[pkg_num].pkg_name   = next->GetText();
                    pck_info[pkg_num].cache_path = next->Attribute("cache_path");
                    pck_info[pkg_num].dst_path   = next->Attribute("dst_path");
                    pck_info[pkg_num].pkg_type   = next->Attribute("pkg_type");
                    std::cout << GREEN << "\npkg_name   : " << pck_info[pkg_num].pkg_name << RESET << std::endl;
                    std::cout << GREEN << "dst_path   : " << pck_info[pkg_num].dst_path << RESET << std::endl;
                    std::cout << GREEN << "pkg_type   : " << pck_info[pkg_num].pkg_type << RESET << std::endl;
                    std::cout << GREEN << "cache_path : " << pck_info[pkg_num].cache_path << RESET << std::endl;
                    pkg_num++;
                }else {
                    std::cout << YELLOW << "WARN : program doesn't use " << next->GetText() << " now" << RESET << std::endl;
                }
                next = next->NextSiblingElement();
            }
        }
        if(strcmp("haomo_package", msg->Value())) {
            //std::cout << msg->Value() << " : " << msg->GetText() << std::endl;
        }
        msg = msg->NextSiblingElement();
    }
}

uint8_t ParseXML::GetPkgNum() {
    return pkg_num;
}
