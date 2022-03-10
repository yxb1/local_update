#include <iostream>
#include <regex>
#include <thread>
#include "udisk_monitor.h"

UDiskMonitor *pMonitor = NULL;
UDiskMonitor::UDiskMonitor() {
    t1 = std::thread(&UDiskMonitor::UdiskNotify, this);
    std::cout << "Debug: UDiskMonitor constructor." << std::endl;
}

UDiskMonitor::~UDiskMonitor() {
    t1.join();
}

char UDiskMonitor::UdiskNotify() {
    pMonitor = this;
    struct sockaddr_nl snl;
    memset(&snl, 0, sizeof(struct sockaddr_nl));
    int buff_size = 1024*20;
    int u_sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if(u_sock == -1)
    {
        printf("\033[31mError: socket failed - %s\033[0m\n", strerror(errno));
        return -1;
    }
    snl.nl_family = AF_NETLINK;
    snl.nl_pid    = getpid();
    snl.nl_groups = 1;
    setsockopt(u_sock, SOL_SOCKET, SO_RCVBUFFORCE, (const int *)&buff_size, sizeof(buff_size));
    int ret = bind(u_sock, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
    if(ret < 0)
    {
        printf("\033[31mError: bind failed - %s\n\033[0m", strerror(errno));
        close(u_sock);
        return -1;
    }
    while(1)
    {
        char recv_buff[1024*20] = {0};
        recv(u_sock, recv_buff, buff_size, 0);
        printf("Recv :  %s\n", recv_buff);
        std::cmatch regex_result;
        if(std::regex_match(recv_buff, regex_result, std::regex("(\\w+)@.*?/block/(s\\w{2})")))
        {
            for(auto x=regex_result.begin()+1;x!=regex_result.end();++x)
            {
                if(!x->str().empty())
                {
                    //std::cout << x->str()<< std::endl;
                    if(!x->compare("add")) {
                        //std::cout << "Info: " << x->str();
                        x+=1;
                        pMonitor->_map["add"] = x->str();
                        std::cout << GREEN << "Info : dev - " << pMonitor->_map.at("add").c_str() << RESET << std::endl;
                        pMonitor->g_cond.notify_all();
                    }
                }
            }
        }
    }
    return 0;
}

const char *UDiskMonitor::GetUDev() {
    return _map.at("add").c_str();
}
