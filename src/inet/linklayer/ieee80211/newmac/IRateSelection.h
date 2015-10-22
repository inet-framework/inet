//
// Copyright (C) 2015 Andras Varga
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

#ifndef __INET_IRATESELECTION_H
#define __INET_IRATESELECTION_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"

using namespace inet::physicallayer;

namespace inet {
namespace ieee80211 {

class Ieee80211Frame;
class Ieee80211DataOrMgmtFrame;
class IRateControl;

/**
 * Abstract interface for rate selection. Rate selection decides what bit rate
 * (or MCS) should be used for any particular frame. The rules of rate selection
 * is described in the 802.11 specification in the section titled "Multirate Support".
 */
class INET_API IRateSelection
{
    public:
        virtual void setRateControl(IRateControl *rateControl) = 0;
        virtual const IIeee80211Mode *getSlowestMandatoryMode() = 0;  // for EIFS computation, etc.
        virtual const IIeee80211Mode *getModeForUnicastDataOrMgmtFrame(Ieee80211DataOrMgmtFrame *frame) = 0;
        virtual const IIeee80211Mode *getModeForMulticastDataOrMgmtFrame(Ieee80211DataOrMgmtFrame *frame) = 0;
        virtual const IIeee80211Mode *getModeForControlFrame(Ieee80211Frame *controlFrame) = 0;
        virtual const IIeee80211Mode *getResponseControlFrameMode() = 0;  // for RTS Duration field computation (needs CTS+Data+ACK durations)
        virtual ~IRateSelection() {}
};

} // namespace ieee80211
} // namespace inet

#endif
