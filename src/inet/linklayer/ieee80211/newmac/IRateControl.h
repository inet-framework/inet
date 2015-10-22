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

#ifndef __INET_IRATECONTROL_H
#define __INET_IRATECONTROL_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

using namespace physicallayer;

/**
 * Abstract interface for auto rate control algorithms. Examples of rate
 * control algorithms are ARF, AARF, Onoe and Minstrel.
 */
class INET_API IRateControl
{
    public:
        virtual void initialize(const Ieee80211ModeSet *modeSet, const IIeee80211Mode *initialMode) = 0;
        virtual const IIeee80211Mode *getRate() = 0;
        virtual void frameTransmitted(const Ieee80211Frame *frame, const IIeee80211Mode *mode, int retryCount, bool isSuccessful, bool isGivenUp) = 0;
        virtual void frameReceived(const Ieee80211Frame *frame, const Ieee80211ReceptionIndication *receptionIndication) = 0;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_IRATECONTROL_H
