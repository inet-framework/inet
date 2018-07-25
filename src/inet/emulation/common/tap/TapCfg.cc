//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#define WANT_WINSOCK2

#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <unistd.h>

#include "inet/common/INETDefs.h"

#include <omnetpp/platdep/sockets.h>

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/linklayer/common/EtherType_m.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/ethernet/EtherPhyFrame_m.h"
#include "inet/emulation/common/tap/TapCfg.h"

#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(TapCfg);

TapCfg::~TapCfg()
{
}

void TapCfg::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER_2) {
        std::string device = par("device").stdstringValue();
        int fd;
        struct ifreq ifr;

        fd = socket(AF_INET, SOCK_DGRAM, 0);

        //Type of address to retrieve - IPv4 IP address
        ifr.ifr_addr.sa_family = AF_INET;

        //Copy the interface name in the ifreq structure
        strncpy(ifr.ifr_name , device.c_str() , IFNAMSIZ-1);

        //get the IPv4 address
        ioctl(fd, SIOCGIFADDR, &ifr);
        Ipv4Address ipv4Address = Ipv4Address(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

        //get the IPv4 netmask
        ioctl(fd, SIOCGIFNETMASK, &ifr);
        Ipv4Address ipv4Netmask = Ipv4Address(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

        ioctl(fd, SIOCGIFHWADDR, &ifr);
        MacAddress macAddress;
        macAddress.setAddressBytes((unsigned char *)ifr.ifr_hwaddr.sa_data);

        ioctl(fd, SIOCGIFMTU, &ifr);
        int mtu = ifr.ifr_mtu;

        //TODO get IPv4 multicast addresses

        //TODO get IPv6 addresses

        close(fd);

        InterfaceEntry *interfaceEntry = getModuleFromPar<InterfaceEntry>(par("interfaceModule"), this);

        Ipv4InterfaceData *interfaceData = interfaceEntry->ipv4Data();
        if (interfaceData == nullptr)
            interfaceEntry->setIpv4Data(interfaceData = new Ipv4InterfaceData());
        interfaceEntry->setMacAddress(macAddress);
        interfaceEntry->setMtu(mtu);
        //interfaceData->setMetric(metric);
        interfaceData->setIPAddress(Ipv4Address(ipv4Address));
        interfaceData->setNetmask(Ipv4Address(ipv4Netmask));
    }
}

void TapCfg::handleMessage(cMessage *msg)
{
    throw cRuntimeError("this module can't accept messages");
}

} // namespace inet

