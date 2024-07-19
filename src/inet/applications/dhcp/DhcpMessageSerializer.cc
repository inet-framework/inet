//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/dhcp/DhcpMessageSerializer.h"

#include "inet/applications/dhcp/DhcpMessage_m.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"

namespace inet {

Register_Serializer(DhcpMessage, DhcpMessageSerializer);

void DhcpMessageSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& dhcpMessage = staticPtrCast<const DhcpMessage>(chunk);

    // length of a DHCP message without options
    uint16_t length = 236;

    stream.writeByte(dhcpMessage->getOp());
    stream.writeByte(dhcpMessage->getHtype());
    stream.writeByte(dhcpMessage->getHlen());
    stream.writeByte(dhcpMessage->getHops());
    stream.writeUint32Be(dhcpMessage->getXid());
    stream.writeUint16Be(dhcpMessage->getSecs());
    stream.writeBit(dhcpMessage->getBroadcast());
    stream.writeNBitsOfUint64Be(dhcpMessage->getReserved(), 15);
    stream.writeIpv4Address(dhcpMessage->getCiaddr());
    stream.writeIpv4Address(dhcpMessage->getYiaddr());
    // FIXME siaddr is missing from the packet representation
    stream.writeUint32Be(0);
    stream.writeIpv4Address(dhcpMessage->getGiaddr());
    stream.writeMacAddress(dhcpMessage->getChaddr());
    stream.writeByte(0);
    stream.writeByte(0);
    stream.writeUint64Be(0);

    int e = 0;

    const char *sname = dhcpMessage->getSname();
    if (sname != nullptr) {
        size_t snameLen = strlen(sname);
        if (snameLen > 64)
            throw cRuntimeError("Cannot serialize DHCP message: server host name (sname) exceeds the allowed 64 byte limit.");
        stream.writeBytes(reinterpret_cast<const uint8_t*>(sname), snameLen);
        // fill in the remaining bytes
        stream.writeByteRepeatedly(0, 64 - snameLen);
    }

    const char *file = dhcpMessage->getFile();
    if (file != nullptr) {
        size_t fileLen = strlen(file);
        if (fileLen > 128)
            throw cRuntimeError("Cannot serialize DHCP message: file name (file) exceeds the allowed 128 byte limit.");
        stream.writeBytes(reinterpret_cast<const uint8_t*>(file), fileLen);
        // fill in the remaining bytes
        stream.writeByteRepeatedly(0, 128 - fileLen);
    }

    DhcpOptions options = dhcpMessage->getOptions();

    // The first four octets of the 'options' field of the DHCP message
    // contain the (decimal) values 99, 130, 83 and 99, respectively (this
    // is the same magic cookie as is defined in RFC 1497 [17]).

    stream.writeByte(99);
    stream.writeByte(130);
    stream.writeByte(83);
    stream.writeByte(99);
    length += 4;

    // options structure
    // |  Tag   | Length |    Data        |
    // | 1 byte | 1 byte | Length byte(s) |

    // DHCP Message Type
    if (options.getMessageType() != static_cast<inet::DhcpMessageType>(-1)) {
        stream.writeByte(DHCP_MSG_TYPE);
        stream.writeByte(1);
        stream.writeByte(options.getMessageType());
        length += 3;
    }

    // Host Name Option
    const char *hostName = options.getHostName();
    if (hostName != nullptr) {
        stream.writeByte(HOSTNAME);
        uint16_t size = strlen(hostName);
        stream.writeByte(size);
        stream.writeBytes(reinterpret_cast<const uint8_t*>(hostName), size);
        length += (2 + size);
    }

    // Parameter Request List
    if (options.getParameterRequestListArraySize() > 0) {
        stream.writeByte(PARAM_LIST);
        uint16_t size = options.getParameterRequestListArraySize();
        stream.writeByte(size);
        for (size_t i = 0; i < options.getParameterRequestListArraySize(); ++i) {
            stream.writeUint8(options.getParameterRequestList(i));
        }
        length += (2 + size);
    }

    // Client-Identifier
    if (!options.getClientIdentifier().isUnspecified()) {
        stream.writeByte(CLIENT_ID);
        uint8_t size = 1 + 6;
        stream.writeByte(size);
        // TODO arp hardware type?
        stream.writeByte(1);
        stream.writeMacAddress(options.getClientIdentifier());
        length += (2 + size);
    }

    // Requested IP Address
    if (!options.getRequestedIp().isUnspecified()) {
        stream.writeByte(REQUESTED_IP);
        stream.writeByte(4);
        stream.writeIpv4Address(options.getRequestedIp());
        length += 6;
    }

