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

#ifndef __INET_HCCA_H
#define __INET_HCCA_H

#include "inet/linklayer/ieee80211/mac/contract/IChannelAccess.h"

namespace inet {
namespace ieee80211 {

/**
 * Implements IEEE 802.11 Hybrid coordination function (HCF) Controlled Channel Access.
 */
class INET_API Hcca : public IChannelAccess, public cSimpleModule
{
    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;

    public:
        virtual bool isOwning();

        virtual void requestChannel(IChannelAccess::ICallback *callback) override;
        virtual void releaseChannel(IChannelAccess::ICallback *callback) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_HCCA_H

