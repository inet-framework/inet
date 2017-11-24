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

#ifndef __INET_IPV6EXTHEADERTAGBASE_H
#define __INET_IPV6EXTHEADERTAGBASE_H

#include "inet/networklayer/contract/ipv6/IPv6ExtHeaderTagBase_m.h"

namespace inet {

class Ipv6Header;
class Ipv6ExtensionHeader;

/**
 *
 * See the IPv6ExtHeaderTagBase.msg file for more info.
 */
class INET_API Ipv6ExtHeaderTagBase : public Ipv6ExtHeaderTagBase_Base
{
  protected:
    typedef std::vector<Ipv6ExtensionHeader *> ExtensionHeaders;
    ExtensionHeaders extensionHeaders;

  private:
    void copy(const Ipv6ExtHeaderTagBase& other);
    void clean();

  public:
    Ipv6ExtHeaderTagBase() : Ipv6ExtHeaderTagBase_Base() { }
    virtual ~Ipv6ExtHeaderTagBase();
    Ipv6ExtHeaderTagBase(const Ipv6ExtHeaderTagBase& other) : Ipv6ExtHeaderTagBase_Base(other) { copy(other); }
    Ipv6ExtHeaderTagBase& operator=(const Ipv6ExtHeaderTagBase& other);
    virtual Ipv6ExtHeaderTagBase *dup() const override { return new Ipv6ExtHeaderTagBase(*this); }

    /**
     * Returns the number of extension headers in this datagram
     */
    virtual unsigned int getExtensionHeaderArraySize() const override;

    /** Generated but unused method, should not be called. */
    virtual void setExtensionHeaderArraySize(unsigned int size) override;

    /**
     * Returns the kth extension header in this datagram
     */
    virtual Ipv6ExtensionHeader *getMutableExtensionHeader(unsigned int k) override;
    virtual const Ipv6ExtensionHeader *getExtensionHeader(unsigned int k) const override;

    /** Generated but unused method, should not be called. */
    virtual void setExtensionHeader(unsigned int k, Ipv6ExtensionHeader *extensionHeader_var) override;

    /**
     * Adds an extension header to the datagram, at the given position.
     * The default (atPos==-1) is to add the header at the end.
     */
    virtual void addExtensionHeader(Ipv6ExtensionHeader *eh, int atPos = -1);

    /**
     * Remove the first extension header and return it.
     */
    Ipv6ExtensionHeader *removeFirstExtensionHeader();

};

} // namespace inet

#endif // ifndef __INET_IPV6EXTHEADERTAGBASE_H

