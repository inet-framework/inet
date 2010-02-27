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


#ifndef _IPv6DATAGRAM_H_
#define _IPv6DATAGRAM_H_

#include <list>
#include "INETDefs.h"
#include "IPv6Datagram_m.h"

/**
 * Represents an IPv6 datagram. More info in the IPv6Datagram.msg file
 * (and the documentation generated from it).
 */
class INET_API IPv6Datagram : public IPv6Datagram_Base
{
  protected:
    typedef std::vector<IPv6ExtensionHeader*> ExtensionHeaders;
    ExtensionHeaders extensionHeaders;

  public:
    IPv6Datagram(const char *name=NULL, int kind=0) : IPv6Datagram_Base(name,kind) {}
    IPv6Datagram(const IPv6Datagram& other) : IPv6Datagram_Base(other.getName()) {operator=(other);}
    IPv6Datagram& operator=(const IPv6Datagram& other);
    virtual IPv6Datagram *dup() const {return new IPv6Datagram(*this);}

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
     * Adds an extension header to the datagram, at the given position.
     * The default (atPos==-1) is to add the header at the end.
     */
    virtual void addExtensionHeader(IPv6ExtensionHeader *eh, int atPos=-1);

    /**
     * Calculates the length of the IPv6 header plus the extension
     * headers.
     */
    virtual int calculateHeaderByteLength() const;
};

/**
 * Represents an IPv6 extension header. More info in the IPv6Datagram.msg file
 * (and the documentation generated from it).
 */
class INET_API IPv6ExtensionHeader : public IPv6ExtensionHeader_Base
{
  public:
    IPv6ExtensionHeader() : IPv6ExtensionHeader_Base() {}
    IPv6ExtensionHeader(const IPv6ExtensionHeader& other) : IPv6ExtensionHeader_Base() {operator=(other);}
    IPv6ExtensionHeader& operator=(const IPv6ExtensionHeader& other) {IPv6ExtensionHeader_Base::operator=(other); return *this;}
    virtual IPv6ExtensionHeader *dup() const {throw cRuntimeError(this, "dup() should be redefined");}
    virtual IPProtocolId getExtensionType() const;
    virtual int getByteLength() const;
};

#endif


