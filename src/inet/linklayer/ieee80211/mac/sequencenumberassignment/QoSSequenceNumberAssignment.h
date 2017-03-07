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

#ifndef __INET_QOSSEQUENCENUMBERASSIGNMENT_H
#define __INET_QOSSEQUENCENUMBERASSIGNMENT_H

#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/ieee80211/mac/common/SequenceControlField.h"
#include "inet/linklayer/ieee80211/mac/contract/ISequenceNumberAssignment.h"
#include <map>

namespace inet {
namespace ieee80211 {

class INET_API QoSSequenceNumberAssignment : public ISequenceNumberAssignment
{
    protected:
        enum CacheType
        {
            SHARED,
            TIME_PRIORITY,
            DATA
        };
        typedef std::pair<MACAddress, Tid> Key;

        std::map<Key, SequenceNumber> lastSentSeqNums; // last sent sequence numbers per RA
        std::map<MACAddress, SequenceNumber> lastSentTimePrioritySeqNums; // last sent sequence numbers per RA
        std::map<MACAddress, SequenceNumber> lastSentSharedSeqNums; // last sent sequence numbers per RA
        SequenceNumber lastSentSharedCounterSeqNum = 0;

    protected:
        virtual CacheType getCacheType(Ieee80211DataOrMgmtFrame *frame, bool incoming);

    public:
        virtual void assignSequenceNumber(Ieee80211DataOrMgmtFrame *frame) override;
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_QOSSEQUENCENUMBERASSIGNMENT_H
