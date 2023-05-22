//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
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

#ifndef INET_NETWORKLAYER_IPV4_IPSEC_PACKETINFO_H_
#define INET_NETWORKLAYER_IPV4_IPSEC_PACKETINFO_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"

namespace inet {
namespace ipsec {

/**
 * Contains the values of selected fields in a packet. PacketSelector performs
 * matching on instances of PacketInfo instead of packets themselves.
 */
class INET_API PacketInfo
{
  private:
    IPv4Address localAddress;
    IPv4Address remoteAddress;
    unsigned int nextProtocol = 0; // next layer protocol
    unsigned int localPort = 0;
    unsigned int remotePort = 0;
    unsigned int icmpType = 0;
    unsigned int icmpCode = 0;
    bool tfcSupported = false;

  public:
    PacketInfo() {}
    const IPv4Address& getLocalAddress() const { return localAddress; }
    void setLocalAddress(const IPv4Address &localAddress) { this->localAddress = localAddress; }
    const IPv4Address& getRemoteAddress() const { return remoteAddress; }
    void setRemoteAddress(const IPv4Address &remoteAddress) { this->remoteAddress = remoteAddress; }
    unsigned int getNextProtocol() const { return nextProtocol; }
    void setNextProtocol(unsigned int nextProtocol) { this->nextProtocol = nextProtocol; }
    unsigned int getLocalPort() const { return localPort; }
    void setLocalPort(unsigned int localPort) { this->localPort = localPort; }
    unsigned int getRemotePort() const { return remotePort; }
    void setRemotePort(unsigned int remotePort) { this->remotePort = remotePort; }
    unsigned int getIcmpType() const { return icmpType; }
    void setIcmpType(unsigned int icmpType) { this->icmpType = icmpType; }
    unsigned int getIcmpCode() const { return icmpCode; }
    void setIcmpCode(unsigned int icmpCode) { this->icmpCode = icmpCode; }
    bool isTfcSupported() const { return tfcSupported; }
    void setTfcSupported(bool tfcSupported) { this->tfcSupported = tfcSupported; }
    std::string str() const;
};

inline std::ostream& operator<<(std::ostream& os, const PacketInfo& e)
{
    return os << e.str();
}

}  // namespace ipsec
}  // namespace inet

#endif /* INET_NETWORKLAYER_IPV4_IPSEC_PACKETINFO_H_ */

