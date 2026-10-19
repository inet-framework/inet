//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"

#include <algorithm>

#include "inet/linklayer/ieee80211/mac/Ieee80211DataSequenceTag_m.h"
#include "inet/linklayer/ieee80211/mac/contract/IOriginatorQosMacDataService.h"

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

        WATCH(inProgressFrames);
        WATCH(droppedFrames);
        WATCH_EXPR("numInProgress", inProgressFrames.size());
    }
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
    SimpleModule::forEachChild(v);
    for (auto frame : inProgressFrames)
        v->visit(frame);
}

bool InProgressFrames::hasEligibleFrameToTransmit()
{
    for (auto frame : inProgressFrames) {
        if (isEligibleToTransmit(frame))
            return true;
    }
    return false;
}

bool InProgressFrames::containsFrame(const Packet *packet) const
{
    return std::find(inProgressFrames.begin(), inProgressFrames.end(), packet) != inProgressFrames.end();
}

bool InProgressFrames::isEligibleToTransmit(Packet *packet)
{
    return (!lifetimeHandler || !lifetimeHandler->isExpired(packet)) &&
        (!transmissionEligible || transmissionEligible(packet)) &&
        ackHandler->isEligibleToTransmit(packet->peekAtFront<Ieee80211DataOrMgmtHeader>());
}

std::vector<Packet *> InProgressFrames::getEligibleFrames()
{
    std::vector<Packet *> eligible;
    for (auto packet : inProgressFrames)
        if (isEligibleToTransmit(packet))
            eligible.push_back(packet);
    return eligible;
}

void InProgressFrames::ensureHasFrameToTransmit()
{
    if (!hasEligibleFrameToTransmit()) {
        auto frames = dataService->extractFramesToTransmit(pendingQueue);
        if (frames) {
            for (auto frame : *frames) {
                EV_DEBUG << "Inserting frame " << frame->getName() << " extracted from MAC data service.\n";
                take(frame);
                ackHandler->frameGotInProgress(frame->peekAtFront<Ieee80211DataOrMgmtHeader>());
                inProgressFrames.push_back(frame);
                frame->setArrivalTime(simTime());
                emit(packetEnqueuedSignal, frame);
            }
            delete frames;
        }
    }
}

void InProgressFrames::adoptPreparedFrame(Packet *packet)
{
    Enter_Method("adoptPreparedFrame");
    take(packet);
    ackHandler->frameGotInProgress(packet->peekAtFront<Ieee80211DataOrMgmtHeader>());
    inProgressFrames.push_back(packet);
    packet->setArrivalTime(simTime());
    emit(packetEnqueuedSignal, packet);
}

void InProgressFrames::adoptPreparedFragments(const std::vector<Packet *>& fragments)
{
    Enter_Method("adoptPreparedFragments");
    ASSERT(!fragments.empty());
    fragmentHistory[fragments.front()->getTag<Ieee80211DataSequenceTag>()->getAllocationIndex()].count = fragments.size();
    for (auto packet : fragments)
        adoptPreparedFrame(packet);
}

void InProgressFrames::replaceWithPreparedFragments(Packet *source, const std::vector<Packet *>& fragments)
{
    Enter_Method("replaceWithPreparedFragments");
    auto position = std::find(inProgressFrames.begin(), inProgressFrames.end(), source);
    ASSERT(position != inProgressFrames.end() && !fragments.empty());
    auto index = position - inProgressFrames.begin();
    auto allocation = source->getTag<Ieee80211DataSequenceTag>()->getAllocationIndex();
    for (auto packet : fragments)
        ASSERT(packet->getTag<Ieee80211DataSequenceTag>()->getAllocationIndex() == allocation);
    ackHandler->dropFrame(source->peekAtFront<Ieee80211DataOrMgmtHeader>());
    dropFrame(source);
    fragmentHistory[allocation].count = fragments.size();
    for (auto packet : fragments) {
        take(packet);
        ackHandler->frameGotInProgress(packet->peekAtFront<Ieee80211DataOrMgmtHeader>());
        inProgressFrames.insert(inProgressFrames.begin() + index++, packet);
        packet->setArrivalTime(simTime());
        emit(packetEnqueuedSignal, packet);
    }
}

std::vector<Packet *> InProgressFrames::getAllocationFrames(Packet *packet) const
{
    std::vector<Packet *> result;
    auto tag = packet->findTag<Ieee80211DataSequenceTag>();
    if (tag) {
        for (auto candidate : inProgressFrames) {
            auto other = candidate->findTag<Ieee80211DataSequenceTag>();
            if (other && other->getAllocationIndex() == tag->getAllocationIndex())
                result.push_back(candidate);
        }
    }
    return result;
}

