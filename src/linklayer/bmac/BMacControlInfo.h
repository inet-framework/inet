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


#ifndef _BMACCONTROLINFO_H_
#define _BMACCONTROLINFO_H_

#include "INETDefs.h"
#include "ILinkLayerControlInfo.h"
#include "BMacControlInfo_m.h"

/**
 * Represents a BMac control info. More info in the BMacControlInfo.msg file
 * (and the documentation generated from it).
 */
class INET_API BMacControlInfo : public BMacControlInfo_Base, public ILinkLayerControlInfo
{
  public:
    BMacControlInfo() : BMacControlInfo_Base() {}
    BMacControlInfo(const BMacControlInfo& other) : BMacControlInfo_Base(other) {}
    BMacControlInfo& operator=(const BMacControlInfo& other) {BMacControlInfo_Base::operator=(other); return *this;}

    virtual BMacControlInfo *dup() const {return new BMacControlInfo(*this);}

    virtual MACAddress getSourceAddress() const { return getSrc(); }
    virtual void setSourceAddress(const MACAddress & address) { setSrc(address); }
    virtual MACAddress getDestinationAddress() const { return getDest(); }
    virtual void setDestinationAddress(const MACAddress & address) { setDest(address); };
};

#endif
