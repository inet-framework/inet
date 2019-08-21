//
// Copyright (C) OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//


#include <omnetpp/platdep/sockets.h>

#ifndef  __linux__
#error The 'Network Emulation Support' feature currently works on Linux systems only
#else

#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "inet/common/Endian.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/NetworkNamespaceContext.h"
#include "inet/emulation/common/ExtInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(ExtInterface);

void ExtInterface::initialize(int stage)
{
    InterfaceEntry::initialize(stage);
    if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        const char *copyConfiguration = par("copyConfiguration");
        if (strcmp("copyFromExt", copyConfiguration))
            configureInterface();
        if (!strcmp("copyFromExt", copyConfiguration))
            copyNetworkInterfaceConfigurationFromExt();
        else if (!strcmp("copyToExt", copyConfiguration))
            copyNetworkInterfaceConfigurationToExt();
    }
    else if (stage == INITSTAGE_NETWORK_ADDRESS_ASSIGNMENT) {
        const char *copyConfiguration = par("copyConfiguration");
        if (!strcmp("copyFromExt", copyConfiguration))
            copyNetworkAddressFromExt();
        else if (!strcmp("copyToExt", copyConfiguration))
            copyNetworkAddressToExt();
    }
}

void ExtInterface::configureInterface()
{
    setMtu(par("mtu"));
}

void ExtInterface::copyNetworkInterfaceConfigurationFromExt()
{
    NetworkNamespaceContext context(par("namespace"));

    std::string device = par("device").stdstringValue();
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , device.c_str() , IFNAMSIZ-1);

    ioctl(fd, SIOCGIFHWADDR, &ifr);
    MacAddress macAddress;
    macAddress.setAddressBytes((unsigned char *)ifr.ifr_hwaddr.sa_data);

    ioctl(fd, SIOCGIFMTU, &ifr);
    int mtu = ifr.ifr_mtu;

    close(fd);

    setMacAddress(macAddress);
    setMtu(mtu);
}

void ExtInterface::copyNetworkAddressFromExt()
{
    NetworkNamespaceContext context(par("namespace"));

    std::string device = par("device").stdstringValue();
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , device.c_str() , IFNAMSIZ-1);

    //get the IPv4 address
    ioctl(fd, SIOCGIFADDR, &ifr);
    Ipv4Address ipv4Address(ntohl(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr));

    //get the IPv4 netmask
    ioctl(fd, SIOCGIFNETMASK, &ifr);
    Ipv4Address ipv4Netmask(ntohl(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr));

    //TODO get IPv4 multicast addresses

    //TODO get IPv6 addresses

    close(fd);

    if (ipv4Address.isUnspecified() && ipv4Netmask.isUnspecified()) {
        auto interfaceData = findProtocolData<Ipv4InterfaceData>();
        if (interfaceData != nullptr) {
            interfaceData->setIPAddress(Ipv4Address());
            interfaceData->setNetmask(Ipv4Address());
        }
    }
    else {
        auto interfaceData = addProtocolDataIfAbsent<Ipv4InterfaceData>();
        interfaceData->setIPAddress(ipv4Address);
        interfaceData->setNetmask(ipv4Netmask);
    }
}

void ExtInterface::copyNetworkInterfaceConfigurationToExt()
{
    NetworkNamespaceContext context(par("namespace"));

    std::string device = par("device").stdstringValue();
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , device.c_str() , IFNAMSIZ-1);

    getMacAddress().getAddressBytes((unsigned char *)ifr.ifr_hwaddr.sa_data);
    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    if (ioctl(fd, SIOCSIFHWADDR, &ifr) == -1)
        throw cRuntimeError("error at mac address setting: %s", strerror(errno));

    ifr.ifr_mtu = getMtu();
    if (ioctl(fd, SIOCSIFMTU, &ifr) == -1)
        throw cRuntimeError("error at mtu setting: %s", strerror(errno));

    close(fd);
}

void ExtInterface::copyNetworkAddressToExt()
{
    NetworkNamespaceContext context(par("namespace"));

    std::string device = par("device").stdstringValue();
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , device.c_str() , IFNAMSIZ-1);

    Ipv4InterfaceData *interfaceData = findProtocolData<Ipv4InterfaceData>();
    if (interfaceData != nullptr) {
        //set the IPv4 address
        ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr = htonl(interfaceData->getIPAddress().getInt());
        ifr.ifr_addr.sa_family = AF_INET;
        if (ioctl(fd, SIOCSIFADDR, &ifr) == -1)
            throw cRuntimeError("error at ipv4 address setting: %s", strerror(errno));

        //set the IPv4 netmask
        ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr = htonl(interfaceData->getNetmask().getInt());
        ifr.ifr_addr.sa_family = AF_INET;
        if (ioctl(fd, SIOCSIFNETMASK, &ifr) == -1)
            throw cRuntimeError("error at ipv4 netmask setting: %s", strerror(errno));

        //TODO set IPv4 multicast addresses

        //TODO set IPv6 addresses
    }

    close(fd);
}

} // namespace inet

#endif // __linux__

