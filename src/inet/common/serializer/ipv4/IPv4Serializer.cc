//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
// Copyright (C) 2009 Thomas Reschka
// Copyright (C) 2010 Zoltan Bojthe
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

#include <algorithm>    // std::min
#include <typeinfo>

#include "inet/common/serializer/SerializerUtil.h"

#include "inet/common/serializer/ipv4/IPv4Serializer.h"
#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/defs.h"

#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/common/serializer/ipv4/headers/ip.h"
#include "inet/common/serializer/ipv4/ICMPSerializer.h"
#include "inet/common/serializer/ipv4/IGMPSerializer.h"
#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/networklayer/common/IPProtocolId_m.h"

#if defined(_MSC_VER)
#undef s_addr    /* MSVC #definition interferes with us */
#endif // if defined(_MSC_VER)

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

// This in_addr field is defined as a macro in Windows and Solaris, which interferes with us
#undef s_addr

namespace inet {

namespace serializer {

Register_Serializer(IPv4Datagram, ETHERTYPE, ETHERTYPE_IPv4, IPv4Serializer);

IPv4OptionSerializerRegistrationList ipv4OptionSerializers("IPv4OptionSerializers"); ///< List of IPv4Option serializers (IPv4OptionSerializerBase)

EXECUTE_ON_SHUTDOWN(
        ipv4OptionSerializers.clear();
        );

IPv4OptionDefaultSerializer IPv4OptionSerializerRegistrationList::defaultSerializer("IPv4OptionDefaultSerializer");

void IPv4OptionDefaultSerializer::serializeOption(const TLVOptionBase *option, Buffer &b, Context& c)
{
    unsigned short type = option->getType();
    unsigned short length = option->getLength();    // length >= 1

    b.writeByte(type);
    if (length > 1)
        b.writeByte(length);

    auto *opt = dynamic_cast<const TLVOptionRaw *>(option);
    if (opt) {
        unsigned int datalen = opt->getBytesArraySize();
        ASSERT(length == 2 + datalen);
        for (unsigned int i = 0; i < datalen; i++)
            b.writeByte(opt->getBytes(i));
        return;
    }

    switch (type) {
        case IPOPTION_END_OF_OPTIONS:
            check_and_cast<const IPv4OptionEnd *>(option);
            ASSERT(length == 1);
            break;

        case IPOPTION_NO_OPTION:
            check_and_cast<const IPv4OptionNop *>(option);
            ASSERT(length == 1);
            break;

        case IPOPTION_STREAM_ID: {
            auto *opt = check_and_cast<const IPv4OptionStreamId *>(option);
            ASSERT(length == 4);
            b.writeUint16(opt->getStreamId());
            break;
        }

        case IPOPTION_TIMESTAMP: {
            auto *opt = check_and_cast<const IPv4OptionTimestamp *>(option);
            int bytes = (opt->getFlag() == IP_TIMESTAMP_TIMESTAMP_ONLY) ? 4 : 8;
            ASSERT(length == 4 + bytes * opt->getRecordTimestampArraySize());
            uint8_t pointer = 5 + opt->getNextIdx() * bytes;
            b.writeByte(pointer);
            uint8_t flagbyte = opt->getOverflow() << 4 | opt->getFlag();
            b.writeByte(flagbyte);
            for (unsigned int count = 0; count < opt->getRecordTimestampArraySize(); count++) {
                if (bytes == 8)
                    b.writeIPv4Address(opt->getRecordAddress(count));
                b.writeUint32(opt->getRecordTimestamp(count).inUnit(SIMTIME_MS));
            }
            break;
        }

        case IPOPTION_RECORD_ROUTE:
        case IPOPTION_LOOSE_SOURCE_ROUTING:
        case IPOPTION_STRICT_SOURCE_ROUTING: {
            auto *opt = check_and_cast<const IPv4OptionRecordRoute *>(option);
            ASSERT(length == 3 + 4 * opt->getRecordAddressArraySize());
            uint8_t pointer = 4 + opt->getNextAddressIdx() * 4;
            b.writeByte(pointer);
            for (unsigned int count = 0; count < opt->getRecordAddressArraySize(); count++) {
                b.writeIPv4Address(opt->getRecordAddress(count));
            }
            break;
        }
        case IPOPTION_ROUTER_ALERT:
        case IPOPTION_SECURITY:
        default: {
            throw cRuntimeError("Unknown IPv4Option type=%d (not in an TLVOptionRaw option)", type);
            break;
        }
    }
}

void IPv4Serializer::serialize(const cPacket *pkt, Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);

