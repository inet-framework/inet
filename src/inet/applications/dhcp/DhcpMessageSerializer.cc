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

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/applications/dhcp/DhcpMessage_m.h"
#include "inet/applications/dhcp/DhcpMessageSerializer.h"

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
    stream.writeNBitsOfUint64Be(dhcpMessage->getMbz(), 15);
    stream.writeIpv4Address(dhcpMessage->getCiaddr());
    stream.writeIpv4Address(dhcpMessage->getYiaddr());
    // FIXME: siaddr is missing from the packet representation
    stream.writeUint32Be(0);
    stream.writeIpv4Address(dhcpMessage->getGiaddr());
    stream.writeNBitsOfUint64Be(dhcpMessage->getChaddr().getInt(), 48);
    stream.writeByte(0);
    stream.writeByte(0);
    stream.writeUint64Be(0);

    int e = 0;

    const char* sname = dhcpMessage->getSname();
    if(sname != nullptr){
        if(sizeof(sname)/sizeof(sname[0]) > 64)
            throw cRuntimeError("Cannot serialize DHCP message: server host name (sname) exceeds the allowed 16 byte limit.");
        // write the string until '\0'
        for(e = 0; sname[e] != '\0'; ++e){
            stream.writeByte(sname[e]);
        }
        // write the '\0'
        stream.writeByte('\0');
        // fill in the remaining bytes
        stream.writeByteRepeatedly(0, 64 - (e + 1));
    }

    const char* file = dhcpMessage->getFile();
    if(file != nullptr){
        if(sizeof(file)/sizeof(file[0]) > 128)
            throw cRuntimeError("Cannot serialize DHCP message: file name (file) exceeds the allowed 128 byte limit.");
        // write the string until '\0'
        for(e = 0; file[e] != '\0'; ++e){
            stream.writeByte(file[e]);
        }
        // write the '\0'
        stream.writeByte('\0');
        // fill in the remaining bytes
        stream.writeByteRepeatedly(0, 128 - (e + 1));
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
    if(options.getMessageType() != static_cast<inet::DhcpMessageType>(-1)){
        stream.writeByte(DHCP_MSG_TYPE);
        stream.writeByte(1);
        stream.writeByte(options.getMessageType());
        length += 3;
    }

    // Host Name Option
    const char* hostName = options.getHostName();
    // FIXME: nullptr and strcmp does not seem to work
    if(hostName != nullptr && false){
        stream.writeByte(HOSTNAME);
        uint16_t size = sizeof(hostName) / sizeof(hostName[0]) - 1;
        stream.writeByte(size);
        for(size_t i = 0; hostName[i] != '\0'; ++i){
            stream.writeByte(hostName[i]);
        }
        length += (2 + size);
    }

    // Parameter Request List
    if(options.getParameterRequestListArraySize() > 0){
        stream.writeByte(PARAM_LIST);
        uint16_t size = options.getParameterRequestListArraySize() * sizeof(uint64_t);
        stream.writeByte(size);
        for(size_t i = 0; i < options.getParameterRequestListArraySize(); ++i){
            stream.writeUint64Be(options.getParameterRequestList(i));
        }
        length += (2 + size);
    }

    // Client-Identifier
    if(!options.getClientIdentifier().isUnspecified()){
        stream.writeByte(CLIENT_ID);
        uint8_t size = 1 + 6;
        stream.writeByte(size);
        // TODO: arp hardware type?
        stream.writeByte(1);
        stream.writeNBitsOfUint64Be(options.getClientIdentifier().getInt(), 48);
        length += (2 + size);
    }

    // Requested IP Address
    if(!options.getRequestedIp().isUnspecified()){
        stream.writeByte(REQUESTED_IP);
        stream.writeByte(4);
        stream.writeIpv4Address(options.getRequestedIp());
        length += 6;
    }

    // Subnet Mask
    if(!options.getSubnetMask().isUnspecified()){
        stream.writeByte(SUBNET_MASK);
        stream.writeByte(4);
        stream.writeIpv4Address(options.getSubnetMask());
        length += 6;
    }

    // Router
    if(options.getRouterArraySize() > 0){
        stream.writeByte(ROUTER);
        uint16_t size = options.getRouterArraySize() * sizeof(uint32_t);
        stream.writeByte(size);
        for(size_t i = 0; i < options.getRouterArraySize(); ++i){
            stream.writeIpv4Address(options.getRouter(i));
        }
        length += (2 + size);
    }

    // Domain Name Server Option
    if(options.getDnsArraySize() > 0){
        stream.writeByte(DNS);
        uint16_t size = options.getDnsArraySize() * sizeof(uint32_t);
        stream.writeByte(size);
        for(size_t i = 0; i < options.getDnsArraySize(); ++i){
            stream.writeIpv4Address(options.getDns(i));
        }
        length += (2 + size);
    }

    // Network Time Protocol Servers Option
    if(options.getNtpArraySize() > 0){
        stream.writeByte(NTP_SRV);
        uint16_t size = options.getNtpArraySize() * sizeof(uint32_t);
        stream.writeByte(size);
        for(size_t i = 0; i < options.getNtpArraySize(); ++i){
            stream.writeIpv4Address(options.getNtp(i));
        }
        length += (2 + size);
    }

    // Server Identifier
    if(!options.getServerIdentifier().isUnspecified()){
        stream.writeByte(SERVER_ID);
        stream.writeByte(4);
        stream.writeIpv4Address(options.getServerIdentifier());
        length += 6;
    }

    // Renewal (T1) Time Value
    if(!options.getRenewalTime().isZero()){
        stream.writeByte(RENEWAL_TIME);
        stream.writeByte(4);
        stream.writeUint32Be(options.getRenewalTime().inUnit(SIMTIME_S));
        length += 6;
    }

    // Rebinding (T2) Time Value
    if(!options.getRebindingTime().isZero()){
        stream.writeByte(REBIND_TIME);
        stream.writeByte(4);
        stream.writeUint32Be(options.getRebindingTime().inUnit(SIMTIME_S));
        length += 6;
    }

    // IP Address Lease Time
    if(!options.getLeaseTime().isZero()){
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
    dhcpMessage->setMbz(stream.readNBitsToUint64Be(15));
    dhcpMessage->setCiaddr(Ipv4Address(stream.readIpv4Address()));
    dhcpMessage->setYiaddr(Ipv4Address(stream.readIpv4Address()));
    // FIXME: siaddr is missing from the packet representation
    stream.readUint32Be();
    dhcpMessage->setGiaddr(Ipv4Address(stream.readIpv4Address()));
    dhcpMessage->setChaddr(MacAddress(stream.readNBitsToUint64Be(48)));
    stream.readByte();
    stream.readByte();
    stream.readUint64Be();

    char sname[64];
    for(int i = 0; i < 64; ++i){
        sname[i] = stream.readByte();
    }
    dhcpMessage->setSname(sname);

    char file[128];
    for(int i = 0; i < 128; ++i){
        file[i] = stream.readByte();
    }
    dhcpMessage->setFile(file);

    // The first four octets of the 'options' field of the DHCP message
    // contain the (decimal) values 99, 130, 83 and 99, respectively (this
    // is the same magic cookie as is defined in RFC 1497 [17]).

    stream.readByte();
    stream.readByte();
    stream.readByte();
    stream.readByte();
    length += 4;

    DhcpOptions options;
    uint8_t code = stream.readByte();

    while(code != 255){
        switch(code){
            case DHCP_MSG_TYPE: {
                stream.readByte();
                options.setMessageType((DhcpMessageType)stream.readByte());
                length += 3;
                break;
            }
            case HOSTNAME: {
                uint8_t size = stream.readByte();
                char* hostName = new char[size + 1];
                uint8_t i = 0;
                for(; i < size; ++i){
                    hostName[i] = stream.readByte();
                }
                hostName[i] = '\0';
                options.setHostName(hostName);
                delete[] hostName;
                length += (2 + size);
                break;
            }
            case PARAM_LIST: {
                uint8_t size = stream.readByte() / sizeof(uint64_t);
                options.setParameterRequestListArraySize(size);
                for(uint8_t i = 0; i < size; ++i){
                    options.setParameterRequestList(i, stream.readUint64Be());
                }
                length += (2 + size);
                break;
            }
            case CLIENT_ID: {
                stream.readByte();
                // TODO: arp hardware type?
                stream.readByte();
                options.setClientIdentifier(MacAddress(stream.readNBitsToUint64Be(48)));
                length += 9;
                break;
            }
            case REQUESTED_IP: {
                stream.readByte();
                options.setRequestedIp(Ipv4Address(stream.readIpv4Address()));
                length += 6;
                break;
            }
            case SUBNET_MASK: {
                stream.readByte();
                options.setSubnetMask(Ipv4Address(stream.readIpv4Address()));
                length += 6;
                break;
            }
            case ROUTER: {
                uint8_t size = stream.readByte() / sizeof(uint32_t);
                options.setRouterArraySize(size);
                for(uint8_t i = 0; i < size; ++i){
                    options.setRouter(i, Ipv4Address(stream.readIpv4Address()));
                }
                length += (2 + size * sizeof(uint32_t));
                break;
            }
            case DNS: {
                uint8_t size = stream.readByte() / sizeof(uint32_t);
                options.setDnsArraySize(size);
                for(uint8_t i = 0; i < size; ++i){
                    options.setDns(i, Ipv4Address(stream.readIpv4Address()));
                }
                length += (2 + size * sizeof(uint32_t));
                break;
            }
            case NTP_SRV: {
                uint8_t size = stream.readByte() / sizeof(uint32_t);
                options.setNtpArraySize(size);
                for(uint8_t i = 0; i < size; ++i){
                    options.setNtp(i, Ipv4Address(stream.readIpv4Address()));
                }
                length += (2 + size * sizeof(uint32_t));
                break;
            }
            case SERVER_ID: {
                stream.readByte();
                options.setServerIdentifier(Ipv4Address(stream.readIpv4Address()));
                length += 6;
                break;
            }
            case RENEWAL_TIME: {
                stream.readByte();
                options.setRenewalTime(SimTime(stream.readUint32Be(), SIMTIME_S));
                length += 6;
                break;
            }
            case REBIND_TIME: {
                stream.readByte();
                options.setRebindingTime(SimTime(stream.readUint32Be(), SIMTIME_S));
                length += 6;
                break;
            }
            case LEASE_TIME: {
                stream.readByte();
                options.setLeaseTime(SimTime(stream.readUint32Be(), SIMTIME_S));
                length += 6;
                break;
            }
            default: {
                dhcpMessage->markIncorrect();
                break;
            }
        }
        code = stream.readByte();
    }
    dhcpMessage->setOptions(options);
    ++length;

    dhcpMessage->setChunkLength(B(length));
    return dhcpMessage;
}

} // namespace inet


















