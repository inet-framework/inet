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

#ifndef __INET_EDCA_H
#define __INET_EDCA_H

#include "inet/linklayer/ieee80211/mac/channelaccess/Edcaf.h"
#include "inet/linklayer/ieee80211/mac/common/Ieee80211Defs.h"
#include "inet/linklayer/ieee80211/mac/lifetime/EdcaTransmitLifetimeHandler.h"
#include "inet/linklayer/ieee80211/mac/originator/NonQosRecoveryProcedure.h"

namespace inet {
namespace ieee80211 {

/**
 * Implements IEEE 802.11 Enhanced Distributed Channel Access.
 */
class INET_API Edca : public cSimpleModule
{
    protected:
        int numEdcafs = -1;
        Edcaf **edcafs = nullptr;
        EdcaTransmitLifetimeHandler *lifetimeHandler = nullptr;
        NonQosRecoveryProcedure *mgmtAndNonQoSRecoveryProcedure = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

        virtual AccessCategory mapTidToAc(Tid tid);

    public:
        virtual ~Edca();

        virtual AccessCategory classifyFrame(const Ptr<const Ieee80211DataHeader>& header);
        virtual Edcaf *getEdcaf(AccessCategory ac) const { return edcafs[ac]; }
        virtual Edcaf *getChannelOwner();
        virtual std::vector<Edcaf*> getInternallyCollidedEdcafs();
        virtual NonQosRecoveryProcedure *getMgmtAndNonQoSRecoveryProcedure() const { return mgmtAndNonQoSRecoveryProcedure; }

        virtual void requestChannelAccess(AccessCategory ac, IChannelAccess::ICallback *callback);
        virtual void releaseChannelAccess(AccessCategory ac, IChannelAccess::ICallback *callback);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_EDCA_H