    if (typeid(*pkt) != typeid(IPv4Datagram)) {
        if (c.throwOnSerializerNotFound)
            throw cRuntimeError("IPv4Serializer: class '%s' not accepted", pkt->getClassName());
        EV_ERROR << "IPv4Serializer: class '" << pkt->getClassName() << "' not accepted.\n";
        b.fillNBytes(pkt->getByteLength(), '?');
        return;
    }

    struct ip *ip = (struct ip *)b.accessNBytes(IP_HEADER_BYTES);
    if (!ip) {
        EV_ERROR << "IPv4Serializer: not enough space for IPv4 header.\n";
        return;
    }
    const IPv4Datagram *dgram = check_and_cast<const IPv4Datagram *>(pkt);
    unsigned int headerLength = dgram->getHeaderLength();
    ASSERT((headerLength & 3) == 0);
    ip->ip_hl = headerLength >> 2;
    ip->ip_v = dgram->getVersion();
    ip->ip_tos = dgram->getTypeOfService();
    ip->ip_id = htons(dgram->getIdentification());
    ASSERT((dgram->getFragmentOffset() & 7) == 0);
    uint16_t ip_off = dgram->getFragmentOffset() / 8;
    if (dgram->getMoreFragments())
        ip_off |= IP_MF;
    if (dgram->getDontFragment())
        ip_off |= IP_DF;
    ip->ip_off = htons(ip_off);
    ip->ip_ttl = dgram->getTimeToLive();
    ip->ip_p = dgram->getTransportProtocol();
    ip->ip_src.s_addr = htonl(dgram->getSrcAddress().getInt());
    ip->ip_dst.s_addr = htonl(dgram->getDestAddress().getInt());
    ip->ip_len = htons(dgram->getTotalLengthField());
    ip->ip_sum = 0;
    c.l3AddressesPtr = &ip->ip_src.s_addr;
    c.l3AddressesLength = sizeof(ip->ip_src.s_addr) + sizeof(ip->ip_dst.s_addr);

    if (headerLength > IP_HEADER_BYTES) {
        Buffer sb(b, headerLength - IP_HEADER_BYTES);
        serializeOptions(dgram, sb, c);
        b.accessNBytes(sb.getPos());
        if (sb.hasError())
            b.setError();
    }

    const cPacket *encapPacket = dgram->getEncapsulatedPacket();
    unsigned int payloadLength = dgram->getByteLength() - b.getPos();

    if (encapPacket) {
        unsigned int totalLength = encapPacket->getByteLength();
        int fragmentOffset = dgram->getFragmentOffset();
        if ((dgram->getMoreFragments() || fragmentOffset != 0) && (payloadLength < totalLength)) {  // IP fragment  //FIXME hack: encapsulated packet contains entire packet if payloadLength < totalLength
            char *buf = new char[totalLength];
            Buffer tmpBuffer(buf, totalLength);
            SerializerBase::lookupAndSerialize(encapPacket, tmpBuffer, c, IP_PROT, dgram->getTransportProtocol());
            tmpBuffer.seek(fragmentOffset);
            b.writeNBytes(tmpBuffer, payloadLength);
            delete [] buf;
        }
        else    // no fragmentation, or the encapsulated packet is represents only the fragment
            SerializerBase::lookupAndSerialize(encapPacket, b, c, IP_PROT, dgram->getTransportProtocol());
    }
    else {
        b.fillNBytes(payloadLength, '?');
    }

