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

#ifndef __INET_SIMPLENETWORKPROTOCOLCONTROLINFO_H
#define __INET_SIMPLENETWORKPROTOCOLCONTROLINFO_H

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/SimpleNetworkProtocolControlInfo_m.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"

namespace inet {

class INET_API SimpleNetworkProtocolControlInfo : public SimpleNetworkProtocolControlInfo_Base, public INetworkProtocolControlInfo
{
  private:
    void copy(const SimpleNetworkProtocolControlInfo& other) {}

  public:
    SimpleNetworkProtocolControlInfo() : SimpleNetworkProtocolControlInfo_Base() {}
    SimpleNetworkProtocolControlInfo(const SimpleNetworkProtocolControlInfo& other) : SimpleNetworkProtocolControlInfo_Base(other) { copy(other); }
    SimpleNetworkProtocolControlInfo& operator=(const SimpleNetworkProtocolControlInfo& other) { if (this == &other) return *this; SimpleNetworkProtocolControlInfo_Base::operator=(other); copy(other); return *this; }
    virtual SimpleNetworkProtocolControlInfo *dup() const { return new SimpleNetworkProtocolControlInfo(*this); }

    virtual short getTransportProtocol() const { return SimpleNetworkProtocolControlInfo_Base::getProtocol(); }
    virtual void setTransportProtocol(short protocol) { SimpleNetworkProtocolControlInfo_Base::setProtocol(protocol); }
    virtual L3Address getSourceAddress() const { return SimpleNetworkProtocolControlInfo_Base::_getSourceAddress(); }
    virtual void setSourceAddress(const L3Address& address) { SimpleNetworkProtocolControlInfo_Base::setSourceAddress(address); }
    virtual L3Address getDestinationAddress() const { return SimpleNetworkProtocolControlInfo_Base::_getDestinationAddress(); }
    virtual void setDestinationAddress(const L3Address& address) { SimpleNetworkProtocolControlInfo_Base::setDestinationAddress(address); }
    virtual int getInterfaceId() const { return SimpleNetworkProtocolControlInfo_Base::getInterfaceId(); }
    virtual void setInterfaceId(int interfaceId) { SimpleNetworkProtocolControlInfo_Base::setInterfaceId(interfaceId); }
    virtual short getHopLimit() const { return SimpleNetworkProtocolControlInfo_Base::getHopLimit(); }
    virtual void setHopLimit(short hopLimit) { SimpleNetworkProtocolControlInfo_Base::setHopLimit(hopLimit); }
};

} // namespace inet

#endif // ifndef __INET_SIMPLENETWORKPROTOCOLCONTROLINFO_H

