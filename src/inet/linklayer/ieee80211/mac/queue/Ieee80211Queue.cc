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

#include "inet/linklayer/ieee80211/mac/queue/Ieee80211Queue.h"

namespace inet {
namespace ieee80211 {

Ieee80211Queue::Ieee80211Queue(int maxQueueSize, const char *name) :
    cQueue(name)
{
    this->maxQueueSize = maxQueueSize;
}

bool Ieee80211Queue::insert(Ieee80211DataOrMgmtFrame *frame)
{
    if (maxQueueSize != -1 && getLength() == maxQueueSize)
        return false;
    cQueue::insert(frame);
    return true;
}

bool Ieee80211Queue::insertBefore(Ieee80211DataOrMgmtFrame *where, Ieee80211DataOrMgmtFrame *frame)
{
    if (maxQueueSize != -1 && getLength() == maxQueueSize)
        return false;
    cQueue::insertBefore(where, frame);
    return true;
}

bool Ieee80211Queue::insertAfter(Ieee80211DataOrMgmtFrame *where, Ieee80211DataOrMgmtFrame *frame)
{
    if (maxQueueSize != -1 && getLength() == maxQueueSize)
        return false;
    cQueue::insertAfter(where, frame);
    return true;
}

Ieee80211DataOrMgmtFrame *Ieee80211Queue::remove(Ieee80211DataOrMgmtFrame *frame)
{
    return check_and_cast<Ieee80211DataOrMgmtFrame *>(cQueue::remove(frame));
}

Ieee80211DataOrMgmtFrame *Ieee80211Queue::pop()
{
    return check_and_cast<Ieee80211DataOrMgmtFrame *>(cQueue::pop());
}

Ieee80211DataOrMgmtFrame *Ieee80211Queue::front() const
{
    return check_and_cast<Ieee80211DataOrMgmtFrame *>(cQueue::front());
}

Ieee80211DataOrMgmtFrame *Ieee80211Queue::back() const
{
    return check_and_cast<Ieee80211DataOrMgmtFrame *>(cQueue::back());
}

bool Ieee80211Queue::contains(Ieee80211DataOrMgmtFrame *frame) const
{
    return cQueue::contains(frame);
}

PendingQueue::PendingQueue(int maxQueueSize, const char *name) :
    Ieee80211Queue(maxQueueSize, name)
{
}

PendingQueue::PendingQueue(int maxQueueSize, const char *name, Priority priority) :
    Ieee80211Queue(maxQueueSize, name)
{
    if (priority == Priority::PRIORITIZE_MGMT_OVER_DATA)
        setup((CompareFunc)cmpMgmtOverData);
    else if (priority == Priority::PRIORITIZE_MULTICAST_OVER_DATA)
        setup((CompareFunc)cmpMgmtOverMulticastOverUnicast);
    else
        throw cRuntimeError("Unknown 802.11 queue priority");
    this->maxQueueSize = maxQueueSize;
}

int PendingQueue::cmpMgmtOverData(Ieee80211DataOrMgmtFrame *a, Ieee80211DataOrMgmtFrame *b)
{
    int aPri = dynamic_cast<Ieee80211ManagementFrame*>(a) ? 1 : 0;  //TODO there should really exist a high-performance isMgmtFrame() function!
    int bPri = dynamic_cast<Ieee80211ManagementFrame*>(b) ? 1 : 0;
    return bPri - aPri;
}

int PendingQueue::cmpMgmtOverMulticastOverUnicast(Ieee80211DataOrMgmtFrame *a, Ieee80211DataOrMgmtFrame *b)
{
    int aPri = dynamic_cast<Ieee80211ManagementFrame*>(a) ? 2 : a->getReceiverAddress().isMulticast() ? 1 : 0;
    int bPri = dynamic_cast<Ieee80211ManagementFrame*>(a) ? 2 : b->getReceiverAddress().isMulticast() ? 1 : 0;
    return bPri - aPri;
}

} /* namespace inet */
} /* namespace ieee80211 */
