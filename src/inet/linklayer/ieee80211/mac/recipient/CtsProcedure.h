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

#ifndef __INET_CTSPROCEDURE_H
#define __INET_CTSPROCEDURE_H

#include "inet/linklayer/ieee80211/mac/contract/ICtsProcedure.h"

namespace inet {
namespace ieee80211 {

/*
 * This class implements 9.3.2.6 CTS procedure
 */
class INET_API CtsProcedure : public ICtsProcedure
{
    protected:
        int numReceivedRts = 0;
        int numSentCts = 0;

    protected:
        virtual const Ptr<Ieee80211CtsFrame> buildCts(const Ptr<const Ieee80211RtsFrame>& rtsFrame) const;

    public:
        virtual void processReceivedRts(Packet *rtsPacket, const Ptr<const Ieee80211RtsFrame>& rtsFrame, ICtsPolicy *ctsPolicy, IProcedureCallback *callback) override;
        virtual void processTransmittedCts(const Ptr<const Ieee80211CtsFrame>& ctsFrame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_CTSPROCEDURE_H
