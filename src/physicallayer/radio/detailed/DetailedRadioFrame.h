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


#ifndef __INET_DETAILEDRADIOFRAME_H_
#define __INET_DETAILEDRADIOFRAME_H_

#include "INETDefs.h"
#include "IRadioFrame.h"
#include "DetailedRadioFrame_m.h"

/**
 * Represents an detailed radio frame. More info in the DetailedRadioFrame.msg file
 * (and the documentation generated from it).
 */
class INET_API DetailedRadioFrame : public DetailedRadioFrame_Base, public IRadioFrame
{
  public:
    DetailedRadioFrame(const char *name = NULL, int kind = 0) : DetailedRadioFrame_Base(name, kind) {}
    DetailedRadioFrame(const DetailedRadioFrame& other) : DetailedRadioFrame_Base(other) {}
    DetailedRadioFrame& operator=(const DetailedRadioFrame& other) {DetailedRadioFrame_Base::operator=(other); return *this;}

    virtual DetailedRadioFrame *dup() const {return new DetailedRadioFrame(*this);}

    virtual IRadioSignal * getRadioSignal() { return &signal_var; }
};

#endif
