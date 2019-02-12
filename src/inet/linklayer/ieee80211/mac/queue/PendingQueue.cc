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

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/queue/PendingQueue.h"

namespace inet {
namespace ieee80211 {

Define_Module(PendingQueue);

simsignal_t PendingQueue::packetEnqueuedSignal = cComponent::registerSignal("packetEnqueued");
simsignal_t PendingQueue::packetDequeuedSignal = cComponent::registerSignal("packetDequeued");

void PendingQueue::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        if (par("prioritizeMulticast"))
            queue.setup((CompareFunc)cmpMgmtOverMulticastOverUnicast);
        else
            queue.setup((CompareFunc)cmpMgmtOverData);
        maxQueueSize = par("maxQueueSize");
    }
}

bool PendingQueue::insert(Packet *frame)
{
    if (maxQueueSize != -1 && queue.getLength() == maxQueueSize)
        return false;
    queue.insert(frame);
    emit(packetEnqueuedSignal, frame);
    return true;
}

bool PendingQueue::insertBefore(Packet *where, Packet *frame)
{
    if (maxQueueSize != -1 && queue.getLength() == maxQueueSize)
        return false;
    queue.insertBefore(where, frame);
    emit(packetEnqueuedSignal, frame);
    return true;
}

bool PendingQueue::insertAfter(Packet *where, Packet *frame)
{
    if (maxQueueSize != -1 && queue.getLength() == maxQueueSize)
        return false;
    queue.insertAfter(where, frame);
    emit(packetEnqueuedSignal, frame);
    return true;
}

Packet *PendingQueue::remove(Packet *frame)
{
    auto packet = check_and_cast<Packet *>(queue.remove(frame));
    emit(packetDequeuedSignal, packet);
    return packet;
}

Packet *PendingQueue::pop()
{
    auto packet = check_and_cast<Packet *>(queue.pop());
    emit(packetDequeuedSignal, packet);
    return packet;
}

Packet *PendingQueue::front() const
{
    return check_and_cast<Packet *>(queue.front());
}

Packet *PendingQueue::back() const
{
    return check_and_cast<Packet *>(queue.back());
}

bool PendingQueue::contains(Packet *frame) const
{
    return queue.contains(frame);
}

int PendingQueue::cmpMgmtOverData(Packet *a, Packet *b)
{
    int aPri = dynamicPtrCast<const Ieee80211MgmtHeader>(a->peekAtFront<Ieee80211MacHeader>()) ? 1 : 0;  //TODO there should really exist a high-performance isMgmtFrame() function!
    int bPri = dynamicPtrCast<const Ieee80211MgmtHeader>(b->peekAtFront<Ieee80211MacHeader>()) ? 1 : 0;
    return bPri - aPri;
}

int PendingQueue::cmpMgmtOverMulticastOverUnicast(Packet *a, Packet *b)
{
    const auto& aHeader = a->peekAtFront<Ieee80211MacHeader>();
    const auto& bHeader = b->peekAtFront<Ieee80211MacHeader>();
    int aPri = dynamicPtrCast<const Ieee80211MgmtHeader>(aHeader) ? 2 : aHeader->getReceiverAddress().isMulticast() ? 1 : 0;
    int bPri = dynamicPtrCast<const Ieee80211MgmtHeader>(bHeader) ? 2 : bHeader->getReceiverAddress().isMulticast() ? 1 : 0;
    return bPri - aPri;
}

} /* namespace inet */
} /* namespace ieee80211 */
