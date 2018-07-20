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

#ifndef __INET_RTSPROCEDURE_H
#define __INET_RTSPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/IRtsProcedure.h"

namespace inet {
namespace ieee80211 {

class INET_API RtsProcedure : public IRtsProcedure
{
    protected:
        int numSentRts = 0;

    public:
        virtual const Ptr<Ieee80211RtsFrame> buildRtsFrame(const Ptr<const Ieee80211DataOrMgmtHeader>& dataOrMgmtHeader) const override;
        virtual void processTransmittedRts(const Ptr<const Ieee80211RtsFrame>& rtsFrame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_RTSPROCEDURE_H
