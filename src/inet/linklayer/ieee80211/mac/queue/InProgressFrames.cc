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

#include "InProgressFrames.h"

namespace inet {
namespace ieee80211 {

bool InProgressFrames::hasEligibleFrameToTransmit()
{
    for (auto frame : inProgressFrames) {
        if (ackHandler->isEligibleToTransmit(frame->peekHeader<Ieee80211DataOrMgmtHeader>()))
            return true;
    }
    return false;
}

void InProgressFrames::ensureHasFrameToTransmit()
{
//    TODO: delete old frames from inProgressFrames
//    if (auto dataFrame = dynamic_cast<Ieee80211DataHeader*>(frame)) {
//        if (transmitLifetimeHandler->isLifetimeExpired(dataFrame))
//            return frame;
//    }
    if (!hasEligibleFrameToTransmit()) {
        auto frames = dataService->extractFramesToTransmit(pendingQueue);
        if (frames) {
            for (auto frame : *frames) {
                ackHandler->frameGotInProgress(frame->peekHeader<Ieee80211DataOrMgmtHeader>());
                inProgressFrames.push_back(frame);
            }
            delete frames;
        }
    }
}

Packet *InProgressFrames::getFrameToTransmit()
{
    ensureHasFrameToTransmit();
    for (auto frame : inProgressFrames) {
        if (ackHandler->isEligibleToTransmit(frame->peekHeader<Ieee80211DataOrMgmtHeader>()))
            return frame;
    }
    return nullptr;
}

Packet *InProgressFrames::getPendingFrameFor(Packet *frame)
{
    auto frameToTransmit = getFrameToTransmit();
    if (std::dynamic_pointer_cast<Ieee80211RtsFrame>(frame->peekHeader<Ieee80211MacHeader>()))
        return frameToTransmit;
    else {
        for (auto frame : inProgressFrames) {
            if (ackHandler->isEligibleToTransmit(frame->peekHeader<Ieee80211DataOrMgmtHeader>()) && frameToTransmit != frame)
                return frame;
        }
        auto frames = dataService->extractFramesToTransmit(pendingQueue);
        if (frames) {
            auto firstFrame = (*frames)[0];
            for (auto frame : *frames) {
                ackHandler->frameGotInProgress(frame->peekHeader<Ieee80211DataOrMgmtHeader>());
                inProgressFrames.push_back(frame);
            }
            delete frames;
            // FIXME: If the next Txop sequence were a BlockAckReqBlockAckFs then this would return
            // a wrong pending frame.
            return firstFrame;
        }
        else
            return nullptr;
    }
}

void InProgressFrames::dropFrame(Packet *packet)
{
    inProgressFrames.remove(packet);
}

void InProgressFrames::dropFrames(std::set<std::pair<MACAddress, std::pair<Tid, SequenceControlField>>> seqAndFragNums)
{
    SequenceControlPredicate predicate(seqAndFragNums);
    inProgressFrames.remove_if(predicate);
}

std::vector<Ieee80211DataHeader*> InProgressFrames::getOutstandingFrames()
{
    std::vector<Ieee80211DataHeader*> outstandingFrames;
    for (auto frame : inProgressFrames) {
        if (ackHandler->isOutstandingFrame(frame->peekHeader<Ieee80211DataOrMgmtHeader>()))
            outstandingFrames.push_back(check_and_cast<Ieee80211DataHeader*>(frame));
    }
    return outstandingFrames;
}

InProgressFrames::~InProgressFrames()
{
    for (auto frame : inProgressFrames)
        delete frame;
}

} /* namespace ieee80211 */
} /* namespace inet */
