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

#include "inet/common/Endian.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/ipv4/Ipv4HeaderSerializer.h"
#include "inet/networklayer/ipv4/headers/ip.h"

namespace inet {

Register_Serializer(Ipv4Header, Ipv4HeaderSerializer);

void Ipv4HeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    auto startPosition = stream.getLength();
    struct ip iphdr;
    const auto& ipv4Header = staticPtrCast<const Ipv4Header>(chunk);
    B headerLength = ipv4Header->getHeaderLength();
    ASSERT((headerLength.get() & 3) == 0 && headerLength >= IPv4_MIN_HEADER_LENGTH && headerLength <= IPv4_MAX_HEADER_LENGTH);
    ASSERT(headerLength <= ipv4Header->getTotalLengthField());
    iphdr.ip_hl = B(headerLength).get() >> 2;
    iphdr.ip_v = ipv4Header->getVersion();
    iphdr.ip_tos = ipv4Header->getTypeOfService();
    iphdr.ip_id = htons(ipv4Header->getIdentification());
    ASSERT((ipv4Header->getFragmentOffset() & 7) == 0);
    uint16_t ip_off = (ipv4Header->getFragmentOffset() / 8) & IP_OFFMASK;
    if (ipv4Header->getReservedBit())
        ip_off |= IP_RF;
    if (ipv4Header->getMoreFragments())
        ip_off |= IP_MF;
    if (ipv4Header->getDontFragment())
        ip_off |= IP_DF;
    iphdr.ip_off = htons(ip_off);
    iphdr.ip_ttl = ipv4Header->getTimeToLive();
    iphdr.ip_p = ipv4Header->getProtocolId();
    iphdr.ip_src.s_addr = htonl(ipv4Header->getSrcAddress().getInt());
    iphdr.ip_dst.s_addr = htonl(ipv4Header->getDestAddress().getInt());
    iphdr.ip_len = htons(B(ipv4Header->getTotalLengthField()).get());
    if (ipv4Header->getCrcMode() != CRC_COMPUTED)
        throw cRuntimeError("Cannot serialize Ipv4 header without a properly computed CRC");
    iphdr.ip_sum = htons(ipv4Header->getCrc());
    stream.writeBytes((uint8_t *)&iphdr, IPv4_MIN_HEADER_LENGTH);

    if (headerLength > IPv4_MIN_HEADER_LENGTH) {
        unsigned short numOptions = ipv4Header->getOptionArraySize();
        B optionsLength = B(0);
        if (numOptions > 0) {    // options present?
            for (unsigned short i = 0; i < numOptions; i++) {
                const TlvOptionBase *option = &ipv4Header->getOption(i);
                serializeOption(stream, option);
                optionsLength += B(option->getLength());
            }
        }    // if options present
        if (ipv4Header->getHeaderLength() < IPv4_MIN_HEADER_LENGTH + optionsLength)
            throw cRuntimeError("Serializing an Ipv4 packet with wrong headerLength value: not enough for store options.\n");
        auto writtenLength = B(stream.getLength() - startPosition);
        if (writtenLength < headerLength)
            stream.writeByteRepeatedly(IPOPTION_END_OF_OPTIONS, B(headerLength - writtenLength).get());
    }
}

