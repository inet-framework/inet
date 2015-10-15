//
// Copyright (C) 2015 Andras Varga
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
// Author: Andras Varga
//

#ifndef __INET_DUPLICATEDETECTORS_H
#define __INET_DUPLICATEDETECTORS_H

#include <map>
#include "IDuplicateDetector.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {
namespace ieee80211 {

class INET_API NoDuplicateDetector : public IDuplicateDetector
{
    public:
        virtual void assignSequenceNumber(Ieee80211DataOrMgmtFrame *frame) override {}
        virtual bool isDuplicate(Ieee80211DataOrMgmtFrame *frame) override {return false;}
};

class INET_API LegacyDuplicateDetector : public IDuplicateDetector
{
    protected:
        struct SeqVal
        {
            uint16_t seqNum;
            short fragNum;
        };
        typedef std::map<MACAddress, SeqVal> Mac2SeqValMap;
        Mac2SeqValMap lastSeenSeqNumCache; // cache of last seen sequence numbers per TA
        uint16_t lastSeqNum = 0;
    public:
        virtual void assignSequenceNumber(Ieee80211DataOrMgmtFrame *frame) override;
        virtual bool isDuplicate(Ieee80211DataOrMgmtFrame *frame) override;
};

class INET_API NonQoSDuplicateDetector : public LegacyDuplicateDetector
{
    protected:
        std::map<MACAddress, int16_t> lastSentSeqNums; // last sent sequence numbers per RA
    public:
        virtual void assignSequenceNumber(Ieee80211DataOrMgmtFrame *frame) override;
};

class INET_API QoSDuplicateDetector : public IDuplicateDetector
{
    private:
        enum CacheType
        {
            SHARED,
            TIME_PRIORITY,
            DATA
        };

    protected:
        typedef int8_t tid_t;
        typedef std::pair<MACAddress,tid_t> Key;
        struct SeqVal
        {
            uint16_t seqNum;
            short fragNum;
        };
        typedef std::map<Key, SeqVal> Key2SeqValMap;
        typedef std::map<MACAddress, SeqVal> Mac2SeqValMap;
        Key2SeqValMap lastSeenSeqNumCache;// cache of last seen sequence numbers per TA
        Mac2SeqValMap lastSeenSharedSeqNumCache;
        Mac2SeqValMap lastSeenTimePriorityManagementSeqNumCache;

        std::map<Key, int16_t> lastSentSeqNums; // last sent sequence numbers per RA
        std::map<MACAddress, int16_t> lastSentTimePrioritySeqNums; // last sent sequence numbers per RA

        std::map<MACAddress, int16_t> lastSentSharedSeqNums; // last sent sequence numbers per RA
        int16_t lastSentSharedCounterSeqNum;
    protected:
        CacheType getCacheType(Ieee80211DataOrMgmtFrame *frame, bool incoming);
    public:
        virtual void assignSequenceNumber(Ieee80211DataOrMgmtFrame *frame) override;
        virtual bool isDuplicate(Ieee80211DataOrMgmtFrame *frame) override;
};

} // namespace ieee80211
} // namespace inet

#endif
