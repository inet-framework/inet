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

#ifndef __INET_LEGACYSEQUENCENUMBERASSIGMENT_H
#define __INET_LEGACYSEQUENCENUMBERASSIGMENT_H

#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/ISequenceNumberAssignment.h"

namespace inet {
namespace ieee80211 {

class INET_API LegacySequenceNumberAssigment : public ISequenceNumberAssignment
{
    protected:
        SequenceNumber lastSeqNum = 0;

    public:
        virtual void assignSequenceNumber(const Ptr<Ieee80211DataOrMgmtHeader>& header) override;

};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_LEGACYSEQUENCENUMBERASSIGMENT_H
