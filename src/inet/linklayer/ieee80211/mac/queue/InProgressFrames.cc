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

Define_Module(InProgressFrames);

simsignal_t InProgressFrames::packetEnqueuedSignal = cComponent::registerSignal("packetEnqueued");
simsignal_t InProgressFrames::packetDequeuedSignal = cComponent::registerSignal("packetDequeued");

void InProgressFrames::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        pendingQueue = check_and_cast<queueing::IPacketQueue *>(getModuleByPath(par("pendingQueueModule")));
        dataService = check_and_cast<IOriginatorMacDataService *>(getModuleByPath(par("originatorMacDataServiceModule")));
        ackHandler = check_and_cast<IAckHandler *>(getModuleByPath(par("ackHandlerModule")));
        updateDisplayString();
    }
}

void InProgressFrames::updateDisplayString()
{
    std::string text = std::to_string(inProgressFrames.size()) + " packets";
    getDisplayString().setTagArg("t", 0, text.c_str());
}

std::string InProgressFrames::str() const
{
    if (inProgressFrames.size() == 0)
        return std::string("empty");
    std::stringstream out;
    out << "length=" << inProgressFrames.size();
    return out.str();
}

void InProgressFrames::forEachChild(cVisitor *v)
{
    cSimpleModule::forEachChild(v);
    for (auto frame : inProgressFrames)
        v->visit(frame);
}

bool InProgressFrames::hasEligibleFrameToTransmit()
{
    for (auto frame : inProgressFrames) {
        if (ackHandler->isEligibleToTransmit(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()))
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
                EV_DEBUG << "Inserting frame " << frame->getName() << " extracted from MAC data service.\n";
                ackHandler->frameGotInProgress(frame->peekAtFront<Ieee80211DataOrMgmtHeader>());
                inProgressFrames.push_back(frame);
                frame->setArrivalTime(simTime());
                updateDisplayString();
                emit(packetEnqueuedSignal, frame);
            }
            delete frames;
        }
    }
}

Packet *InProgressFrames::getFrameToTransmit()
{
    ensureHasFrameToTransmit();
    for (auto frame : inProgressFrames) {
        if (ackHandler->isEligibleToTransmit(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()))
            return frame;
    }
    return nullptr;
}

Packet *InProgressFrames::getPendingFrameFor(Packet *frame)
{
    auto frameToTransmit = getFrameToTransmit();
    if (dynamicPtrCast<const Ieee80211RtsFrame>(frame->peekAtFront<Ieee80211MacHeader>()))
        return frameToTransmit;
    else {
        for (auto frame : inProgressFrames) {
            if (ackHandler->isEligibleToTransmit(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()) && frameToTransmit != frame)
                return frame;
        }
        auto frames = dataService->extractFramesToTransmit(pendingQueue);
        if (frames) {
            auto firstFrame = (*frames)[0];
            for (auto frame : *frames) {
                ackHandler->frameGotInProgress(frame->peekAtFront<Ieee80211DataOrMgmtHeader>());
                inProgressFrames.push_back(frame);
                frame->setArrivalTime(simTime());
                updateDisplayString();
                emit(packetEnqueuedSignal, frame);
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
    EV_DEBUG << "Dropping frame " << packet->getName() << ".\n";
    inProgressFrames.erase(std::remove(inProgressFrames.begin(), inProgressFrames.end(), packet), inProgressFrames.end());
    droppedFrames.push_back(packet);
    updateDisplayString();
    emit(packetDequeuedSignal, packet);
}

void InProgressFrames::dropFrames(std::set<std::pair<MacAddress, std::pair<Tid, SequenceControlField>>> seqAndFragNums)
{
    for (auto it = inProgressFrames.begin(); it != inProgressFrames.end();) {
        auto frame = *it;
        auto header = frame->peekAtFront<Ieee80211MacHeader>();
        if (header->getType() == ST_DATA_WITH_QOS) {
            auto dataheader = CHK(dynamicPtrCast<const Ieee80211DataHeader>(header));
            if (seqAndFragNums.count(std::make_pair(dataheader->getReceiverAddress(), std::make_pair(dataheader->getTid(), SequenceControlField(dataheader->getSequenceNumber().get(), dataheader->getFragmentNumber())))) != 0) {
                EV_DEBUG << "Dropping frame " << frame->getName() << ".\n";
                it = inProgressFrames.erase(it);
                droppedFrames.push_back(frame);
                updateDisplayString();
                emit(packetDequeuedSignal, frame);
            }
            else
                it++;
        }
        else
            it++;
    }
}

std::vector<Packet *> InProgressFrames::getOutstandingFrames()
{
    std::vector<Packet *> outstandingFrames;
    for (auto frame : inProgressFrames) {
        if (ackHandler->isOutstandingFrame(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()))
            outstandingFrames.push_back(frame);
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
