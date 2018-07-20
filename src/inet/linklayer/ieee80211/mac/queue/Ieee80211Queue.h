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

#include "inet/common/packet/Packet.h"

namespace inet {
namespace ieee80211 {

class INET_API Ieee80211Queue : public cQueue
{
    protected:
        int maxQueueSize = -1; // -1 means unlimited queue

    public:
        virtual ~Ieee80211Queue() { }
        Ieee80211Queue(int maxQueueSize, const char *name);

        virtual bool insert(Packet *frame);
        virtual bool insertBefore(Packet *where, Packet *frame);
        virtual bool insertAfter(Packet *where, Packet *frame);

        virtual Packet *remove(Packet *frame);
        virtual Packet *pop() override;

        virtual Packet *front() const override;
        virtual Packet *back() const override;

        virtual bool contains(Packet *frame) const;

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
        static int cmpMgmtOverData(Packet *a, Packet *b);
        static int cmpMgmtOverMulticastOverUnicast(Packet *a, Packet *b);
};

} /* namespace ieee80211 */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211QUEUE_H
