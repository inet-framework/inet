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

#ifndef INET_NETWORKLAYER_IPV4_IPSEC_PACKETSELECTOR_H_
#define INET_NETWORKLAYER_IPV4_IPSEC_PACKETSELECTOR_H_

#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/ipv4/ipsec/PacketInfo.h"
#include "inet/networklayer/ipv4/ipsec/rangelist.h"

namespace inet {
namespace ipsec {

/**
 * IPsec selector that can match packets based on local/remote address,
 * protocol, local/remote port (for TCP/UDP), and type/code (for ICMP).
 *
 * All selector fields can accept multiple values and value ranges.
 */
class INET_API PacketSelector
{
  private:
    rangelist<IPv4Address> localAddress;
    rangelist<IPv4Address> remoteAddress;
    rangelist<unsigned int> nextProtocol; // next layer protocol
    rangelist<unsigned int> localPort;
    rangelist<unsigned int> remotePort;
    rangelist<unsigned int> icmpType;
    rangelist<unsigned int> icmpCode;

  public:
    PacketSelector() {}
    const rangelist<IPv4Address>& getLocalAddress() const { return localAddress; }
    void setLocalAddress(const rangelist<IPv4Address> &localAddress) { this->localAddress = localAddress; }
    const rangelist<IPv4Address>& getRemoteAddress() const { return remoteAddress; }
    void setRemoteAddress(const rangelist<IPv4Address> &remoteAddress) { this->remoteAddress = remoteAddress; }
    const rangelist<unsigned int>& getNextProtocol() const { return nextProtocol; }
    void setNextProtocol(const rangelist<unsigned int>& nextProtocol) { this->nextProtocol = nextProtocol; }
    const rangelist<unsigned int>& getLocalPort() const { return localPort; }
    void setLocalPort(const rangelist<unsigned int>& localPort) { this->localPort = localPort; }
    const rangelist<unsigned int>& getRemotePort() const { return remotePort; }
    void setRemotePort(const rangelist<unsigned int>& remotePort) { this->remotePort = remotePort; }
    const rangelist<unsigned int>& getIcmpType() const { return icmpType; }
    void setIcmpType(const rangelist<unsigned int>& icmpType) { this->icmpType = icmpType; }
    const rangelist<unsigned int>& getIcmpCode() const { return icmpCode; }
    void setIcmpCode(const rangelist<unsigned int>& icmpCode) { this->icmpCode = icmpCode; }
    bool matches(const PacketInfo *packet) const;
    std::string str() const;
};

inline std::ostream& operator<<(std::ostream& os, const PacketSelector& e)
{
    return os << e.str();
}

}  // namespace ipsec
}  // namespace inet

#endif /* INET_NETWORKLAYER_IPV4_IPSEC_PACKETSELECTOR_H_ */

