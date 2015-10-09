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

#ifndef __INET_MACUTILS_H
#define __INET_MACUTILS_H

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/linklayer/common/MACAddress.h"

using namespace inet::physicallayer;

namespace inet {
namespace ieee80211 {

class Ieee80211Frame;
class Ieee80211RTSFrame;
class Ieee80211CTSFrame;
class Ieee80211ACKFrame;
class Ieee80211DataOrMgmtFrame;
class IMacParameters;

class INET_API MacUtils
{
    private:
        IMacParameters *params;
    public:
        MacUtils(IMacParameters *params);

        virtual simtime_t getAckDuration() const;
        virtual simtime_t getCtsDuration() const;
        virtual simtime_t getAckTimeout() const;
        virtual simtime_t getCtsTimeout() const;

        virtual Ieee80211RTSFrame *buildRtsFrame(Ieee80211DataOrMgmtFrame *dataFrame, const IIeee80211Mode *dataFrameMode) const;
        virtual Ieee80211CTSFrame *buildCtsFrame(Ieee80211RTSFrame *rtsFrame) const;
        virtual Ieee80211ACKFrame *buildAckFrame(Ieee80211DataOrMgmtFrame *dataFrame) const;

        virtual Ieee80211Frame *setFrameMode(Ieee80211Frame *frame, const IIeee80211Mode *mode) const;

        virtual bool isForUs(Ieee80211Frame *frame) const;
        virtual bool isMulticast(Ieee80211Frame *frame) const;
        virtual bool isBroadcast(Ieee80211Frame *frame) const;
        virtual bool isCts(Ieee80211Frame *frame) const;
        virtual bool isAck(Ieee80211Frame *frame) const;
};

} // namespace ieee80211
} // namespace inet

#endif
