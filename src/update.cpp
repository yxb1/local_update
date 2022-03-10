#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include "update.h"

LocalUpdate *update_ptr = NULL;
LocalUpdate::LocalUpdate(UDiskMonitor *_monitor, ParseXML *_parser)
 : monitor(_monitor), parser(_parser) {
    handle = std::thread(&LocalUpdate::UpdateHandle, this);
    parser->xml_parser(pkg_info);
    std::cout << "Debug: LocalUpdate constructor" << std::endl; 
};

void LocalUpdate::UpdateHandle() {
    update_ptr = this;
    while(true) {
        std::cout << GREEN << "Info : enter the waiting state" << RESET << std::endl;
        {
            std::unique_lock<std::mutex> myLock(update_ptr->g_mutex);
            update_ptr->monitor->g_cond.wait(myLock);
            std::cout << GREEN << "Info : leave waiting state." << RESET << std::endl;
        }
        if(parser->GetPkgNum() == 0) {
            continue;
        }
        else {
            int ret = UDiskMount();
            if(!SYSTEM_RUN_RESULT(ret)) {
                std::cout << RED << "Error: mount failed" << RESET << std::endl;
                continue;
            }
        }
        for(uint8_t i = 0; i < parser->GetPkgNum(); i++) {
            if(!pkg_info[i].pkg_type.compare("HAOMO")) {
                std::cout << GREEN << "Info : update haomo package" << RESET << std::endl;
                if(update_ptr->SOCUpdate(&pkg_info[i]) == 0) {
                    std::string record_result = "echo soc success `date +%Y-%m-%d_%H-%M-%S` >> ";
                    record_result += pkg_info[i].cache_path.c_str();
                    record_result += "/update.log";
                    system(record_result.c_str());
                }
            }
            else if (!pkg_info[i].pkg_type.compare("AVP")) {
                std::cout << GREEN << "Info : update avp package" << RESET << std::endl;
                if(update_ptr->AVPUpdate(&pkg_info[i]) == 0) {
                    std::string record_result = "echo avp success `date +%Y-%m-%d_%H-%M-%S` >> ";
                    record_result += pkg_info[i].cache_path.c_str();
                    record_result += "/update.log";
                    system(record_result.c_str());
                }
            }
            else {
                std::cout << RED << "Error: pkg_type not surport" << RESET << std::endl;
            }
        }
        if(parser->GetPkgNum() == 0) {
            continue;
        }
        else {
            int ret = UDiskUmount();
            if(!SYSTEM_RUN_RESULT(ret)) {
                std::cout << RED << "Error: umount failed" << RESET << std::endl;
                continue;
            }
        }
    }
}

int LocalUpdate::SOCUpdate(PKG_INFO *pkg) {
    //std::cout << GREEN << "==========haomo update==========" << RESET << std::endl;
    char src_path[256] = {0};
    if(CheckValidity(pkg->cache_path.c_str(), src_path, pkg) == false) {
        std::cout << RED << "Error: don't find update package" << RESET << std::endl;
        return -1;
    }

    int ret = RemountPartition("/opt/app", "rw");
    if(!SYSTEM_RUN_RESULT(ret)) {
        std::cout << RED << "Error: unmount fail - " << strerror(errno) << RESET << std::endl;
        return -1;
    }

    if(access(pkg->dst_path.c_str(), F_OK)) {
        if(mkdir(pkg->dst_path.c_str(), 0777) != 0) {
            std::cout << RED << "Error: create dst_path fail - " << strerror(errno) << RESET << std::endl;
            RemountPartition("/opt/app", "ro");
            return -1;
        }
    }
    ret = UnzipTarBag(src_path, pkg->dst_path.c_str());
    if(!SYSTEM_RUN_RESULT(ret)) {
        std::cout << RED << "Error: tar tgz bag fail" << RESET << std::endl;
        RemountPartition("/opt/app", "ro");
        return -1;
    }

    system("sync");
    RemountPartition("/opt/app", "ro");
    //std::cout << GREEN << "==========haomo update end======" << RESET << std::endl;
    return 0;
}

int LocalUpdate::AVPUpdate(PKG_INFO *pkg) {
    //std::cout << GREEN << "**********avp update**********" << RESET << std::endl;
    char src_path[256] = {0};
    if(CheckValidity(pkg->cache_path.c_str(), src_path, pkg) == false) {
        std::cout << RED << "Error: don't find update package" << RESET << std::endl;
        return -1;
    }

    int ret = UnzipTarBag(src_path, pkg->dst_path.c_str());
    if(!SYSTEM_RUN_RESULT(ret)) {
        std::cout << RED << "Error: tar tgz bag fail - " << strerror(errno) << RESET << std::endl;
        return -1;
    }

    PKG_INFO tmp;
    tmp.pkg_name = "deploy.sh";
    system("sync");
    if(CheckValidity(pkg->dst_path.c_str(), src_path, &tmp, 5) == false) {
        std::cout << RED << "Error: don't find avp deploy.sh file" << RESET << std::endl;
        return -1;
    }
    
    char _dir[256] = {0};
    strcpy(_dir, src_path);
    if(chdir(dirname(_dir)) != 0) {
        std::cout << RED << "Error: get dir name fail - " << strerror(errno) << RESET << std::endl;
        return -1;
    }

    std::string script_cmd = "/bin/bash ";
    script_cmd.append(src_path);
    script_cmd += " p03";
    std::cout << GREEN << "Info : deploy avp - " << script_cmd << RESET << std::endl;
    ret = system(script_cmd.c_str());
    if(!SYSTEM_RUN_RESULT(ret)) {
        std::cout << RED << "Error: deploy fail - " << strerror(errno) << RESET << std::endl;
        return -1;
    }
    system("sync");

    //script_cmd.clear();
    //script_cmd = "rm -fr ";
    //script_cmd.append(_dir);
    //std::cout << GREEN << "Info : " << script_cmd << RESET << std::endl;
    //ret = system(script_cmd.c_str());
    //if(!SYSTEM_RUN_RESULT(ret)) {
    //    std::cout << RED << "Error: rm avp unzip bag - " << strerror(errno) << RESET << std::endl;
    //}
    //std::cout << GREEN << "**********avp update end******" << RESET << std::endl;
    return 0;
}

