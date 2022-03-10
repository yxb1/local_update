#ifndef _LOCAL_UPDATE_HEADER_FILE_
#define _LOCAL_UPDATE_HEADER_FILE_

#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <unistd.h>

#include "pkg_info.h"
#include "parse_xml.h"
#include "udisk_monitor.h"

#define SYSTEM_RUN_RESULT(ret) ((ret != -1) && WIFEXITED(ret) && (WEXITSTATUS(ret) == 0))

class LocalUpdate {
public:
    LocalUpdate(UDiskMonitor *_monitor, ParseXML *_parser);
    virtual ~LocalUpdate() = default;
    int SOCUpdate(PKG_INFO *pkg);
    void UpdateHandle();
    bool CheckValidity(const char * dir, char *UPath, PKG_INFO *pkg, int depth = 3);
    int UDiskMount();
    int UDiskUmount();
    int UnzipTarBag(char *tgz, const char *dst = NULL);
    int AVPUpdate(PKG_INFO *pkg);
    int RemountPartition(const char *partition, const char *rw);
    int KillSocProgram(PKG_INFO *pkg);

private:
    ParseXML     *parser;
    UDiskMonitor *monitor;
    std::mutex   g_mutex;
    std::thread  handle;
    PKG_INFO     pkg_info[5];
};

#endif
