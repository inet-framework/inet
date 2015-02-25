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

#include "inet/common/serializer/headerserializers/arp/ARPSerializer.h"

namespace INETFw // load headers into a namespace, to avoid conflicts with platform definitions of the same stuff
{
#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/common/serializer/headerserializers/headers/ethernet.h"
#include "inet/common/serializer/headerserializers/arp/headers/arp.h"
};

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#endif

namespace inet {
namespace serializer {

using namespace INETFw;

int ARPSerializer::serialize(const ARPPacket *pkt, unsigned char *buf, unsigned int bufsize)
{
    struct arphdr *arphdr = (struct arphdr *) (buf);

    arphdr->ar_hrd = htons(1); //ethernet
    arphdr->ar_pro = htons(ETHERTYPE_IP);
    arphdr->ar_hln = ETHER_ADDR_LEN;
    arphdr->ar_pln = 4; //IPv4
    arphdr->ar_op = htons(pkt->getOpcode());
    pkt->getSrcMACAddress().getAddressBytes(arphdr->ar_sha);
    arphdr->ar_spa = htonl(pkt->getSrcIPAddress().getInt());
    pkt->getDestMACAddress().getAddressBytes(arphdr->ar_tha);
    arphdr->ar_tpa = htonl(pkt->getDestIPAddress().getInt());

    int packetLength = 2+2+1+1+2+2*arphdr->ar_hln+2*arphdr->ar_pln;
    return packetLength;
}

void ARPSerializer::parse(const unsigned char *buf, unsigned int bufsize, ARPPacket *pkt)
{
    struct arphdr *arphdr = (struct arphdr *) (buf);

    //arphdr->ar_hrd
    //arphdr->ar_pro
    //arphdr->ar_hln
    //arphdr->ar_pln

    pkt->setOpcode(ntohs(arphdr->ar_op));
    MACAddress temp;
    temp.setAddressBytes(arphdr->ar_sha);
    pkt->setSrcMACAddress(temp);
    temp.setAddressBytes(arphdr->ar_tha);
    pkt->setDestMACAddress(temp);
    pkt->setSrcIPAddress(IPv4Address(ntohl(arphdr->ar_spa)));
    pkt->setDestIPAddress(IPv4Address(ntohl(arphdr->ar_tpa)));
    pkt->setByteLength(28);
}

} // namespace serializer
} // namespace inet
