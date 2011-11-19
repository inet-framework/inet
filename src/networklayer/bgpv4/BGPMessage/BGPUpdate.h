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

class BGPUpdateMessage : public BGPUpdateMessage_Base
{
protected:
    unsigned short computePathAttributesBytes(const BGPUpdatePathAttributeList& pathAttrs);
public:
    BGPUpdateMessage(const char *name = NULL, int kind = 0) : BGPUpdateMessage_Base(name, kind) {}
    virtual BGPUpdateMessage *dup() const {return new BGPUpdateMessage(*this);}
    void setWithdrawnRoutesArraySize(unsigned int size);
    void setPathAttributeList(const BGPUpdatePathAttributeList& pathAttributeList_var);
    void setNLRI(const BGPUpdateNLRI& NLRI_var);
};

#endif

