//
// Copyright (C) 2011 Andras Varga
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

#ifndef __INET_IPV4DATAGRAM_H
#define __INET_IPV4DATAGRAM_H

#include "inet/common/INETDefs.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

namespace inet {

/**
 * Represents an Ipv4 datagram. More info in the Ipv4Header.msg file
 * (and the documentation generated from it).
 */
class INET_API Ipv4Header : public Ipv4Header_Base
{
  private:
    void copy(const Ipv4Header& other);
    void clean();

  public:
    Ipv4Header() : Ipv4Header_Base() {}
    Ipv4Header(const Ipv4Header& other) : Ipv4Header_Base(other) {}
    Ipv4Header& operator=(const Ipv4Header& other) { Ipv4Header_Base::operator=(other); return *this; }

    virtual Ipv4Header *dup() const override { return new Ipv4Header(*this); }

    /**
     * getter/setter for totalLength field in datagram
     * if set to -1, then getter returns getByteLength()
     */
    virtual int getTotalLengthField() const override;

    /**
     * Returns bits 0-5 of the Type of Service field, a value in the 0..63 range
     */
    virtual int getDiffServCodePoint() const override { return getTypeOfService() & 0x3f; }

    /**
     * Sets bits 0-5 of the Type of Service field; expects a value in the 0..63 range
     */
    virtual void setDiffServCodePoint(int dscp) override { setTypeOfService((getTypeOfService() & 0xc0) | (dscp & 0x3f)); }

    /**
     * Returns bits 6-7 of the Type of Service field, a value in the range 0..3
     */
    virtual int getExplicitCongestionNotification() const override { return (getTypeOfService() >> 6) & 0x03; }

    /**
     * Sets bits 6-7 of the Type of Service; expects a value in the 0..3 range
     */
    virtual void setExplicitCongestionNotification(int ecn) override { setTypeOfService((getTypeOfService() & 0x3f) | ((ecn & 0x3) << 6)); }

    /**
     * Returns the number of extension headers in this datagram
     */
    virtual unsigned int getOptionArraySize() const { return options.getTlvOptionArraySize(); }

    /**
     * Returns the kth extension header in this datagram
     */
    virtual TlvOptionBase& getOptionForUpdate(unsigned int k) { return *check_and_cast<TlvOptionBase *>((options.getTlvOptionForUpdate(k))); }
    virtual const TlvOptionBase& getOption(unsigned int k) const { return *check_and_cast<const TlvOptionBase *>((options.getTlvOption(k))); }

    /**
     * Returns the TlvOptionBase of the specified type,
     * or nullptr. If index is 0, then the first, if 1 then the
     * second option is returned.
     */
    virtual TlvOptionBase *findMutableOptionByType(short int optionType, int index = 0);
    virtual const TlvOptionBase *findOptionByType(short int optionType, int index = 0) const;

    /**
     * Adds an TlvOptionBase to the datagram.
     */
    virtual void addOption(TlvOptionBase *opt);
    virtual void addOption(TlvOptionBase *opt, int atPos);

    /**
     * Calculates the length of the Ipv6 header plus the extension
     * headers.
     */
    virtual int calculateHeaderByteLength() const;


    virtual L3Address getSourceAddress() const override { return L3Address(getSrcAddress()); }
    virtual void setSourceAddress(const L3Address& address) override { setSrcAddress(address.toIPv4()); }
    virtual L3Address getDestinationAddress() const override { return L3Address(getDestAddress()); }
    virtual void setDestinationAddress(const L3Address& address) override { setDestAddress(address.toIPv4()); }
    virtual const Protocol *getProtocol() const override { return ProtocolGroup::ipprotocol.findProtocol(getProtocolId()); }
    virtual void setProtocol(const Protocol *protocol) override { setProtocolId((IpProtocolId)ProtocolGroup::ipprotocol.getProtocolNumber(protocol)); }
};

} // namespace inet

#endif // ifndef __INET_IPV4DATAGRAM_H