    ip->ip_sum = htons(TCPIPchecksum::checksum(ip, IP_HEADER_BYTES));
}

TLVOptionBase *IPv4OptionDefaultSerializer::deserializeOption(Buffer &b, Context& c)
{
    unsigned int pos = b.getPos();
    unsigned char type = b.readByte();
    unsigned char length = 1;

    switch (type) {
        case IPOPTION_END_OF_OPTIONS:    // EOL
            return new IPv4OptionEnd();

        case IPOPTION_NO_OPTION:    // NOP
            return new IPv4OptionNop();

        case IPOPTION_STREAM_ID:
            length = b.readByte();
            if (length == 4) {
                auto *option = new IPv4OptionStreamId();
                option->setType(type);
                option->setLength(length);
                option->setStreamId(b.readUint16());
                return option;
            }
            break;

        case IPOPTION_TIMESTAMP: {
            length = b.readByte();
            uint8_t pointer = b.readByte();
            uint8_t flagbyte = b.readByte();
            uint8_t overflow = flagbyte >> 4;
            int flag = -1;
            int bytes = 0;
            switch (flagbyte & 0x0f) {
                case 0: flag = IP_TIMESTAMP_TIMESTAMP_ONLY; bytes = 4; break;
                case 1: flag = IP_TIMESTAMP_WITH_ADDRESS; bytes = 8; break;
                case 3: flag = IP_TIMESTAMP_SENDER_INIT_ADDRESS; bytes = 8; break;
                default: break;
            }
            if (flag != -1 && length > 4 && bytes && ((length-4) % bytes) == 0 && pointer >= 5 && ((pointer-5) % bytes) == 0) {
                auto *option = new IPv4OptionTimestamp();
                option->setType(type);
                option->setLength(length);
                option->setFlag(flag);
                option->setOverflow(overflow);
                option->setRecordTimestampArraySize((length - 4) / bytes);
                if (bytes == 8)
                    option->setRecordAddressArraySize((length - 4) / bytes);
                option->setNextIdx((pointer-5) / bytes);
                for (unsigned int count = 0; count < option->getRecordAddressArraySize(); count++) {
                    if (bytes == 8)
                        option->setRecordAddress(count, b.readIPv4Address());
                    option->setRecordTimestamp(count, SimTime(b.readUint32(), SIMTIME_MS));
                }
                return option;
            }
            break;
        }

        case IPOPTION_RECORD_ROUTE:
        case IPOPTION_LOOSE_SOURCE_ROUTING:
        case IPOPTION_STRICT_SOURCE_ROUTING: {
            length = b.readByte();
            uint8_t pointer = b.readByte();
            if (length > 3 && (length % 4) == 3 && pointer >= 4 && (pointer % 4) == 0) {
                auto *option = new IPv4OptionRecordRoute();
                option->setType(type);
                option->setLength(length);
                option->setRecordAddressArraySize((length - 3) / 4);
                option->setNextAddressIdx((pointer-4) / 4);
                for (unsigned int count = 0; count < option->getRecordAddressArraySize(); count++) {
                    option->setRecordAddress(count, b.readIPv4Address());
                }
                return option;
            }
            break;
        }

        case IPOPTION_ROUTER_ALERT:
        case IPOPTION_SECURITY:
        default:
            length = b.readByte();
            break;
    }    // switch

    auto *option = new TLVOptionRaw();
    b.seek(pos);
    type = b.readByte();
    length = b.readByte();
    option->setType(type);
    option->setLength(length);
    if (length > 2)
        option->setBytesArraySize(length - 2);
    for (unsigned int i = 2; i < length; i++)
        option->setBytes(i-2, b.readByte());
    return option;
}

cPacket* IPv4Serializer::deserialize(const Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);

    IPv4Datagram *dest = new IPv4Datagram("parsed-ipv4");
    unsigned int bufsize = b.getRemainingSize();
    const struct ip *ip = static_cast<const struct ip *>(b.accessNBytes(IP_HEADER_BYTES));
    if (!ip ) {
        delete dest;
        return nullptr;
    }
    unsigned int totalLength, headerLength;
    c.l3AddressesPtr = &ip->ip_src.s_addr;
    c.l3AddressesLength = sizeof(ip->ip_src.s_addr) + sizeof(ip->ip_dst.s_addr);

    dest->setVersion(ip->ip_v);
    dest->setHeaderLength(IP_HEADER_BYTES);
    dest->setSrcAddress(IPv4Address(ntohl(ip->ip_src.s_addr)));
    dest->setDestAddress(IPv4Address(ntohl(ip->ip_dst.s_addr)));
    dest->setTransportProtocol(ip->ip_p);
    dest->setTimeToLive(ip->ip_ttl);
    dest->setIdentification(ntohs(ip->ip_id));
    uint16_t ip_off = ntohs(ip->ip_off);
    dest->setMoreFragments((ip_off & IP_MF) != 0);
    dest->setDontFragment((ip_off & IP_DF) != 0);
    dest->setFragmentOffset((ntohs(ip->ip_off) & IP_OFFMASK) * 8);
    dest->setTypeOfService(ip->ip_tos);
    totalLength = ntohs(ip->ip_len);
    dest->setTotalLengthField(totalLength);
    headerLength = ip->ip_hl << 2;

