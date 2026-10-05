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
        if (!removingFrames.count(frame) && ackHandler->isEligibleToTransmit(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()))
            return true;
    }
    return false;
}

void InProgressFrames::ensureHasFrameToTransmit()
{
//    TODO delete old frames from inProgressFrames
//    if (auto dataFrame = dynamic_cast<Ieee80211DataHeader*>(frame)) {
//        if (transmitLifetimeHandler->isLifetimeExpired(dataFrame))
//            return frame;
//    }
    if (!hasEligibleFrameToTransmit())
        extractAndRegisterFrames();
}

bool InProgressFrames::extractAndRegisterFrames()
{
    std::unique_ptr<std::vector<Packet *>> frames(dataService->extractFramesToTransmit(pendingQueue));
    if (!frames || frames->empty())
        return false;
    if (frames->size() > 16)
        throw cRuntimeError("A conventional fragment set cannot exceed 16 fragments");
    // The data service returns the complete conventional fragment set in one extraction.
    auto unit = std::make_shared<UnitHistory>();
    unit->identity = frames->front()->getId();
    unit->transmissions.resize(frames->size(), 0);
    for (auto frame : *frames) {
        auto header = frame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        if (header->getFragmentNumber() >= unit->transmissions.size())
            throw cRuntimeError("Incomplete conventional fragment set from the data service");
        take(frame);
        ackHandler->frameGotInProgress(header);
        frameHistories.emplace(frame->getId(), FrameHistory{unit, frame->getDataLength(), header->getFragmentNumber()});
        inProgressFrames.push_back(frame);
        frame->setArrivalTime(simTime());
        emit(packetEnqueuedSignal, frame);
    }
    return true;
}

void InProgressFrames::stageForPlanning()
{
    Enter_Method("stageForPlanning");
    for (;;) {
        size_t eligible = 0;
        for (auto frame : inProgressFrames)
            if (!removingFrames.count(frame))
                eligible += ackHandler->snapshotFrameState(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()).eligible;
        if (eligible >= 2 || !extractAndRegisterFrames())
            return;
    }
}

std::vector<StagedFrameView> InProgressFrames::inspectStagedFrames() const
{
    std::vector<StagedFrameView> result;
    for (auto frame : inProgressFrames) {
        if (removingFrames.count(frame))
            continue;
        const auto& history = frameHistories.at(frame->getId());
        auto header = frame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        StagedFrameView view;
        view.frame = frame;
        view.owner = this;
        view.identity = frame->getId();
        view.epoch = lifecycleEpoch;
        view.unitIdentity = history.unit->identity;
        view.receiver = header->getReceiverAddress();
        if (header->getType() == ST_DATA_WITH_QOS)
            view.tid = staticPtrCast<const Ieee80211DataHeader>(header)->getTid();
        view.sequenceControl = SequenceControlField(header->getSequenceNumber().get(), header->getFragmentNumber());
        view.length = frame->getDataLength();
        view.originalLength = history.originalLength;
        view.fragmentCount = history.unit->transmissions.size();
        view.transmissions = history.unit->transmissions.at(history.fragment);
        for (int i = 0; i < history.fragment; i++)
            view.earlierFragmentRetransmitted |= history.unit->transmissions[i] > 1;
        view.ack = ackHandler->snapshotFrameState(header);
        result.push_back(view);
    }
    return result;
}

bool InProgressFrames::isRetained(const StagedFrameView& view) const
{
    if (view.owner != this || view.epoch != lifecycleEpoch)
        return false;
    for (auto frame : inProgressFrames)
        if (frame == view.frame && frame->getId() == view.identity && !removingFrames.count(frame)) {
            auto header = frame->peekAtFront<Ieee80211DataOrMgmtHeader>();
            auto tid = header->getType() == ST_DATA_WITH_QOS ? staticPtrCast<const Ieee80211DataHeader>(header)->getTid() : -1;
            return frame->getDataLength() == view.length && header->getReceiverAddress() == view.receiver && tid == view.tid &&
                header->getSequenceNumber().get() == view.sequenceControl.getSequenceNumber() &&
                header->getFragmentNumber() == view.sequenceControl.getFragmentNumber();
        }
    return false;
}

void InProgressFrames::recordTransmission(const StagedFrameView& view)
{
    if (!isRetained(view))
        throw cRuntimeError("Transmission refers to a removed staged frame");
    auto& history = frameHistories.at(view.identity);
    history.unit->transmissions.at(history.fragment)++;
}

void InProgressFrames::resetForLifecycle()
{
    Enter_Method("resetForLifecycle");
    lifecycleEpoch++;
    auto retained = inProgressFrames;
    for (auto frame : retained) {
        ackHandler->dropFrame(frame->peekAtFront<Ieee80211DataOrMgmtHeader>());
        dropFrame(frame);
    }
    // A synchronous callback can still borrow a dropped frame. Normal deferred cleanup owns disposal.
}

std::string InProgressFrames::getFragmentationDescription() const
{
    auto service = dynamic_cast<cModule *>(dataService);
    auto policy = service ? service->getSubmodule("fragmentationPolicy") : nullptr;
    if (!policy)
        return "fragmentationPolicy=absent";
    return "fragmentationPolicy=" + policy->getFullPath() + " fragmentationThreshold=" +
        (policy->hasPar("fragmentationThreshold") ? policy->par("fragmentationThreshold").str() : "absent");
}

Packet *InProgressFrames::getFrameToTransmit()
{
    ensureHasFrameToTransmit();
    for (auto frame : inProgressFrames) {
        if (!removingFrames.count(frame) && ackHandler->isEligibleToTransmit(frame->peekAtFront<Ieee80211DataOrMgmtHeader>()))
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
        auto previousSize = inProgressFrames.size();
        if (extractAndRegisterFrames()) {
            auto firstFrame = inProgressFrames.at(previousSize);
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
    if (removingFrames.count(packet) || std::find(inProgressFrames.begin(), inProgressFrames.end(), packet) == inProgressFrames.end())
        return;
    EV_DEBUG << "Dropping frame " << packet->getName() << ".\n";
    removingFrames.insert(packet);
    // Cancellation can retire a context synchronously. Preserve its borrowed original until this call returns.
    retainFrameReferences();
    if (removalCallback)
        removalCallback->frameWillBeRemoved(this, packet);
    inProgressFrames.erase(std::remove(inProgressFrames.begin(), inProgressFrames.end(), packet), inProgressFrames.end());
    frameHistories.erase(packet->getId());
    droppedFrames.push_back(packet);
    emit(packetDequeuedSignal, packet);
    removingFrames.erase(packet);
    releaseFrameReferences();
}

void InProgressFrames::dropFrames(std::set<std::pair<MacAddress, std::pair<Tid, SequenceControlField>>> seqAndFragNums)
{
    auto retained = inProgressFrames;
    for (auto frame : retained) {
        auto header = frame->peekAtFront<Ieee80211MacHeader>();
        if (header->getType() == ST_DATA_WITH_QOS) {
            auto dataHeader = staticPtrCast<const Ieee80211DataHeader>(header);
            auto key = std::make_pair(dataHeader->getReceiverAddress(), std::make_pair(dataHeader->getTid(),
                SequenceControlField(dataHeader->getSequenceNumber().get(), dataHeader->getFragmentNumber())));
            if (seqAndFragNums.count(key))
                dropFrame(frame);
        }
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
    if (frameReferenceUsers != 0)
        return;
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
