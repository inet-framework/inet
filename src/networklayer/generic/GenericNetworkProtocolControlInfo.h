//
// Copyright (C) 2012 Opensim Ltd.
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

#ifndef __GenericNetworkProtocolControlInfo_H
#define __GenericNetworkProtocolControlInfo_H

#include "INETDefs.h"

#include "Address.h"
#include "GenericNetworkProtocolControlInfo_m.h"
#include "INetworkProtocolControlInfo.h"

class INET_API GenericNetworkProtocolControlInfo : public GenericNetworkProtocolControlInfo_Base, public INetworkProtocolControlInfo
{
  private:
    void copy(const GenericNetworkProtocolControlInfo& other) { }

  public:
    GenericNetworkProtocolControlInfo() : GenericNetworkProtocolControlInfo_Base() {}
    GenericNetworkProtocolControlInfo(const GenericNetworkProtocolControlInfo& other) : GenericNetworkProtocolControlInfo_Base(other) {copy(other);}
    GenericNetworkProtocolControlInfo& operator=(const GenericNetworkProtocolControlInfo& other) {if (this==&other) return *this; GenericNetworkProtocolControlInfo_Base::operator=(other); copy(other); return *this;}
    virtual GenericNetworkProtocolControlInfo *dup() const {return new GenericNetworkProtocolControlInfo(*this);}

    virtual short getProtocol() const { return GenericNetworkProtocolControlInfo_Base::getProtocol(); }
    virtual void setProtocol(short protocol) { GenericNetworkProtocolControlInfo_Base::setProtocol(protocol); }
    virtual Address getSourceAddress() const { return GenericNetworkProtocolControlInfo_Base::_getSourceAddress(); }
    virtual void setSourceAddress(const Address & address) { GenericNetworkProtocolControlInfo_Base::setSourceAddress(address); }
    virtual Address getDestinationAddress() const { return GenericNetworkProtocolControlInfo_Base::_getDestinationAddress(); }
    virtual void setDestinationAddress(const Address & address) { GenericNetworkProtocolControlInfo_Base::setDestinationAddress(address); }
    virtual int getInterfaceId() const { return GenericNetworkProtocolControlInfo_Base::getInterfaceId(); }
    virtual void setInterfaceId(int interfaceId) { GenericNetworkProtocolControlInfo_Base::setInterfaceId(interfaceId); }
    virtual short getHopLimit() const { return GenericNetworkProtocolControlInfo_Base::getHopLimit(); }
    virtual void setHopLimit(short hopLimit) { GenericNetworkProtocolControlInfo_Base::setHopLimit(hopLimit); }
};

#endif

