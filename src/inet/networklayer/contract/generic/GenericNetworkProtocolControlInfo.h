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

#ifndef __INET_GENERICNETWORKPROTOCOLCONTROLINFO_H
#define __INET_GENERICNETWORKPROTOCOLCONTROLINFO_H

#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/contract/generic/GenericNetworkProtocolControlInfo_m.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

namespace inet {

class INET_API GenericNetworkProtocolControlInfo : public GenericNetworkProtocolControlInfo_Base, public INetworkProtocolControlInfo
{
  private:
    void copy(const GenericNetworkProtocolControlInfo& other) {}

  public:
    GenericNetworkProtocolControlInfo() : GenericNetworkProtocolControlInfo_Base() {}
    GenericNetworkProtocolControlInfo(const GenericNetworkProtocolControlInfo& other) : GenericNetworkProtocolControlInfo_Base(other) { copy(other); }
    GenericNetworkProtocolControlInfo& operator=(const GenericNetworkProtocolControlInfo& other) { if (this == &other) return *this; GenericNetworkProtocolControlInfo_Base::operator=(other); copy(other); return *this; }
    virtual GenericNetworkProtocolControlInfo *dup() const override { return new GenericNetworkProtocolControlInfo(*this); }

    virtual short getHopLimit() const override { return GenericNetworkProtocolControlInfo_Base::getHopLimit(); }
    virtual void setHopLimit(short hopLimit) override { GenericNetworkProtocolControlInfo_Base::setHopLimit(hopLimit); }
};

} // namespace inet

#endif // ifndef __INET_GENERICNETWORKPROTOCOLCONTROLINFO_H

