//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_IDEALRADIOFRAME_H_
#define __INET_IDEALRADIOFRAME_H_

#include "IRadioFrame.h"
#include "IdealRadioFrame_m.h"

/**
 * Represents an ideal radio frame. More info in the IdealRadioFrame.msg file
 * (and the documentation generated from it).
 */
class INET_API IdealRadioFrame : public IdealRadioFrame_Base, public IRadioFrame
{
  public:
    IdealRadioFrame(const char *name = NULL, int kind = 0) : IdealRadioFrame_Base(name, kind) {}
    IdealRadioFrame(const IdealRadioFrame& other) : IdealRadioFrame_Base(other) {}
    IdealRadioFrame& operator=(const IdealRadioFrame& other) {IdealRadioFrame_Base::operator=(other); return *this;}

    virtual IdealRadioFrame *dup() const {return new IdealRadioFrame(*this);}

    virtual IRadioSignal * getRadioSignal() { return NULL; }
};

#endif
