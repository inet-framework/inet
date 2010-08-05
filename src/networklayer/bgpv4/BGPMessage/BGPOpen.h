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

#ifndef __INET_BGPOPEN_H
#define __INET_BGPOPEN_H

#include "BGPOpen_m.h"

class BGPOpenMessage : public BGPOpenMessage_Base
{
public:
    BGPOpenMessage(const char *name="BGPOpenMessage", int kind=0) : BGPOpenMessage_Base(name,kind)
    {
        setType(BGP_OPEN);
        setBitLength(BGP_HEADER_OCTETS + BGP_OPEN_OCTETS);
    }

    BGPOpenMessage(const BGPOpenMessage& other) : BGPOpenMessage_Base(other.getName()) {operator=(other);}
    BGPOpenMessage& operator=(const BGPOpenMessage& other) {BGPOpenMessage_Base::operator=(other); return *this;}
    virtual BGPOpenMessage_Base *dup() const {return new BGPOpenMessage(*this);}

    void setBitLength(unsigned short length_var)
    {
        BGPHeader::setLength(length_var);
        setByteLength(length_var);
    }

    static const int BGP_OPEN_OCTETS = 10;
};

#endif

