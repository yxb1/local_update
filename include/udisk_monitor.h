#ifndef _UDISK_MONITOR_H_
#define _UDISK_MONITOR_H_

#include <map>
#include <mutex>
#include <condition_variable>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "parse_xml.h"

class UDiskMonitor {
public:
    UDiskMonitor();
    virtual ~UDiskMonitor();
    char UdiskNotify();
    const char *GetUDev();
public:
    std::condition_variable g_cond;
private:
    std::thread t1;
    std::map<std::string, std::string> _map;
};

#endif
