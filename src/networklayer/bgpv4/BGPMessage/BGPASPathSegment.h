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

#ifndef __INET_BGPASPATHSEGMENT_H
#define __INET_BGPASPATHSEGMENT_H

#include "BGPASPathSegment_m.h"

class BGPASPathSegment : public BGPASPathSegment_Base
{
  public:
    BGPASPathSegment() : BGPASPathSegment_Base() {}
    BGPASPathSegment(const BGPASPathSegment& other) : BGPASPathSegment_Base() {operator=(other);}
    BGPASPathSegment& operator=(const BGPASPathSegment& other) {BGPASPathSegment_Base::operator=(other); return *this;}
    virtual BGPASPathSegment_Base *dup() const {return new BGPASPathSegment(*this);}
};

#endif

