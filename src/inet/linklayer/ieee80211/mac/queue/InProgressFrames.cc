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

#include <algorithm>
#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"

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

void InProgressFrames::dropFrame(Ieee80211DataOrMgmtFrame *frame)
{
    inProgressFrames.remove(frame);
    droppedFrames.push_back(frame);
}

void InProgressFrames::dropFrames(std::set<std::pair<MACAddress, std::pair<Tid, SequenceControlField>>> seqAndFragNums)
{
    for (auto it = inProgressFrames.begin(); it != inProgressFrames.end();) {
        auto frame = *it;
        if (frame->getType() == ST_DATA_WITH_QOS) {
            auto dataFrame = check_and_cast<const Ieee80211DataFrame*>(frame);
            if (seqAndFragNums.count(std::make_pair(dataFrame->getReceiverAddress(), std::make_pair(dataFrame->getTid(), SequenceControlField(dataFrame->getSequenceNumber(), dataFrame->getFragmentNumber())))) != 0) {
                it = inProgressFrames.erase(it);
                droppedFrames.push_back(frame);
            }
            else
                it++;
        }
        else
            it++;
    }
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

void InProgressFrames::clearDroppedFrames()
{
    for (auto frame : droppedFrames)
        delete frame;
    droppedFrames.clear();
}

InProgressFrames::~InProgressFrames()
{
    for (auto frame : inProgressFrames)
        delete frame;
    for (auto frame : droppedFrames)
        delete frame;
}

} /* namespace ieee80211 */
} /* namespace inet */
