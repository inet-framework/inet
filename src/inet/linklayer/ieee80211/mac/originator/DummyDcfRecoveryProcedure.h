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

#ifndef __INET_DUMMYDCFRECOVERYPROCEDURE_H
#define __INET_DUMMYDCFRECOVERYPROCEDURE_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"
#include <map>

namespace inet {
namespace ieee80211 {

class INET_API DummyDcfRecoveryProcedure
{
    protected:
        std::map<int, int> retries;
        InProgressFrames * inProgressFrames;

    public:
        DummyDcfRecoveryProcedure(InProgressFrames * inProgressFrames) :
            inProgressFrames(inProgressFrames)
        { }
        virtual ~DummyDcfRecoveryProcedure() { }

        virtual void increaseRetryCount(Ieee80211Frame *frame);
        virtual int computeCw(int cwMin, int cwMax);

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_DUMMYDCFRECOVERYPROCEDURE_H
