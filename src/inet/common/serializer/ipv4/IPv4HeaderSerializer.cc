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

#include "inet/common/packet/SerializerRegistry.h"
#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/defs.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/ipv4/headers/ip.h"
#include "inet/common/serializer/ipv4/IPv4HeaderSerializer.h"
#include "inet/networklayer/ipv4/IPv4Header.h"

namespace inet {

namespace serializer {

Register_Serializer(IPv4Header, IPv4HeaderSerializer);

void IPv4HeaderSerializer::serialize(ByteOutputStream& stream, const std::shared_ptr<Chunk>& chunk) const
{
    struct ip iphdr;
    const auto& ipv4Header = std::static_pointer_cast<const IPv4Header>(chunk);
    unsigned int headerLength = ipv4Header->getHeaderLength();
    ASSERT((headerLength & 3) == 0);
    iphdr.ip_hl = headerLength >> 2;
    iphdr.ip_v = ipv4Header->getVersion();
    iphdr.ip_tos = ipv4Header->getTypeOfService();
    iphdr.ip_id = htons(ipv4Header->getIdentification());
    ASSERT((ipv4Header->getFragmentOffset() & 7) == 0);
    uint16_t ip_off = ipv4Header->getFragmentOffset() / 8;
    if (ipv4Header->getMoreFragments())
        ip_off |= IP_MF;
    if (ipv4Header->getDontFragment())
        ip_off |= IP_DF;
    iphdr.ip_off = htons(ip_off);
    iphdr.ip_ttl = ipv4Header->getTimeToLive();
    iphdr.ip_p = ipv4Header->getTransportProtocol();
    iphdr.ip_src.s_addr = htonl(ipv4Header->getSrcAddress().getInt());
    iphdr.ip_dst.s_addr = htonl(ipv4Header->getDestAddress().getInt());
    iphdr.ip_len = htons(ipv4Header->getTotalLengthField());
    iphdr.ip_sum = 0;

    if (headerLength > IP_HEADER_BYTES) {
        throw cRuntimeError("ideiglenesen");
//        Buffer sb(b, headerLength - IP_HEADER_BYTES);
//        serializeOptions(dgram, sb, c);
//        stream.accessNBytes(sstream.getPos());
//        if (sstream.hasError())
//            stream.setError();
    }

    iphdr.ip_sum = htons(ipv4Header->getCrc());

    for (int i = 0; i < IP_HEADER_BYTES; i++)
        stream.writeByte(((uint8_t *)&iphdr)[i]);
}

std::shared_ptr<Chunk> IPv4HeaderSerializer::deserialize(ByteInputStream& stream) const
{
    uint8_t buffer[IP_HEADER_BYTES];
    for (int i = 0; i < IP_HEADER_BYTES; i++)
        buffer[i] = stream.readByte();
    auto ipv4Header = std::make_shared<IPv4Header>();
    unsigned int bufsize = stream.getRemainingSize();
    const struct ip *iphdr = static_cast<const struct ip *>((void *)&buffer);
    unsigned int totalLength, headerLength;

    ipv4Header->setVersion(iphdr->ip_v);
    ipv4Header->setHeaderLength(IP_HEADER_BYTES);
    ipv4Header->setSrcAddress(IPv4Address(ntohl(iphdr->ip_src.s_addr)));
    ipv4Header->setDestAddress(IPv4Address(ntohl(iphdr->ip_dst.s_addr)));
    ipv4Header->setTransportProtocol(iphdr->ip_p);
    ipv4Header->setTimeToLive(iphdr->ip_ttl);
    ipv4Header->setIdentification(ntohs(iphdr->ip_id));
    uint16_t ip_off = ntohs(iphdr->ip_off);
    ipv4Header->setMoreFragments((ip_off & IP_MF) != 0);
    ipv4Header->setDontFragment((ip_off & IP_DF) != 0);
    ipv4Header->setFragmentOffset((ntohs(iphdr->ip_off) & IP_OFFMASK) * 8);
    ipv4Header->setTypeOfService(iphdr->ip_tos);
    totalLength = ntohs(iphdr->ip_len);
    ipv4Header->setTotalLengthField(totalLength);
    headerLength = iphdr->ip_hl << 2;

    if (headerLength < IP_HEADER_BYTES) {
        ipv4Header->markIncorrect();
        headerLength = IP_HEADER_BYTES;
    }

    ipv4Header->setHeaderLength(headerLength);

    if (headerLength > IP_HEADER_BYTES) {    // options present?
        unsigned short optionBytes = headerLength - IP_HEADER_BYTES;
        throw cRuntimeError("ideiglenesen");
//        Buffer sb(b, optionBytes);
//        deserializeOptions(dest, sb, c);
//        if (sstream.hasError())
//            stream.setError();
    }
    stream.seek(headerLength);

    if (totalLength > bufsize)
        EV_ERROR << "Can not handle IPv4 packet of total length " << totalLength << "(captured only " << bufsize << " bytes).\n";

    ipv4Header->setHeaderLength(headerLength);
    return ipv4Header;
}

} // namespace serializer

} // namespace inet

