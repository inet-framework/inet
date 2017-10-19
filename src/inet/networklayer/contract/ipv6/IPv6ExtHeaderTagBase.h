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
class INET_API IPv6ExtHeaderTagBase : public IPv6ExtHeaderTagBase_Base
{
  protected:
    typedef std::vector<Ipv6ExtensionHeader *> ExtensionHeaders;
    ExtensionHeaders extensionHeaders;

  private:
    void copy(const IPv6ExtHeaderTagBase& other);
    void clean();

  public:
    IPv6ExtHeaderTagBase() : IPv6ExtHeaderTagBase_Base() { }
    virtual ~IPv6ExtHeaderTagBase();
    IPv6ExtHeaderTagBase(const IPv6ExtHeaderTagBase& other) : IPv6ExtHeaderTagBase_Base(other) { copy(other); }
    IPv6ExtHeaderTagBase& operator=(const IPv6ExtHeaderTagBase& other);
    virtual IPv6ExtHeaderTagBase *dup() const override { return new IPv6ExtHeaderTagBase(*this); }

    /**
     * Returns the number of extension headers in this datagram
     */
    virtual unsigned int getExtensionHeaderArraySize() const override;

    /** Generated but unused method, should not be called. */
    virtual void setExtensionHeaderArraySize(unsigned int size) override;

    /**
     * Returns the kth extension header in this datagram
     */
    virtual IPv6ExtensionHeaderPtr& getMutableExtensionHeader(unsigned int k) override;
    virtual const IPv6ExtensionHeaderPtr& getExtensionHeader(unsigned int k) const override;

    /** Generated but unused method, should not be called. */
    virtual void setExtensionHeader(unsigned int k, const IPv6ExtensionHeaderPtr& extensionHeader_var) override;

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

