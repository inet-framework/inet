//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IPV6DATAGRAM_H
#define __INET_IPV6DATAGRAM_H

#include <list>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/INetworkDatagram.h"
#include "inet/networklayer/ipv6/IPv6Datagram_m.h"

namespace inet {

/**
 * Represents an IPv6 datagram. More info in the IPv6Datagram.msg file
 * (and the documentation generated from it).
 */
class INET_API IPv6Datagram : public IPv6Datagram_Base, public INetworkDatagram
{
  protected:
    typedef std::vector<IPv6ExtensionHeader *> ExtensionHeaders;
    ExtensionHeaders extensionHeaders;

  private:
    void copy(const IPv6Datagram& other);
    void clean();
    int getExtensionHeaderOrder(IPv6ExtensionHeader *eh);

  public:
    IPv6Datagram(const char *name = NULL, int kind = 0) : IPv6Datagram_Base(name, kind) {}
    IPv6Datagram(const IPv6Datagram& other) : IPv6Datagram_Base(other) { copy(other); }
    IPv6Datagram& operator=(const IPv6Datagram& other);
    ~IPv6Datagram();

    virtual IPv6Datagram *dup() const { return new IPv6Datagram(*this); }

    /**
     * Returns bits 0-5 of the Traffic Class field, a value in the 0..63 range
     */
    virtual int getDiffServCodePoint() const { return getTrafficClass() & 0x3f; }

    /**
     * Sets bits 0-5 of the Traffic Class field; expects a value in the 0..63 range
     */
    virtual void setDiffServCodePoint(int dscp) { setTrafficClass((getTrafficClass() & 0xc0) | (dscp & 0x3f)); }

    /**
     * Returns bits 6-7 of the Traffic Class field, a value in the range 0..3
     */
    virtual int getExplicitCongestionNotification() const { return (getTrafficClass() >> 6) & 0x03; }

    /**
     * Sets bits 6-7 of the Traffic Class field; expects a value in the 0..3 range
     */
    virtual void setExplicitCongestionNotification(int ecn) { setTrafficClass((getTrafficClass() & 0x3f) | ((ecn & 0x3) << 6)); }

    /** Generated but unused method, should not be called. */
    virtual void setExtensionHeaderArraySize(unsigned int size);

    /** Generated but unused method, should not be called. */
    virtual void setExtensionHeader(unsigned int k, const IPv6ExtensionHeaderPtr& extensionHeader_var);

    /**
     * Returns the number of extension headers in this datagram
     */
    virtual unsigned int getExtensionHeaderArraySize() const;

    /**
     * Returns the kth extension header in this datagram
     */
    virtual IPv6ExtensionHeaderPtr& getExtensionHeader(unsigned int k);

    /**
     * Returns the extension header of the specified type,
     * or NULL. If index is 0, then the first, if 1 then the
     * second extension is returned. (The datagram might
     * contain two Destination Options extension.)
     */
    virtual IPv6ExtensionHeader *findExtensionHeaderByType(IPProtocolId extensionType, int index = 0) const;

    /**
     * Adds an extension header to the datagram.
     * The atPos parameter should not be used, the extension
     * headers are stored in the order specified in RFC 2460 4.1.
     */
    virtual void addExtensionHeader(IPv6ExtensionHeader *eh, int atPos = -1);

    /**
     * Calculates the length of the IPv6 header plus the extension
     * headers.
     */
    virtual int calculateHeaderByteLength() const;

    /**
     * Calculates the length of the unfragmentable part of IPv6 header
     * plus the extension headers.
     */
    virtual int calculateUnfragmentableHeaderByteLength() const;

    /**
     * Calculates the length of the payload and extension headers
     * after the Fragment Header.
     */
    virtual int calculateFragmentLength() const;

    /**
     * Removes and returns the first extension header of this datagram
     */
    virtual IPv6ExtensionHeader *removeFirstExtensionHeader();

    /**
     * Removes and returns the first extension header with the given type.
     */
    virtual IPv6ExtensionHeader *removeExtensionHeader(IPProtocolId extensionType);

    virtual L3Address getSourceAddress() const { return L3Address(getSrcAddress()); }
    virtual void setSourceAddress(const L3Address& address) { setSrcAddress(address.toIPv6()); }
    virtual L3Address getDestinationAddress() const { return L3Address(getDestAddress()); }
    virtual void setDestinationAddress(const L3Address& address) { setDestAddress(address.toIPv6()); }
    virtual int getTransportProtocol() const { return IPv6Datagram_Base::getTransportProtocol(); }
    virtual void setTransportProtocol(int protocol) { IPv6Datagram_Base::setTransportProtocol(protocol); }
};

std::ostream& operator<<(std::ostream& out, const IPv6ExtensionHeader&);

} // namespace inet

#endif // ifndef __INET_IPV6DATAGRAM_H

