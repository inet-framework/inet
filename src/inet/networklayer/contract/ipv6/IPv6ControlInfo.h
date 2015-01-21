//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IPV6CONTROLINFO_H
#define __INET_IPV6CONTROLINFO_H

#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/networklayer/contract/ipv6/IPv6ControlInfo_m.h"

namespace inet {

class IPv6Datagram;
class IPv6ExtensionHeader;

/**
 * Control information for sending/receiving packets over IPv6.
 *
 * See the IPv6ControlInfo.msg file for more info.
 */
class INET_API IPv6ControlInfo : public IPv6ControlInfo_Base, public INetworkProtocolControlInfo
{
  protected:
    IPv6Datagram *dgram;
    typedef std::vector<IPv6ExtensionHeader *> ExtensionHeaders;
    ExtensionHeaders extensionHeaders;

  private:
    void copy(const IPv6ControlInfo& other);
    void clean();

  public:
    IPv6ControlInfo() : IPv6ControlInfo_Base() { dgram = nullptr; }
    virtual ~IPv6ControlInfo();
    IPv6ControlInfo(const IPv6ControlInfo& other) : IPv6ControlInfo_Base(other) { copy(other); }
    IPv6ControlInfo& operator=(const IPv6ControlInfo& other);
    virtual IPv6ControlInfo *dup() const override { return new IPv6ControlInfo(*this); }

    virtual void setOrigDatagram(IPv6Datagram *d);
    virtual IPv6Datagram *getOrigDatagram() const { return dgram; }
    virtual IPv6Datagram *removeOrigDatagram();

    /**
     * Returns bits 0-5 of the Traffic Class field, a value in the 0..63 range
     */
    virtual int getDiffServCodePoint() const override { return getTrafficClass() & 0x3f; }

    /**
     * Sets bits 0-5 of the Traffic Class field; expects a value in the 0..63 range
     */
    virtual void setDiffServCodePoint(int dscp) override { setTrafficClass((getTrafficClass() & 0xc0) | (dscp & 0x3f)); }

    /**
     * Returns bits 6-7 of the Traffic Class field, a value in the range 0..3
     */
    virtual int getExplicitCongestionNotification() const override { return (getTrafficClass() >> 6) & 0x03; }

    /**
     * Sets bits 6-7 of the Traffic Class field; expects a value in the 0..3 range
     */
    virtual void setExplicitCongestionNotification(int ecn) override { setTrafficClass((getTrafficClass() & 0x3f) | ((ecn & 0x3) << 6)); }

    /**
     * Returns the number of extension headers in this datagram
     */
    virtual unsigned int getExtensionHeaderArraySize() const override;

    /** Generated but unused method, should not be called. */
    virtual void setExtensionHeaderArraySize(unsigned int size) override;

    /**
     * Returns the kth extension header in this datagram
     */
    virtual IPv6ExtensionHeaderPtr& getExtensionHeader(unsigned int k) override;

    /** Generated but unused method, should not be called. */
    virtual void setExtensionHeader(unsigned int k, const IPv6ExtensionHeaderPtr& extensionHeader_var) override;

    /**
     * Adds an extension header to the datagram, at the given position.
     * The default (atPos==-1) is to add the header at the end.
     */
    virtual void addExtensionHeader(IPv6ExtensionHeader *eh, int atPos = -1);

    /**
     * Remove the first extension header and return it.
     */
    IPv6ExtensionHeader *removeFirstExtensionHeader();

    virtual short getTransportProtocol() const override { return IPv6ControlInfo_Base::getProtocol(); }
    virtual void setTransportProtocol(short protocol) override { IPv6ControlInfo_Base::setProtocol(protocol); }
    virtual L3Address getSourceAddress() const override { return L3Address(srcAddr_var); }
    virtual void setSourceAddress(const L3Address& address) override { srcAddr_var = address.toIPv6(); }
    virtual L3Address getDestinationAddress() const override { return L3Address(destAddr_var); }
    virtual void setDestinationAddress(const L3Address& address) override { destAddr_var = address.toIPv6(); }
    virtual int getInterfaceId() const override { return IPv6ControlInfo_Base::getInterfaceId(); }
    virtual void setInterfaceId(int interfaceId) override { IPv6ControlInfo_Base::setInterfaceId(interfaceId); }
    virtual short getHopLimit() const override { return IPv6ControlInfo_Base::getHopLimit(); }
    virtual void setHopLimit(short hopLimit) override { IPv6ControlInfo_Base::setHopLimit(hopLimit); }
};

} // namespace inet

#endif // ifndef __INET_IPV6CONTROLINFO_H

