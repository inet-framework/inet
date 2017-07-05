//
// Copyright (C) 2016 OpenSim Ltd.
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

#ifndef __INET_MCF_H
#define __INET_MCF_H

#include "inet/linklayer/ieee80211/mac/contract/IChannelAccess.h"
#include "inet/linklayer/ieee80211/mac/contract/ICoordinationFunction.h"
#include "inet/linklayer/ieee80211/mac/framesequence/McfFs.h"

namespace inet {
namespace ieee80211 {

/**
 * Implements IEEE 802.11 Mesh Coordination Function.
 */
class INET_API Mcf : public ICoordinationFunction, public cSimpleModule
{
    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    public:
        virtual void processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header) override { throw cRuntimeError("Unimplemented!"); }
        virtual void processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header) override { throw cRuntimeError("Unimplemented!"); };
        virtual void corruptedFrameReceived() override { throw cRuntimeError("Unimplemented!"); }
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_MCF_H

