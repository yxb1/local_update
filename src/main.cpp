#include <string>
#include <iostream>
#include <sstream>
#include <memory>

#include "update.h"

int main(int argc, char **argv) {
    std::shared_ptr<LocalUpdate> _update = std::make_shared<LocalUpdate>(new UDiskMonitor, new ParseXML);
    while(1) {
        sleep(10);
    }
    return 0;
}