    if (headerLength < IP_HEADER_BYTES) {
        dest->setBitError(true);
        headerLength = IP_HEADER_BYTES;
    }

    dest->setHeaderLength(headerLength);

    if (headerLength > b._getBufSize() || TCPIPchecksum::checksum(ip, headerLength) != 0)
        dest->setBitError(true);

    if (headerLength > IP_HEADER_BYTES) {    // options present?
        unsigned short optionBytes = headerLength - IP_HEADER_BYTES;
        Buffer sb(b, optionBytes);
        deserializeOptions(dest, sb, c);
        if (sb.hasError())
            b.setError();
    }
    b.seek(headerLength);

    if (totalLength > bufsize)
        EV_ERROR << "Can not handle IPv4 packet of total length " << totalLength << "(captured only " << bufsize << " bytes).\n";

    dest->setByteLength(headerLength);
    unsigned int payloadLength = totalLength - headerLength;
    cPacket *encapPacket = nullptr;
    if (dest->getMoreFragments() || dest->getFragmentOffset() != 0) {  // IP fragment
        Buffer subBuffer(b, payloadLength);
        encapPacket = serializers.byteArraySerializer.deserialize(subBuffer, c);
        b.accessNBytes(subBuffer.getPos());
    }
    else
        encapPacket = SerializerBase::lookupAndDeserialize(b, c, IP_PROT, dest->getTransportProtocol(), payloadLength);

    if (encapPacket) {
        dest->encapsulate(encapPacket);
        dest->setName(encapPacket->getName());
    }
    return dest;
}

void IPv4Serializer::serializeOptions(const IPv4Datagram *dgram, Buffer& b, Context& c)
{
    unsigned short numOptions = dgram->getOptionArraySize();
    unsigned int optionsLength = 0;
    if (numOptions > 0) {    // options present?
        for (unsigned short i = 0; i < numOptions; i++) {
            const TLVOptionBase *option = &dgram->getOption(i);
            ipv4OptionSerializers.lookup(option->getType())->serializeOption(option, b, c);
            optionsLength += option->getLength();
        }
    }    // if options present
    if (dgram->getHeaderLength() < IP_HEADER_BYTES + optionsLength)
        throw cRuntimeError("Serializing an IPv4 packet with wrong headerLength value: not enough for store options.\n");
    //padding:
    if (b.getRemainingSize()) {
        b.fillNBytes(b.getRemainingSize(), IPOPTION_END_OF_OPTIONS);
    }
}

void IPv4Serializer::deserializeOptions(IPv4Datagram *dgram, Buffer &b, Context& c)
{
    while (b.getRemainingSize()) {
        unsigned int pos = b.getPos();
        unsigned char type = b.readByte();
        b.seek(pos);
        TLVOptionBase *option = ipv4OptionSerializers.lookup(type)->deserializeOption(b, c);
        dgram->addOption(option);
    }
}

//

IPv4OptionSerializerRegistrationList::~IPv4OptionSerializerRegistrationList()
{
    if (!keyToSerializerMap.empty())
        throw cRuntimeError("SerializerRegistrationList not empty, should call the SerializerRegistrationList::clear() function");
}

void IPv4OptionSerializerRegistrationList::clear()
{
    for (auto elem : keyToSerializerMap) {
        dropAndDelete(elem.second);
    }
    keyToSerializerMap.clear();
}

void IPv4OptionSerializerRegistrationList::add(int id, IPv4OptionSerializerBase *obj)
{
    Key key(id);

    take(obj);
    keyToSerializerMap.insert(std::pair<Key,IPv4OptionSerializerBase*>(key, obj));
}

IPv4OptionSerializerBase *IPv4OptionSerializerRegistrationList::lookup(int id) const
{
    auto it = keyToSerializerMap.find(Key(id));
    return it==keyToSerializerMap.end() ? &defaultSerializer : it->second;
}


} // namespace serializer

} // namespace inet

