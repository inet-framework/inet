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
        if (ackHandler->isEligibleToTransmit(frame))
            return true;
    }
    return false;
}

void InProgressFrames::ensureHasFrameToTransmit()
{
//    TODO: delete old frames from inProgressFrames
//    if (auto dataFrame = dynamic_cast<Ieee80211DataFrame*>(frame)) {
//        if (transmitLifetimeHandler->isLifetimeExpired(dataFrame))
//            return frame;
//    }
    if (!hasEligibleFrameToTransmit()) {
        auto frames = dataService->extractFramesToTransmit(pendingQueue);
        if (frames) {
            for (auto frame : *frames) {
                ackHandler->frameGotInProgress(frame);
                inProgressFrames.push_back(frame);
            }
            delete frames;
        }
    }
}

Ieee80211DataOrMgmtFrame *InProgressFrames::getFrameToTransmit()
{
    ensureHasFrameToTransmit();
    for (auto frame : inProgressFrames) {
        if (ackHandler->isEligibleToTransmit(frame))
            return frame;
    }
    return nullptr;
}

Ieee80211DataOrMgmtFrame* InProgressFrames::getPendingFrameFor(Ieee80211Frame *frame)
{
    auto frameToTransmit = getFrameToTransmit();
    if (dynamic_cast<Ieee80211RTSFrame *>(frame))
        return frameToTransmit;
    else {
        for (auto frame : inProgressFrames) {
            if (ackHandler->isEligibleToTransmit(frame) && frameToTransmit != frame)
                return frame;
        }
        auto frames = dataService->extractFramesToTransmit(pendingQueue);
        if (frames) {
            auto firstFrame = (*frames)[0];
            for (auto frame : *frames) {
                ackHandler->frameGotInProgress(frame);
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

void InProgressFrames::dropFrame(Ieee80211DataOrMgmtFrame* frame)
{
    inProgressFrames.remove(frame);
}

void InProgressFrames::dropFrames(std::set<std::pair<MACAddress, std::pair<Tid, SequenceControlField>>> seqAndFragNums)
{
    SequenceControlPredicate predicate(seqAndFragNums);
    inProgressFrames.remove_if(predicate);
}

std::vector<Ieee80211DataFrame*> InProgressFrames::getOutstandingFrames()
{
    std::vector<Ieee80211DataFrame*> outstandingFrames;
    for (auto frame : inProgressFrames) {
        if (ackHandler->isOutstandingFrame(frame))
            outstandingFrames.push_back(check_and_cast<Ieee80211DataFrame*>(frame));
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
