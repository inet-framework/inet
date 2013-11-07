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


#ifndef __INET_IEEE802CTRL_H_
#define __INET_IEEE802CTRL_H_

#include "INETDefs.h"
#include "ILinkLayerControlInfo.h"
#include "Ieee802Ctrl_m.h"

/**
 * Represents a IEEE 802 control info. More info in the Ieee802Ctrl.msg file
 * (and the documentation generated from it).
 */
class INET_API Ieee802Ctrl : public Ieee802Ctrl_Base, public ILinkLayerControlInfo
{
  public:
    Ieee802Ctrl() : Ieee802Ctrl_Base() {}
    Ieee802Ctrl(const Ieee802Ctrl& other) : Ieee802Ctrl_Base(other) {}
    Ieee802Ctrl& operator=(const Ieee802Ctrl& other) {Ieee802Ctrl_Base::operator=(other); return *this;}

    virtual Ieee802Ctrl *dup() const {return new Ieee802Ctrl(*this);}

    virtual MACAddress getSourceAddress() const { return getSrc(); }
    virtual void setSourceAddress(const MACAddress & address) { setSrc(address); }
    virtual MACAddress getDestinationAddress() const { return getDest(); }
    virtual void setDestinationAddress(const MACAddress & address) { setDest(address); };
};

#endif