void Ipv4HeaderSerializer::serializeOption(MemoryOutputStream& stream, const TlvOptionBase *option) const
{
    unsigned short type = option->getType();
    unsigned short length = option->getLength();    // length >= 1

    stream.writeByte(type);
    if (length > 1)
        stream.writeByte(length);

    auto *opt = dynamic_cast<const TlvOptionRaw *>(option);
    if (opt) {
        unsigned int datalen = opt->getBytesArraySize();
        ASSERT(length == 2 + datalen);
        for (unsigned int i = 0; i < datalen; i++)
            stream.writeByte(opt->getBytes(i));
        return;
    }

    switch (type) {
        case IPOPTION_END_OF_OPTIONS:
            check_and_cast<const Ipv4OptionEnd *>(option);
            ASSERT(length == 1);
            break;

        case IPOPTION_NO_OPTION:
            check_and_cast<const Ipv4OptionNop *>(option);
            ASSERT(length == 1);
            break;

        case IPOPTION_STREAM_ID: {
            auto *opt = check_and_cast<const Ipv4OptionStreamId *>(option);
            ASSERT(length == 4);
            stream.writeUint16Be(opt->getStreamId());
            break;
        }

        case IPOPTION_TIMESTAMP: {
            auto *opt = check_and_cast<const Ipv4OptionTimestamp *>(option);
            int bytes = (opt->getFlag() == IP_TIMESTAMP_TIMESTAMP_ONLY) ? 4 : 8;
            ASSERT(length == 4 + bytes * opt->getRecordTimestampArraySize());
            uint8_t pointer = 5 + opt->getNextIdx() * bytes;
            stream.writeByte(pointer);
            uint8_t flagbyte = opt->getOverflow() << 4 | opt->getFlag();
            stream.writeByte(flagbyte);
            for (unsigned int count = 0; count < opt->getRecordTimestampArraySize(); count++) {
                if (bytes == 8)
                    stream.writeIpv4Address(opt->getRecordAddress(count));
                stream.writeUint32Be(opt->getRecordTimestamp(count).inUnit(SIMTIME_MS));
            }
            break;
        }

        case IPOPTION_RECORD_ROUTE:
        case IPOPTION_LOOSE_SOURCE_ROUTING:
        case IPOPTION_STRICT_SOURCE_ROUTING: {
            auto *opt = check_and_cast<const Ipv4OptionRecordRoute *>(option);
            ASSERT(length == 3 + 4 * opt->getRecordAddressArraySize());
            uint8_t pointer = 4 + opt->getNextAddressIdx() * 4;
            stream.writeByte(pointer);
            for (unsigned int count = 0; count < opt->getRecordAddressArraySize(); count++) {
                stream.writeIpv4Address(opt->getRecordAddress(count));
            }
            break;
        }
        case IPOPTION_ROUTER_ALERT: {
            auto *opt = check_and_cast<const Ipv4OptionRouterAlert *>(option);
            ASSERT(length == 4);
            stream.writeUint16Be(opt->getRouterAlert());
            break;
        }

        case IPOPTION_SECURITY:
        default: {
            throw cRuntimeError("Unknown Ipv4Option type=%d (not in an TlvOptionRaw option)", type);
            break;
        }
    }
}

const Ptr<Chunk> Ipv4HeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto position = stream.getPosition();
    B bufsize = stream.getRemainingLength();
    uint8_t buffer[B(IPv4_MIN_HEADER_LENGTH).get()];
    stream.readBytes(buffer, IPv4_MIN_HEADER_LENGTH);
    auto ipv4Header = makeShared<Ipv4Header>();
    const struct ip& iphdr = *static_cast<const struct ip *>((void *)&buffer);
    B totalLength, headerLength;

    ipv4Header->setVersion(iphdr.ip_v);
    ipv4Header->setHeaderLength(IPv4_MIN_HEADER_LENGTH);
    ipv4Header->setSrcAddress(Ipv4Address(ntohl(iphdr.ip_src.s_addr)));
    ipv4Header->setDestAddress(Ipv4Address(ntohl(iphdr.ip_dst.s_addr)));
    ipv4Header->setProtocolId((IpProtocolId)iphdr.ip_p);
    ipv4Header->setTimeToLive(iphdr.ip_ttl);
    ipv4Header->setIdentification(ntohs(iphdr.ip_id));
    uint16_t ip_off = ntohs(iphdr.ip_off);
    ipv4Header->setReservedBit((ip_off & IP_RF) != 0);
    ipv4Header->setMoreFragments((ip_off & IP_MF) != 0);
    ipv4Header->setDontFragment((ip_off & IP_DF) != 0);
    ipv4Header->setFragmentOffset((ntohs(iphdr.ip_off) & IP_OFFMASK) * 8);
    ipv4Header->setTypeOfService(iphdr.ip_tos);
    totalLength = B(ntohs(iphdr.ip_len));
    ipv4Header->setTotalLengthField(totalLength);
    headerLength = B(iphdr.ip_hl << 2);

    if (iphdr.ip_v != 4)
        ipv4Header->markIncorrect();
    if (headerLength < IPv4_MIN_HEADER_LENGTH) {
        ipv4Header->markIncorrect();
        headerLength = IPv4_MIN_HEADER_LENGTH;
    }
    if (totalLength < headerLength)
        ipv4Header->markIncorrect();

    ipv4Header->setHeaderLength(headerLength);

    if (headerLength > IPv4_MIN_HEADER_LENGTH) {    // options present?
        while (stream.getRemainingLength() > B(0) && stream.getPosition() - position < headerLength) {
            TlvOptionBase *option = deserializeOption(stream);
            ipv4Header->addOption(option);
        }
    }
    if (headerLength > bufsize) {
        ipv4Header->markIncomplete();
    }

    ipv4Header->setCrc(ntohs(iphdr.ip_sum));
    ipv4Header->setCrcMode(CRC_COMPUTED);

    return ipv4Header;
}

