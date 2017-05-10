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

        virtual simtime_t computeRtsDurationField(Packet *rtsPacket, const Ptr<Ieee80211RtsFrame>& rtsFrame, Packet *pendingPacket, const Ptr<Ieee80211DataOrMgmtHeader>& pendingFrame);
        virtual simtime_t computeDataFrameDurationField(Packet *dataPacket, const Ptr<Ieee80211DataHeader>& dataFrame, Packet *pendingPacket, const Ptr<Ieee80211DataOrMgmtHeader>& pendingFrame);
        virtual simtime_t computeMgmtFrameDurationField(Packet *mgmtPacket, const Ptr<Ieee80211MgmtHeader>& mgmtFrame, Packet *pendingPacket, const Ptr<Ieee80211DataOrMgmtHeader>& pendingFrame);

    public:
        virtual ~OriginatorProtectionMechanism() { }

        virtual simtime_t computeDurationField(Packet *packet, const Ptr<Ieee80211MacHeader>& frame, Packet *pendingPacket, const Ptr<Ieee80211DataOrMgmtHeader>& pendingFrame);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_ORIGINATORPROTECTIONMECHANISM_H
