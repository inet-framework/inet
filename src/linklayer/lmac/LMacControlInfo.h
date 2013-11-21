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

#ifndef __INET_LMACCONTROLINFO_H_
#define __INET_LMACCONTROLINFO_H_

#include "INETDefs.h"
#include "ILinkLayerControlInfo.h"
#include "LMacControlInfo_m.h"

/**
 * Represents a LMac control info. More info in the LMacControlInfo.msg file
 * (and the documentation generated from it).
 */
class INET_API LMacControlInfo : public LMacControlInfo_Base, public ILinkLayerControlInfo
{
  public:
    LMacControlInfo() : LMacControlInfo_Base() {}
    LMacControlInfo(const LMacControlInfo& other) : LMacControlInfo_Base(other) {}
    LMacControlInfo& operator=(const LMacControlInfo& other) {LMacControlInfo_Base::operator=(other); return *this;}

    virtual LMacControlInfo *dup() const {return new LMacControlInfo(*this);}

    virtual MACAddress getSourceAddress() const { return getSrc(); }
    virtual void setSourceAddress(const MACAddress & address) { setSrc(address); }
    virtual MACAddress getDestinationAddress() const { return getDest(); }
    virtual void setDestinationAddress(const MACAddress & address) { setDest(address); };
    virtual int getInterfaceId() const { return LMacControlInfo_Base::getInterfaceId(); }
    virtual void setInterfaceId(int interfaceId) { LMacControlInfo_Base::setInterfaceId(interfaceId); }
};

#endif