int LocalUpdate::UDiskMount() {
    std::string mnt = "mount /dev/";
    mnt += monitor->GetUDev();
    mnt += "1 ";
    mnt.append(pkg_info[0].cache_path.c_str());  //take a look
    std::cout << "Debug: " << mnt.c_str() << std::endl;
    return system(mnt.c_str());
}

int LocalUpdate::UDiskUmount() {
    std::string mnt = "umount -v /dev/";
    mnt += monitor->GetUDev();
    mnt += "1";
    std::cout << "Debug: " << mnt.c_str() << std::endl;
    return system(mnt.c_str());
}

int LocalUpdate::UnzipTarBag(char *tgz, const char *dst) {
    std::string mnt = "tar -zxf ";
    mnt += tgz;
    if(dst != NULL) {
        mnt += " -C ";
        mnt.append(dst); 
    }
    std::cout << GREEN << "Info : " << mnt.c_str() << RESET << std::endl;
    std::cout << GREEN << "Info : please waiting..." << RESET << std::endl;
    return system(mnt.c_str());
}

int LocalUpdate::RemountPartition(const char *partition, const char *rw) {
    std::string cmd = "mount -o remount,";
    cmd += rw;
    cmd += " ";
    cmd.append(partition);
    std::cout << "Debug: " << cmd.c_str() << std::endl;
    return system(cmd.c_str());
}

bool LocalUpdate::CheckValidity(const char * dir, char *UPath, PKG_INFO *pkg, int depth) {
    if(dir == NULL)
	{
        std::cout << RED << "Error: folder path is null" << RESET << std::endl;
		return false;
	}

	unsigned char ticks = 5;
	do
	{
		if(access(dir, F_OK) != 0)
		{
            std::cout << RED << "Error: folder path don't exit!" << RESET << std::endl;
			sleep(1);
			if(ticks == 0) return false;
			continue;
		}
		else
		{
            std::cout << "Debug: folder path exits!" << std::endl;
			break;
		}
	}while(ticks--);

	DIR *pdir = NULL;
	struct dirent *pdirent = NULL;
	char storagepath[100] = {0};
	char fullpath[100] = {0};
	char parentpath[100] = {0};
	struct stat filestatus;

	if((pdir = opendir(dir)) == NULL)
	{
        printf("\033[33mWarn : opendir %s falied - %s \033[0m\n", dir, strerror(errno));
		return false;
	}

	memset(&filestatus, 0, sizeof(filestatus));
	strcpy(parentpath, dir);
	if(parentpath[strlen(dir) - 1] != '/') strcat(parentpath, "/");
    printf("Debug: parentpath - %s \n", parentpath);
	
	while((pdirent = readdir(pdir)) != NULL)
	{
        printf("Debug: d_name - %s\n", pdirent->d_name);
		if((strcmp(pdirent->d_name, ".") == 0) || (strcmp(pdirent->d_name, "..") == 0)) continue;
		if(strstr(pdirent->d_name, "=") != NULL) continue;
		strcpy(fullpath, parentpath);
		strcat(fullpath, pdirent->d_name);
        printf("Debug: fullpath - %s\n", fullpath);

		if(stat(fullpath, &filestatus) != 0)
		{
            printf("\033[31mError: stat failed - %s\033[0m\n", strerror(errno));
			return false;
		}
		
		if(S_ISREG(filestatus.st_mode) || S_ISDIR(filestatus.st_mode))
		{
            std::string file_name = pdirent->d_name;
            std::string::size_type idx = file_name.find(pkg->pkg_name);
            if(idx == std::string::npos) {
                if(depth > 1)
				{
					if(CheckValidity((const char *)fullpath, UPath, pkg, (depth - 1))) return true;
				}
				else
				{
                    printf("\033[33mWARN : depth(%d) is too deep to search, folder name: %s\033[0m\n", depth, fullpath);
				}
            }else {
				memset(storagepath, 0, sizeof(storagepath));
				strcpy(storagepath, dir);
				strcat(storagepath, "/");
				strcat(storagepath, pdirent->d_name);
				strcpy(UPath, storagepath);
                printf("\033[32mInfo : it's a trusted UDisk, copy path = %s\033[0m\n", storagepath);
				return true;
            }
		}
	}
	return false;
}