void InProgressFrames::recordDataTransmission(Packet *packet)
{
    auto allocation = packet->getTag<Ieee80211DataSequenceTag>()->getAllocationIndex();
    auto fragment = packet->peekAtFront<Ieee80211DataHeader>()->getFragmentNumber();
    auto& history = fragmentHistory[allocation];
    auto result = history.transmittedLengths.emplace(fragment, packet->getDataLength());
    if (!result.second) {
        ASSERT(result.first->second == packet->getDataLength());
        history.retransmitted.insert(fragment);
    }
}

bool InProgressFrames::isSameSizeDataRetry(Packet *packet) const
{
    auto tag = packet->findTag<Ieee80211DataSequenceTag>();
    auto it = tag ? fragmentHistory.find(tag->getAllocationIndex()) : fragmentHistory.end();
    if (it == fragmentHistory.end())
        return false;
    auto length = it->second.transmittedLengths.find(packet->peekAtFront<Ieee80211DataHeader>()->getFragmentNumber());
    return length != it->second.transmittedLengths.end() && length->second == packet->getDataLength();
}

bool InProgressFrames::hasRetriedPreviousFragment(Packet *packet) const
{
    auto tag = packet->findTag<Ieee80211DataSequenceTag>();
    auto it = tag ? fragmentHistory.find(tag->getAllocationIndex()) : fragmentHistory.end();
    return it != fragmentHistory.end() && !it->second.retransmitted.empty() &&
        *it->second.retransmitted.begin() < packet->peekAtFront<Ieee80211DataHeader>()->getFragmentNumber();
}

int InProgressFrames::getFragmentCount(Packet *packet) const
{
    auto tag = packet->findTag<Ieee80211DataSequenceTag>();
    auto it = tag ? fragmentHistory.find(tag->getAllocationIndex()) : fragmentHistory.end();
    return it == fragmentHistory.end() ? 1 : it->second.count;
}

void InProgressFrames::dropAllocation(Packet *packet)
{
    Enter_Method("dropAllocation");
    for (auto member : getAllocationFrames(packet)) {
        ackHandler->dropFrame(member->peekAtFront<Ieee80211DataOrMgmtHeader>());
        if (lifetimeHandler)
            lifetimeHandler->forgetFrame(member->getId());
        dropFrame(member);
    }
}

Packet *InProgressFrames::getFrameToTransmit()
{
    ensureHasFrameToTransmit();
    for (auto frame : inProgressFrames) {
        if (isEligibleToTransmit(frame))
            return frame;
    }
    return nullptr;
}

Packet *InProgressFrames::peekFrameToTransmit()
{
    for (auto frame : inProgressFrames) {
        if (isEligibleToTransmit(frame))
            return frame;
    }
    return check_and_cast<IOriginatorQosMacDataService *>(dataService)->peekEligibleFrame(pendingQueue);
}

SequenceNumberCyclic InProgressFrames::getCandidateSequenceNumber(Packet *packet) const
{
    auto header = packet->peekAtFront<Ieee80211DataHeader>();
    if (containsFrame(packet))
        return header->getSequenceNumber();
    return check_and_cast<IOriginatorQosMacDataService *>(dataService)->getNextSequenceNumber(header->getReceiverAddress(), header->getTid());
}

Packet *InProgressFrames::getPendingFrameFor(Packet *frame, bool extract)
{
    auto frameToTransmit = extract ? getFrameToTransmit() : peekFrameToTransmit();
    if (dynamicPtrCast<const Ieee80211RtsFrame>(frame->peekAtFront<Ieee80211MacHeader>()))
        return frameToTransmit;
    else {
        for (auto frame : inProgressFrames) {
            if (isEligibleToTransmit(frame) && frameToTransmit != frame)
                return frame;
        }
        if (!extract)
            return check_and_cast<IOriginatorQosMacDataService *>(dataService)->peekEligibleFrame(pendingQueue);
        auto frames = dataService->extractFramesToTransmit(pendingQueue);
        if (frames) {
            auto firstFrame = (*frames)[0];
            for (auto frame : *frames) {
                take(frame);
                ackHandler->frameGotInProgress(frame->peekAtFront<Ieee80211DataOrMgmtHeader>());
                inProgressFrames.push_back(frame);
                frame->setArrivalTime(simTime());
                emit(packetEnqueuedSignal, frame);
            }
            delete frames;
            // FIXME If the next Txop sequence were a BlockAckReqBlockAckFs then this would return
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
    if (auto tag = packet->findTag<Ieee80211DataSequenceTag>())
        if (getAllocationFrames(packet).empty())
            fragmentHistory.erase(tag->getAllocationIndex());
    droppedFrames.push_back(packet);
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
        if ((!lifetimeHandler || !lifetimeHandler->isExpired(frame)) &&
            ackHandler->isOutstandingFrame(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()))
            outstandingFrames.push_back(frame);
    }
    return outstandingFrames;
}

void InProgressFrames::clearDroppedFrames()
{
    Enter_Method("clearDroppedFrames");
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
