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

#ifndef __INET_LEGACYDUPLICATEDETECTOR_H
#define __INET_LEGACYDUPLICATEDETECTOR_H

#include <map>

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/IDuplicateRemoval.h"

namespace inet {
namespace ieee80211 {

class INET_API LegacyDuplicateRemoval : public IDuplicateRemoval
{
    protected:
        std::map<MacAddress, SequenceControlField> lastSeenSeqNumCache; // cache of last seen sequence numbers per TA

    public:
        virtual bool isDuplicate(const Ptr<const Ieee80211DataOrMgmtHeader>& header) override;
};

} // namespace ieee80211
} // namespace inet

#endif

