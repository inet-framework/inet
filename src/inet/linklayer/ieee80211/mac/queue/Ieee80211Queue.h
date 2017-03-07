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

#ifndef __INET_IEEE80211QUEUE_H
#define __INET_IEEE80211QUEUE_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211Queue : public cQueue
{
    protected:
        int maxQueueSize = -1; // -1 means unlimited queue

    public:
        virtual ~Ieee80211Queue() { }
        Ieee80211Queue(int maxQueueSize, const char *name);

        virtual bool insert(Ieee80211DataOrMgmtFrame *frame);
        virtual bool insertBefore(Ieee80211DataOrMgmtFrame *where, Ieee80211DataOrMgmtFrame *frame);
        virtual bool insertAfter(Ieee80211DataOrMgmtFrame *where, Ieee80211DataOrMgmtFrame *frame);

        virtual Ieee80211DataOrMgmtFrame *remove(Ieee80211DataOrMgmtFrame *frame);
        virtual Ieee80211DataOrMgmtFrame *pop();

        virtual Ieee80211DataOrMgmtFrame *front() const;
        virtual Ieee80211DataOrMgmtFrame *back() const;

        virtual bool contains(Ieee80211DataOrMgmtFrame *frame) const;

        int getNumberOfFrames() { return getLength(); }
        int getMaxQueueSize() { return maxQueueSize; }

};

class PendingQueue : public Ieee80211Queue {
    public:
        enum class Priority {
            PRIORITIZE_MGMT_OVER_DATA,
            PRIORITIZE_MULTICAST_OVER_DATA
        };

    public:
        virtual ~PendingQueue() { }
        PendingQueue(int maxQueueSize, const char *name);
        PendingQueue(int maxQueueSize, const char *name, Priority priority);

    public:
        static int cmpMgmtOverData(Ieee80211DataOrMgmtFrame *a, Ieee80211DataOrMgmtFrame *b);
        static int cmpMgmtOverMulticastOverUnicast(Ieee80211DataOrMgmtFrame *a, Ieee80211DataOrMgmtFrame *b);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211QUEUE_H