    // Subnet Mask
    if (!options.getSubnetMask().isUnspecified()) {
        stream.writeByte(SUBNET_MASK);
        stream.writeByte(4);
        stream.writeIpv4Address(options.getSubnetMask());
        length += 6;
    }

    // Router
    if (options.getRouterArraySize() > 0) {
        stream.writeByte(ROUTER);
        uint16_t size = options.getRouterArraySize() * 4; // IPv4_ADDRESS_SIZE
        stream.writeByte(size);
        for (size_t i = 0; i < options.getRouterArraySize(); ++i) {
            stream.writeIpv4Address(options.getRouter(i));
        }
        length += (2 + size);
    }

    // Domain Name Server Option
    if (options.getDnsArraySize() > 0) {
        stream.writeByte(DNS);
        uint16_t size = options.getDnsArraySize() * 4; // IPv4_ADDRESS_SIZE
        stream.writeByte(size);
        for (size_t i = 0; i < options.getDnsArraySize(); ++i) {
            stream.writeIpv4Address(options.getDns(i));
        }
        length += (2 + size);
    }

    // Network Time Protocol Servers Option
    if (options.getNtpArraySize() > 0) {
        stream.writeByte(NTP_SRV);
        uint16_t size = options.getNtpArraySize() * 4; // IPv4_ADDRESS_SIZE
        stream.writeByte(size);
        for (size_t i = 0; i < options.getNtpArraySize(); ++i) {
            stream.writeIpv4Address(options.getNtp(i));
        }
        length += (2 + size);
    }

    // Server Identifier
    if (!options.getServerIdentifier().isUnspecified()) {
        stream.writeByte(SERVER_ID);
        stream.writeByte(4);
        stream.writeIpv4Address(options.getServerIdentifier());
        length += 6;
    }

    // Renewal (T1) Time Value
    if (!options.getRenewalTime().isZero()) {
        stream.writeByte(RENEWAL_TIME);
        stream.writeByte(4);
        stream.writeUint32Be(options.getRenewalTime().inUnit(SIMTIME_S));
        length += 6;
    }

    // Rebinding (T2) Time Value
    if (!options.getRebindingTime().isZero()) {
        stream.writeByte(REBIND_TIME);
        stream.writeByte(4);
        stream.writeUint32Be(options.getRebindingTime().inUnit(SIMTIME_S));
        length += 6;
    }

    // IP Address Lease Time
    if (!options.getLeaseTime().isZero()) {
        stream.writeByte(LEASE_TIME);
        stream.writeByte(4);
        stream.writeUint32Be(options.getLeaseTime().inUnit(SIMTIME_S));
        length += 6;
    }

    // End Option
    stream.writeByte(255);
    length += 1;

    ASSERT(dhcpMessage->getChunkLength() == B(length));
}

