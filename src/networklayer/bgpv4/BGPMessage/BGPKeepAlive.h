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

#ifndef __INET_BGPKEEPALIVE_H
#define __INET_BGPKEEPALIVE_H

#include "BGPKeepAlive_m.h"

class BGPKeepAliveMessage : public BGPKeepAliveMessage_Base
{
public:
    BGPKeepAliveMessage(const char *name="BGPKeepAlive", int kind=0) : BGPKeepAliveMessage_Base(name,kind)
    {
        setType(BGP_KEEPALIVE);
        setBitLength(BGP_HEADER_OCTETS);
    }

    BGPKeepAliveMessage(const BGPKeepAliveMessage& other) : BGPKeepAliveMessage_Base(other.getName()) {operator=(other);}
    BGPKeepAliveMessage& operator=(const BGPKeepAliveMessage& other) {BGPKeepAliveMessage_Base::operator=(other); return *this;}
    virtual BGPKeepAliveMessage_Base *dup() const {return new BGPKeepAliveMessage(*this);}

    void setBitLength(unsigned short length_var)
    {
        BGPHeader::setLength(length_var);
        setByteLength(length_var);
    }
};

#endif

