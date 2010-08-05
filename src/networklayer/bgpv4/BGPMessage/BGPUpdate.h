//
// Copyright (C) 2010 Helene Lageber
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

#ifndef __INET_BGPUPDATE_H
#define __INET_BGPUPDATE_H

#include "BGPUpdate_m.h"

class BGPUpdate : public BGPUpdate_Base
{
public:
    BGPUpdate(const char *name="BGPUpdate", int kind=0) : BGPUpdate_Base(name,kind)
    {
        setType(BGP_UPDATE);
        setBitLength(BGP_HEADER_OCTETS + BGP_EMPTY_UPDATE_OCTETS);
    }

    BGPUpdate(const BGPUpdate& other) : BGPUpdate_Base(other.getName()) {operator=(other);}
    BGPUpdate& operator=(const BGPUpdate& other) {BGPUpdate_Base::operator=(other); return *this;}
    virtual BGPUpdate_Base *dup() const {return new BGPUpdate(*this);}

    void setBitLength(unsigned short length_var)
    {
        BGPHeader::setLength(length_var);
        setByteLength(length_var);
    }

    void setUnfeasibleRoutesLength(unsigned short unfeasibleRoutesLength_var);
    void setWithdrawnRoutesArraySize(unsigned int size);

    void setTotalPathAttributeLength(unsigned short totalPathAttributeLength_var);
    void setPathAttributesContent(const BGPUpdatePathAttributesContent& pathAttributesContent_var);

    void setNLRI(const BGPUpdateNLRI& NLRI_var);

    static const int BGP_EMPTY_UPDATE_OCTETS = 4; // UnfeasibleRoutesLength (2) + TotalPathAttributeLength (2)

};

#endif

