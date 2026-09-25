//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/queue/InProgressFrames.h"

#include <algorithm>

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
        if (ackHandler->isEligibleToTransmit(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()))
            return true;
    }
    return false;
}

TxopFrameIdentity InProgressFrames::getFrameIdentity(const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    auto data = dynamicPtrCast<const Ieee80211DataHeader>(header);
    return {header->getReceiverAddress(), data && data->getType() == ST_DATA_WITH_QOS ? data->getTid() : -1,
            header->getSequenceNumber().get()};
}

Packet *InProgressFrames::findPreparedCandidate(const Packet *excluded) const
{
    for (auto frame : inProgressFrames)
        if (frame != excluded && ackHandler->isEligibleToTransmit(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()))
            return frame;
    return nullptr;
}

void InProgressFrames::prepareCandidate(const Packet *excluded)
{
    if (findPreparedCandidate(excluded) != nullptr)
        return;
    auto frames = dataService->extractFramesToTransmit(pendingQueue);
    if (frames == nullptr)
        return;
    for (auto frame : *frames) {
        take(frame);
        auto header = frame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        ackHandler->frameGotInProgress(header);
        auto& history = fragmentHistory[getFrameIdentity(header)];
        history.fragmentCount = std::max(history.fragmentCount, header->getFragmentNumber() + 1);
        inProgressFrames.push_back(frame);
        frame->setArrivalTime(simTime());
        emit(packetEnqueuedSignal, frame);
    }
    delete frames;
}

void InProgressFrames::describeTransmission(TxopExchangePlan& plan) const
{
    auto header = plan.payload->peekAtFront<Ieee80211DataOrMgmtHeader>();
    plan.identity = getFrameIdentity(header);
    plan.fragmentNumber = header->getFragmentNumber();
    plan.packetId = plan.payload->getId();
    plan.dataOrManagement = true;
    plan.group = header->getReceiverAddress().isMulticast();
    plan.retry = header->getRetry();
    auto it = fragmentHistory.find(plan.identity);
    if (it != fragmentHistory.end()) {
        const auto& history = it->second;
        auto length = history.transmittedLengths.find(header->getFragmentNumber());
        plan.unchangedRetry = plan.retry && length != history.transmittedLengths.end() && length->second == plan.payload->getDataLength();
        plan.initialFragmentAfterRetry = !plan.retry && history.retried && history.fragmentCount > 1;
        plan.sixteenFragments = history.fragmentCount == 16;
    }
}

void InProgressFrames::recordTransmission(Packet *packet)
{
    auto header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    auto& history = fragmentHistory[getFrameIdentity(header)];
    history.transmittedLengths.emplace(header->getFragmentNumber(), packet->getDataLength());
    history.retried |= header->getRetry();
}

void InProgressFrames::ensureHasFrameToTransmit()
{
    prepareCandidate();
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
    if (dynamicPtrCast<const Ieee80211RtsFrame>(frame->peekAtFront<Ieee80211MacHeader>()))
        return findPreparedCandidate();
    return findPreparedCandidate(frame);
}

void InProgressFrames::dropFrame(Packet *packet)
{
    EV_DEBUG << "Dropping frame " << packet->getName() << ".\n";
    inProgressFrames.erase(std::remove(inProgressFrames.begin(), inProgressFrames.end(), packet), inProgressFrames.end());
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
        if (ackHandler->isOutstandingFrame(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()))
            outstandingFrames.push_back(frame);
    }
    return outstandingFrames;
}

void InProgressFrames::clearDroppedFrames()
{
    Enter_Method("clearDroppedFrames");
    for (auto it = fragmentHistory.begin(); it != fragmentHistory.end();) {
        bool retained = false;
        for (auto frame : inProgressFrames)
            retained |= getFrameIdentity(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()) == it->first;
        if (!retained)
            it = fragmentHistory.erase(it);
        else
            ++it;
    }
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