TlvOptionBase *Ipv4HeaderSerializer::deserializeOption(MemoryInputStream& stream) const
{
    auto position = stream.getPosition();
    unsigned char type = stream.readByte();
    unsigned char length = 1;

    switch (type) {
        case IPOPTION_END_OF_OPTIONS:    // EOL
            return new Ipv4OptionEnd();

        case IPOPTION_NO_OPTION:    // NOP
            return new Ipv4OptionNop();

        case IPOPTION_STREAM_ID:
            length = stream.readByte();
            if (length == 4) {
                auto *option = new Ipv4OptionStreamId();
                option->setType(type);
                option->setLength(length);
                option->setStreamId(stream.readUint16Be());
                return option;
            }
            break;

        case IPOPTION_TIMESTAMP: {
            length = stream.readByte();
            uint8_t pointer = stream.readByte();
            uint8_t flagbyte = stream.readByte();
            uint8_t overflow = flagbyte >> 4;
            TimestampFlag flag = static_cast<TimestampFlag>(-1);
            int bytes = 0;
            switch (flagbyte & 0x0f) {
                case 0: flag = IP_TIMESTAMP_TIMESTAMP_ONLY; bytes = 4; break;
                case 1: flag = IP_TIMESTAMP_WITH_ADDRESS; bytes = 8; break;
                case 3: flag = IP_TIMESTAMP_SENDER_INIT_ADDRESS; bytes = 8; break;
                default: break;
            }
            if (flag != static_cast<TimestampFlag>(-1) && length > 4 && bytes && ((length-4) % bytes) == 0 && pointer >= 5 && ((pointer-5) % bytes) == 0) {
                auto *option = new Ipv4OptionTimestamp();
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
                        option->setRecordAddress(count, stream.readIpv4Address());
                    option->setRecordTimestamp(count, SimTime(stream.readUint32Be(), SIMTIME_MS));
                }
                return option;
            }
            break;
        }

        case IPOPTION_RECORD_ROUTE:
        case IPOPTION_LOOSE_SOURCE_ROUTING:
        case IPOPTION_STRICT_SOURCE_ROUTING: {
            length = stream.readByte();
            uint8_t pointer = stream.readByte();
            if (length > 3 && (length % 4) == 3 && pointer >= 4 && (pointer % 4) == 0) {
                auto *option = new Ipv4OptionRecordRoute();
                option->setType(type);
                option->setLength(length);
                option->setRecordAddressArraySize((length - 3) / 4);
                option->setNextAddressIdx((pointer-4) / 4);
                for (unsigned int count = 0; count < option->getRecordAddressArraySize(); count++) {
                    option->setRecordAddress(count, stream.readIpv4Address());
                }
                return option;
            }
            break;
        }

        case IPOPTION_ROUTER_ALERT:
            length = stream.readByte();
            if (length == 4) {
                auto *option = new Ipv4OptionRouterAlert();
                option->setType(type);
                option->setLength(length);
                option->setRouterAlert(stream.readUint16Be());
                return option;
            }
            break;

        case IPOPTION_SECURITY:
        default:
            length = stream.readByte();
            break;
    }    // switch

    auto *option = new TlvOptionRaw();
    stream.seek(position);
    type = stream.readByte();
    length = stream.readByte();
    option->setType(type);
    option->setLength(length);
    if (length > 2)
        option->setBytesArraySize(length - 2);
    for (unsigned int i = 2; i < length; i++)
        option->setBytes(i-2, stream.readByte());
    return option;
}

} // namespace inet

