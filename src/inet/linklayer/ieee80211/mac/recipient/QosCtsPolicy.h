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

#ifndef __INET_QOSCTSPOLICY_H
#define __INET_QOSCTSPOLICY_H

#include "inet/linklayer/ieee80211/mac/contract/ICtsPolicy.h"
#include "inet/linklayer/ieee80211/mac/contract/IQosRateSelection.h"
#include "inet/linklayer/ieee80211/mac/contract/IRx.h"

namespace inet {
namespace ieee80211 {

class INET_API QosCtsPolicy : public ModeSetListener, public ICtsPolicy
{
    protected:
        IRx *rx = nullptr;
        IQosRateSelection *rateSelection = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual simtime_t computeCtsDuration(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const;

    public:
        virtual simtime_t computeCtsDurationField(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame) const override;
        virtual bool isCtsNeeded(const Ptr<const Ieee80211RtsFrame>& rtsFrame) const override;

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_QOSCTSPOLICY_H