const Ptr<Chunk> DhcpMessageSerializer::deserialize(MemoryInputStream& stream) const
{
    auto dhcpMessage = makeShared<DhcpMessage>();

    int length = 236;

    dhcpMessage->setOp((stream.readByte() == 1 ? BOOTREQUEST : BOOTREPLY));
    dhcpMessage->setHtype(stream.readByte());
    dhcpMessage->setHlen(stream.readByte());
    dhcpMessage->setHops(stream.readByte());
    dhcpMessage->setXid(stream.readUint32Be());
    dhcpMessage->setSecs(stream.readUint16Be());
    dhcpMessage->setBroadcast(stream.readBit());
    dhcpMessage->setReserved(stream.readNBitsToUint64Be(15));
    dhcpMessage->setCiaddr(stream.readIpv4Address());
    dhcpMessage->setYiaddr(stream.readIpv4Address());
    // FIXME siaddr is missing from the packet representation
    stream.readUint32Be();
    dhcpMessage->setGiaddr(stream.readIpv4Address());
    dhcpMessage->setChaddr(stream.readMacAddress());
    stream.readByte();
    stream.readByte();
    stream.readUint64Be();

    char sname[64];
    stream.readBytes(reinterpret_cast<uint8_t*>(sname), 64);
    dhcpMessage->setSname(sname);

    char file[128];
    stream.readBytes(reinterpret_cast<uint8_t*>(file), 128);
    dhcpMessage->setFile(file);

    // The first four octets of the 'options' field of the DHCP message
    // contain the (decimal) values 99, 130, 83 and 99, respectively (this
    // is the same magic cookie as is defined in RFC 1497 [17]).
    uint8_t b1 = stream.readByte();
    uint8_t b2 = stream.readByte();
    uint8_t b3 = stream.readByte();
    uint8_t b4 = stream.readByte();
    ASSERT(b1 == 99 && b2 == 130 && b3 == 83 && b4 == 99);
    length += 4;

    DhcpOptions& options = dhcpMessage->getOptionsForUpdate();
    uint8_t code = stream.readByte();

    while (code != 255) {
        switch (code) {
            case DHCP_MSG_TYPE: {
                uint8_t size = stream.readByte();
                ASSERT(size == 1);
                options.setMessageType((DhcpMessageType)stream.readByte());
                length += 2 + size;
                break;
            }
            case HOSTNAME: {
                uint8_t size = stream.readByte();
                std::vector<uint8_t> hostName(size);
                stream.readBytes(hostName.data(), size);
                options.setHostName(reinterpret_cast<char*>(hostName.data()));
                length += 2 + size;
                break;
            }
            case PARAM_LIST: {
                uint8_t size = stream.readByte();
                options.setParameterRequestListArraySize(size);
                for (uint8_t i = 0; i < size; ++i) {
                    options.setParameterRequestList(i, static_cast<DhcpOptionCode>(stream.readUint8()));
                }
                length += 2 + size;
                break;
            }
            case CLIENT_ID: {
                uint8_t size = stream.readByte();
                uint8_t type = stream.readByte();
                ASSERT(size == 7 && type == 1);
                options.setClientIdentifier(stream.readMacAddress());
                length += 2 + size;
                break;
            }
            case REQUESTED_IP: {
                uint8_t size = stream.readByte();
                ASSERT(size == 4); // IPv4_ADDRESS_SIZE
                options.setRequestedIp(stream.readIpv4Address());
                length += 2 + size;
                break;
            }
            case SUBNET_MASK: {
                uint8_t size = stream.readByte();
                ASSERT(size == 4); // IPv4_ADDRESS_SIZE
                options.setSubnetMask(stream.readIpv4Address());
                length += 2 + size;
                break;
            }
            case ROUTER: {
                uint8_t size = stream.readByte();
                ASSERT((size % 4) == 0); // n * IPv4_ADDRESS_SIZE
                uint8_t dim = size / 4; // IPv4_ADDRESS_SIZE
                options.setRouterArraySize(dim);
                for (uint8_t i = 0; i < dim; ++i) {
                    options.setRouter(i, stream.readIpv4Address());
                }
                length += 2 + size;
                break;
            }
            case DNS: {
                uint8_t size = stream.readByte();
                ASSERT((size % 4) == 0); // n * IPv4_ADDRESS_SIZE
                uint8_t dim = size / 4; // IPv4_ADDRESS_SIZE
                options.setDnsArraySize(dim);
                for (uint8_t i = 0; i < dim; ++i) {
                    options.setDns(i, stream.readIpv4Address());
                }
                length += 2 + size;
                break;
            }
            case NTP_SRV: {
                uint8_t size = stream.readByte();
                ASSERT((size % 4) == 0); // n * IPv4_ADDRESS_SIZE
                uint8_t dim = size / 4; // IPv4_ADDRESS_SIZE
                options.setNtpArraySize(dim);
                for (uint8_t i = 0; i < dim; ++i) {
                    options.setNtp(i, stream.readIpv4Address());
                }
                length += 2 + size;
                break;
            }
            case SERVER_ID: {
                uint8_t size = stream.readByte();
                ASSERT(size == 4); // IPv4_ADDRESS_SIZE
                options.setServerIdentifier(stream.readIpv4Address());
                length += 2 + size;
                break;
            }
            case RENEWAL_TIME: {
                uint8_t size = stream.readByte();
                ASSERT(size == 4); // Time size
                options.setRenewalTime(SimTime(stream.readUint32Be(), SIMTIME_S));
                length += 2 + size;
                break;
            }
            case REBIND_TIME: {
                uint8_t size = stream.readByte();
                ASSERT(size == 4); // Time size
                options.setRebindingTime(SimTime(stream.readUint32Be(), SIMTIME_S));
                length += 2 + size;
                break;
            }
            case LEASE_TIME: {
                uint8_t size = stream.readByte();
                ASSERT(size == 4); // Time size
                options.setLeaseTime(SimTime(stream.readUint32Be(), SIMTIME_S));
                length += 2 + size;
                break;
            }
            default: {
                uint8_t size = stream.readByte();
                dhcpMessage->markIncorrect();
                std::vector<uint8_t> buffer(size);
                stream.readBytes(buffer.data(), size);
                length += 2 + size;
                break;
            }
        }
        code = stream.readByte();
    }
    ++length;

    dhcpMessage->setChunkLength(B(length));
    return dhcpMessage;
}

} // namespace inet

