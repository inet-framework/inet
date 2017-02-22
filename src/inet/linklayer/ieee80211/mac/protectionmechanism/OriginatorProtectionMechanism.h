//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_ORIGINATORPROTECTIONMECHANISM_H
#define __INET_ORIGINATORPROTECTIONMECHANISM_H

#include "inet/linklayer/ieee80211/mac/common/ModeSetListener.h"
#include "inet/linklayer/ieee80211/mac/contract/IRateSelection.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API OriginatorProtectionMechanism : public ModeSetListener
{
    protected:
        IRateSelection *rateSelection = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

        virtual simtime_t computeRtsDurationField(Ieee80211RTSFrame *rtsFrame, Ieee80211DataOrMgmtFrame *pendingFrame);
        virtual simtime_t computeDataFrameDurationField(Ieee80211DataFrame *dataFrame, Ieee80211DataOrMgmtFrame *pendingFrame);
        virtual simtime_t computeMgmtFrameDurationField(Ieee80211ManagementFrame *mgmtFrame, Ieee80211DataOrMgmtFrame *pendingFrame);

    public:
        virtual ~OriginatorProtectionMechanism() { }

        virtual simtime_t computeDurationField(Ieee80211Frame *frame, Ieee80211DataOrMgmtFrame *pendingFrame);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_ORIGINATORPROTECTIONMECHANISM_H
