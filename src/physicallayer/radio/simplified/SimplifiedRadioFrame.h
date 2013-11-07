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


#ifndef __INET_SIMPLIFIEDRADIOFRAME_H_
#define __INET_SIMPLIFIEDRADIOFRAME_H_

#include "INETDefs.h"
#include "IRadioFrame.h"
#include "SimplifiedRadioFrame_m.h"

/**
 * Represents a simplified radio frame. More info in the SimplifiedRadioFrame.msg file
 * (and the documentation generated from it).
 */
class INET_API SimplifiedRadioFrame : public SimplifiedRadioFrame_Base, public IRadioFrame
{
  public:
    SimplifiedRadioFrame(const char *name = NULL, int kind = 0) : SimplifiedRadioFrame_Base(name, kind) {}
    SimplifiedRadioFrame(const SimplifiedRadioFrame& other) : SimplifiedRadioFrame_Base(other) {}
    SimplifiedRadioFrame& operator=(const SimplifiedRadioFrame& other) {SimplifiedRadioFrame_Base::operator=(other); return *this;}

    virtual SimplifiedRadioFrame *dup() const {return new SimplifiedRadioFrame(*this);}

    virtual IRadioSignal * getRadioSignal() { return NULL; }
};

#endif
