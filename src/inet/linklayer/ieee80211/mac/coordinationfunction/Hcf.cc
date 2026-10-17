//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211DataSequenceTag_m.h"
#include "inet/linklayer/ieee80211/mac/contract/FrameTransmissionDetails_m.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"
#include "inet/linklayer/ieee80211/mac/framesequence/HcfFs.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtTransactionTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"
#include "inet/queueing/contract/ISelectivePacketCollection.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"

namespace inet {
namespace ieee80211 {

using namespace inet::physicallayer;

simsignal_t Hcf::edcaCollisionDetectedSignal = cComponent::registerSignal("edcaCollisionDetected");
simsignal_t Hcf::blockAckAgreementAddedSignal = cComponent::registerSignal("blockAckAgreementAdded");
simsignal_t Hcf::blockAckAgreementDeletedSignal = cComponent::registerSignal("blockAckAgreementDeleted");

Define_Module(Hcf);

void Hcf::initialize(int stage)
{
    ModeSetListener::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *configuredVariant = par("blockAckVariant").stringValue();
        if (!strcmp(configuredVariant, "basic"))
            blockAckVariant = BlockAckVariant::BASIC;
        else if (!strcmp(configuredVariant, "compressed"))
            blockAckVariant = BlockAckVariant::COMPRESSED;
        else
            throw cRuntimeError("Unknown blockAckVariant \"%s\": expected \"basic\" or \"compressed\"", configuredVariant);
        if (blockAckVariant == BlockAckVariant::COMPRESSED)
            throw cRuntimeError("Compressed Block Ack exchanges are not implemented");
        mac = check_and_cast<Ieee80211Mac *>(getContainingNicModule(this)->getSubmodule("mac"));
        mib = check_and_cast<Ieee80211Mib *>(mac->getModuleByPath(mac->par("mibModule")));
        startRxTimer = new cMessage("startRxTimeout");
        inactivityTimer = new cMessage("blockAckInactivityTimer");
        dataLifetimeTimer = new cMessage("dataLifetimeTimer");
        addbaResponseTimer = new cMessage("addbaResponseTimer");
        edca = check_and_cast<Edca *>(getSubmodule("edca"));
        hcca = check_and_cast<Hcca *>(getSubmodule("hcca"));
        tx = check_and_cast<ITx *>(getModuleByPath(par("txModule")));
        rx = check_and_cast<IRx *>(getModuleByPath(par("rxModule")));
        dataAndMgmtRateControl = dynamic_cast<IRateControl *>(getSubmodule("rateControl"));
        originatorBlockAckAgreementPolicy = dynamic_cast<IOriginatorBlockAckAgreementPolicy *>(getSubmodule("originatorBlockAckAgreementPolicy"));
        recipientBlockAckAgreementPolicy = dynamic_cast<IRecipientBlockAckAgreementPolicy *>(getSubmodule("recipientBlockAckAgreementPolicy"));
        rateSelection = check_and_cast<IQosRateSelection *>(getSubmodule("rateSelection"));
        frameSequenceHandler = new FrameSequenceHandler();
        WATCH_EXPR("frameSequenceInfo", getFrameSequenceInfo());
        originatorDataService = check_and_cast<IOriginatorMacDataService *>(getSubmodule(("originatorMacDataService")));
        recipientDataService = check_and_cast<IRecipientQosMacDataService *>(getSubmodule("recipientMacDataService"));
        originatorAckPolicy = check_and_cast<IOriginatorQoSAckPolicy *>(getSubmodule("originatorAckPolicy"));
        recipientAckPolicy = check_and_cast<IRecipientQosAckPolicy *>(getSubmodule("recipientAckPolicy"));
        singleProtectionMechanism = check_and_cast<SingleProtectionMechanism *>(getSubmodule("singleProtectionMechanism"));
        rtsProcedure = new RtsProcedure();
        rtsPolicy = check_and_cast<IRtsPolicy *>(getSubmodule("rtsPolicy"));
        recipientAckProcedure = new RecipientAckProcedure();
        ctsProcedure = new CtsProcedure();
        ctsPolicy = check_and_cast<ICtsPolicy *>(getSubmodule("ctsPolicy"));
        if (originatorBlockAckAgreementPolicy && recipientBlockAckAgreementPolicy) {
            recipientBlockAckAgreementHandler = new RecipientBlockAckAgreementHandler();
            originatorBlockAckAgreementHandler = new OriginatorBlockAckAgreementHandler();
            originatorBlockAckProcedure = new OriginatorBlockAckProcedure();
            recipientBlockAckProcedure = new RecipientBlockAckProcedure();
        }
    }
    else if (stage == INITSTAGE_LAST) {
        // Edca resolves its Edcaf array at the link-layer stage. Install the
        // queue signal listeners after all child initialization has completed.
        auto lifetimeHandler = edca->getLifetimeHandler();
        lifetimeHandler->setDeadlineChangedCallback([this]() { scheduleDataLifetimeTimer(); });
        check_and_cast<OriginatorQosMacDataService *>(originatorDataService)->setLifetimeHandler(
            lifetimeHandler, [this]() { expireDataFrames(); });
        check_and_cast<OriginatorQosMacDataService *>(originatorDataService)->setEligibility(
            [this](Packet *packet) { return isFrameEligible(packet, false); },
            [this](Packet *packet) { return isAMsduPermitted(packet); });
        mib->subscribe(Ieee80211Mib::rateStateChangedSignal, this);
        for (int ac = 0; ac < AC_NUMCATEGORIES; ac++) {
            edca->getEdcaf(static_cast<AccessCategory>(ac))->setFrameEligibility(
                [this, ac]() { return hasEligibleFrameToTransmit(static_cast<AccessCategory>(ac)); });
            check_and_cast<cModule *>(edca->getEdcaf(static_cast<AccessCategory>(ac))->getPendingQueue())->subscribe(packetDroppedSignal, this);
            auto frames = edca->getEdcaf(static_cast<AccessCategory>(ac))->getInProgressFrames();
            frames->setLifetimeHandler(lifetimeHandler);
            frames->setTransmissionEligibility([this](Packet *packet) { return isFrameEligible(packet, true); });
            frames->subscribe(InProgressFrames::packetDequeuedSignal, this);
        }
    }
}

std::string Hcf::getFrameSequenceInfo() const
{
    if (!frameSequenceHandler->isSequenceRunning())
        return "";
    auto history = frameSequenceHandler->getFrameSequence()->getHistory();
    if (history.length() > 32) {
        history.erase(history.begin(), history.end() - 32);
        history = "..." + history;
    }
    return "Fs: " + history;
}

void Hcf::scheduleDataLifetimeTimer()
{
    Enter_Method("scheduleDataLifetimeTimer");
    std::set<int64_t> referenced;
    for (auto packet : getReferencedDataFrames())
        referenced.insert(packet->getId());
    auto deadline = edca->getLifetimeHandler()->getEarliestDeadline(referenced);
    if (deadline != SIMTIME_MAX)
        deadline = std::max(simTime(), deadline);
    if (dataLifetimeTimer->isScheduled() && dataLifetimeTimer->getArrivalTime() == deadline)
        return;
    cancelEvent(dataLifetimeTimer);
    if (deadline != SIMTIME_MAX)
        scheduleAt(deadline, dataLifetimeTimer);
}

std::set<const Packet *> Hcf::getReferencedDataFrames() const
{
    std::set<const Packet *> frames;
    if (frameSequenceHandler->isSequenceRunning()) {
        auto context = frameSequenceHandler->getContext();
        for (int i = 0; i < context->getNumSteps(); i++) {
            auto step = context->getStep(i);
            if (auto transmit = dynamic_cast<ITransmitStep *>(step))
                frames.insert(transmit->getFrameToTransmit());
            if (auto rts = dynamic_cast<RtsTransmitStep *>(step))
                frames.insert(rts->getProtectedFrame());
        }
    }
    return frames;
}

void Hcf::expireDataFrames(bool preserveSequence)
{
    Enter_Method("expireDataFrames");
    auto handler = edca->getLifetimeHandler();
    if (handler->getNumFrames() == 0)
        return;
    if (dataLifetimeTimer->isScheduled() && dataLifetimeTimer->getArrivalTime() > simTime()) {
        if (preserveSequence)
            return;
        bool expiredReference = false;
        for (auto packet : getReferencedDataFrames())
            expiredReference |= handler->isExpired(packet);
        // A safe boundary needs a queue scan only if it resolves expired referenced DATA.
        if (!expiredReference)
            return;
    }
    std::set<int64_t> referenced;
    if (preserveSequence) {
        for (auto packet : getReferencedDataFrames())
            referenced.insert(packet->getId());
    }
    for (auto id : handler->getExpiredFrameIds(referenced)) {
        auto packet = handler->findFrame(id);
        // A drop notification can retire another due frame through its queue owner.
        if (packet == nullptr)
            continue;
        auto header = packet->peekAtFront<Ieee80211DataHeader>();
        auto edcaf = edca->getEdcaf(edca->classifyFrame(header));
        auto frames = edcaf->getInProgressFrames();
        bool inProgress = frames->containsFrame(packet);
        if (inProgress) {
            edcaf->getAckHandler()->dropFrame(header);
            edcaf->getRecoveryProcedure()->dropFrame(header);
            frames->dropFrame(packet);
        }
        else {
            edcaf->getPendingQueue()->removePacket(packet);
            take(packet);
            handler->forgetFrame(packet->getId());
        }
        PacketDropDetails details;
        details.setReason(LIFETIME_EXPIRED);
        emit(packetDroppedSignal, packet, &details);
        if (inProgress)
            firstDataDiscarded(header);
        if (!inProgress)
            delete packet;
    }
    for (int ac = 0; ac < AC_NUMCATEGORIES; ac++) {
        auto frames = edca->getEdcaf(static_cast<AccessCategory>(ac))->getInProgressFrames();
        if (!frameSequenceHandler->isSequenceRunning() ||
            frameSequenceHandler->getContext()->getInProgressFrames() != frames)
            frames->clearDroppedFrames();
    }
    scheduleDataLifetimeTimer();
    advanceOriginatorWindows();
}

void Hcf::forEachChild(cVisitor *v)
{
    SimpleModule::forEachChild(v);
    if (frameSequenceHandler != nullptr && frameSequenceHandler->getContext() != nullptr)
        v->visit(const_cast<FrameSequenceContext *>(frameSequenceHandler->getContext()));
}

void Hcf::handleMessage(cMessage *msg)
{
    if (msg == addbaResponseTimer) {
        std::vector<std::pair<MacAddress, Tid>> expired;
        for (const auto& entry : originatorBlockAckAgreementHandler->getProcedures()) {
            if (entry.second.responseDeadline <= simTime())
                expired.push_back(entry.first);
        }
        for (const auto& id : expired)
            beginTeardown(id.first, id.second, true, 39);
        scheduleAddbaResponseTimer();
        updateContention();
    }
    else if (msg == dataLifetimeTimer) {
        expireDataFrames();
        if (!frameSequenceHandler->isSequenceRunning())
            completeTransfers();
        updateAgreementProcedures();
        updateContention();
    }
    else if (msg == startRxTimer) {
        if (!isReceptionInProgress()) {
            frameSequenceHandler->handleStartRxTimeout();
        }
    }
    else if (msg == inactivityTimer) {
        if (originatorBlockAckAgreementHandler && recipientBlockAckAgreementHandler) {
            std::vector<std::tuple<MacAddress, Tid, bool>> expired;
            for (const auto& entry : originatorBlockAckAgreementHandler->getAgreements()) {
                if (entry.second->getIsAddbaResponseReceived() && entry.second->getExpirationTime() <= simTime())
                    expired.push_back({entry.first.first, entry.first.second, true});
            }
            for (const auto& entry : recipientBlockAckAgreementHandler->getAgreements()) {
                if (entry.second->getExpirationTime() <= simTime())
                    expired.push_back({entry.first.first, entry.first.second, false});
            }
            for (const auto& entry : expired)
                beginTeardown(std::get<0>(entry), std::get<1>(entry), std::get<2>(entry), 39);
            scheduleInactivityTimer();
            updateContention();
        }
        else
            throw cRuntimeError("Unknown event");
    }
    else
        throw cRuntimeError("Unknown msg type");
}

void Hcf::refreshDisplay() const
{
    ModeSetListener::refreshDisplay();
    if (frameSequenceHandler->isSequenceRunning()) {
        auto history = frameSequenceHandler->getFrameSequence()->getHistory();
        getDisplayString().setTagArg("tt", 0, ("Fs: " + history).c_str());
    }
    else {
        getDisplayString().removeTag("tt");
    }
}

void Hcf::processUpperFrame(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    Enter_Method("processUpperFrame(%s)", packet->getName());
    take(packet);
    EV_INFO << "Processing upper frame: " << packet->getName() << endl;
    // TODO
    // A QoS STA should send individually addressed Management frames that are addressed to a non-QoS STA
    // using the access category AC_BE and shall send all other management frames using the access category
    // AC_VO. A QoS STA that does not send individually addressed Management frames that are addressed to a
    // non-QoS STA using the access category AC_BE shall send them using the access category AC_VO.
    // Management frames are exempted from any and all restrictions on transmissions arising from admission
    // control procedures.
    AccessCategory ac = AccessCategory(-1);
    if (dynamicPtrCast<const Ieee80211MgmtHeader>(header)) // TODO + non-QoS frames
        ac = AccessCategory::AC_VO;
    else if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header))
        ac = edca->classifyFrame(dataHeader);
    else
        throw cRuntimeError("Unknown message type");
    EV_INFO << "The upper frame has been classified as a " << printAccessCategory(ac) << " frame." << endl;
    auto pendingQueue = edca->getEdcaf(ac)->getPendingQueue();
    if (header->getType() == ST_DATA_WITH_QOS)
        edca->getLifetimeHandler()->registerFrame(packet, ac);
    pendingQueue->enqueuePacket(packet);
    updateAgreementProcedures();
    if (!pendingQueue->isEmpty() && !handlingAgreementAction && hasFrameToTransmit(ac)) {
        auto edcaf = edca->getChannelOwner();
        if (edcaf == nullptr || edcaf->getAccessCategory() != ac) {
            EV_DETAIL << "Requesting channel for access category " << printAccessCategory(ac) << endl;
            edca->requestChannelAccess(ac, this);
        }
    }
}

void Hcf::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    if (signalID == InProgressFrames::packetDequeuedSignal) {
        Enter_Method("packetDequeued");
        edca->getLifetimeHandler()->forgetFrame(check_and_cast<Packet *>(obj)->getId());
    }
    else if (signalID == packetDroppedSignal) {
        Enter_Method("%s", cComponent::getSignalName(signalID));
        auto packet = check_and_cast<Packet *>(obj);
        edca->getLifetimeHandler()->forgetFrame(packet->getId());
        actionTerminated(packet, false);
        if (packet->findTag<Ieee80211MgmtTransactionTag>() != nullptr) {
            FrameTransmissionDetails transmissionDetails;
            transmissionDetails.setStatus(FRAME_TRANSMISSION_STATUS_DROPPED_BEFORE_TRANSMISSION);
            emit(Ieee80211Mac::frameTransmissionOutcomeSignal, packet, &transmissionDetails);
        }
    }
    else
        ModeSetListener::receiveSignal(source, signalID, obj, details);
}

void Hcf::scheduleStartRxTimer(simtime_t timeout)
{
    Enter_Method("scheduleStartRxTimer");
    scheduleAfter(timeout, startRxTimer);
}

void Hcf::scheduleInactivityTimer()
{
    Enter_Method("scheduleInactivityTimer");
    // IEEE Std 802.11-2024, 11.5.4 requires inactivity expiration. HCF reads each role's absolute deadline.
    simtime_t deadline = SIMTIME_MAX;
    if (originatorBlockAckAgreementHandler && recipientBlockAckAgreementHandler)
        deadline = std::min(originatorBlockAckAgreementHandler->computeEarliestExpirationTime(),
            recipientBlockAckAgreementHandler->computeEarliestExpirationTime());
    cancelEvent(inactivityTimer);
    if (deadline != SIMTIME_MAX)
        scheduleAt(std::max(simTime(), deadline), inactivityTimer);
}

void Hcf::scheduleAddbaResponseTimer()
{
    simtime_t deadline = SIMTIME_MAX;
    if (originatorBlockAckAgreementHandler) {
        for (const auto& entry : originatorBlockAckAgreementHandler->getProcedures())
            deadline = std::min(deadline, entry.second.responseDeadline);
    }
    cancelEvent(addbaResponseTimer);
    if (deadline != SIMTIME_MAX)
        scheduleAt(std::max(simTime(), deadline), addbaResponseTimer);
}

bool Hcf::isProfileAllowed(MacAddress peer, bool originator) const
{
    auto state = mib->findPeerHtState(peer);
    bool peerHt = state && state->valid && (originator ? state->negotiatedCapabilities.localTxPeerRx.valid :
        state->negotiatedCapabilities.localRxPeerTx.valid);
    return originator ? originatorBlockAckAgreementPolicy->isProfileAllowed(blockAckVariant, mib->isHtOperationSupported(), peerHt) :
        recipientBlockAckAgreementPolicy->isProfileAllowed(blockAckVariant, mib->isHtOperationSupported(), peerHt);
}

bool Hcf::hasObsoleteActionInFlight(MacAddress peer, Tid tid, bool originator) const
{
    if (finishingSequence || !frameSequenceHandler->isSequenceRunning())
        return false;
    auto context = frameSequenceHandler->getContext();
    for (int i = 0; i < context->getNumSteps(); i++) {
        auto step = dynamic_cast<ITransmitStep *>(context->getStep(i));
        if (!step)
            continue;
        auto packet = step->getFrameToTransmit();
        if (!obsoleteActions.count(packet->getId()))
            continue;
        auto header = packet->peekAtFront<Ieee80211MgmtHeader>();
        if (header->getReceiverAddress() != peer)
            continue;
        if (auto request = dynamicPtrCast<const Ieee80211AddbaRequest>(header)) {
            if (originator && request->getTid() == tid)
                return true;
        }
        else if (auto response = dynamicPtrCast<const Ieee80211AddbaResponse>(header)) {
            if (!originator && response->getTid() == tid)
                return true;
        }
        else if (auto delba = dynamicPtrCast<const Ieee80211Delba>(header)) {
            if (delba->getInitiator() == originator && delba->getTid() == tid)
                return true;
        }
    }
    return false;
}

bool Hcf::isFrameEligible(Packet *packet, bool numbered) const
{
    if (obsoleteActions.count(packet->getId()))
        return false;
    auto data = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
    if (!data || data->getType() != ST_DATA_WITH_QOS || !originatorBlockAckAgreementHandler)
        return true;
    auto peer = data->getReceiverAddress();
    auto tid = data->getTid();
    if (hasObsoleteActionInFlight(peer, tid, true))
        return false;
    const auto& procedures = originatorBlockAckAgreementHandler->getProcedures();
    auto it = procedures.find({peer, tid});
    if (it == procedures.end())
        return true;
    if (it->second.state == IOriginatorBlockAckAgreementHandler::ProcedureState::WAITING_FOR_SUCCESSOR)
        return true;
    return numbered && it->second.state == IOriginatorBlockAckAgreementHandler::ProcedureState::SETUP_INTENT;
}

bool Hcf::isAMsduPermitted(Packet *packet) const
{
    auto data = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
    if (!data || !originatorBlockAckAgreementHandler)
        return true;
    auto agreement = originatorBlockAckAgreementHandler->getAgreement(data->getReceiverAddress(), data->getTid());
    return !agreement || !agreement->getIsAddbaResponseReceived() || agreement->isAMsduPermitted();
}

bool Hcf::hasQueuedData(MacAddress peer, Tid tid) const
{
    auto matches = [this, peer, tid](Packet *packet) {
        auto data = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
        return data && data->getType() == ST_DATA_WITH_QOS && data->getReceiverAddress() == peer && data->getTid() == tid &&
            !edca->getLifetimeHandler()->isExpired(packet);
    };
    // Check the TID's category first. Other categories retain support for custom classification.
    int first = edca->mapTidToAc(tid);
    for (int offset = 0; offset < AC_NUMCATEGORIES; offset++) {
        int ac = (first + offset) % AC_NUMCATEGORIES;
        auto queue = edca->getEdcaf(static_cast<AccessCategory>(ac))->getPendingQueue();
        if (auto selected = dynamic_cast<queueing::ISelectivePacketCollection *>(queue)) {
            try {
                if (selected->peekEligiblePacket(matches))
                    return true;
                continue;
            }
            catch (const queueing::UnsupportedPacketSelection&) {
                // Existence checks also support queues without optional selected extraction.
            }
        }
        for (int i = 0; i < queue->getNumPackets(); i++) {
            if (matches(queue->getPacket(i)))
                return true;
        }
    }
    return false;
}

bool Hcf::hasNumberedData(MacAddress peer, Tid tid) const
{
    for (int ac = 0; ac < AC_NUMCATEGORIES; ac++) {
        auto frames = edca->getEdcaf(static_cast<AccessCategory>(ac))->getInProgressFrames();
        for (int i = 0; i < frames->getLength(); i++) {
            auto data = dynamicPtrCast<const Ieee80211DataHeader>(frames->getFrames(i)->peekAtFront<Ieee80211MacHeader>());
            if (data && data->getReceiverAddress() == peer && data->getTid() == tid)
                return true;
        }
    }
    return false;
}

void Hcf::updateAgreementProcedures()
{
    if (!originatorBlockAckAgreementHandler || updatingProcedures)
        return;
    updatingProcedures = true;
    auto procedures = originatorBlockAckAgreementHandler->getProcedures();
    for (const auto& entry : procedures) {
        auto peer = entry.first.first;
        auto tid = entry.first.second;
        auto state = entry.second.state;
        using State = IOriginatorBlockAckAgreementHandler::ProcedureState;
        bool queued = hasQueuedData(peer, tid);
        // INET setup policy waits for a successor after the first eligible DATA transmission.
        if (state == State::WAITING_FOR_SUCCESSOR) {
            if (!queued)
                continue;
            originatorBlockAckAgreementHandler->beginSetupIntent(peer, tid);
            state = State::SETUP_INTENT;
        }
        if (state == State::SETUP_INTENT) {
            if (!queued)
                originatorBlockAckAgreementHandler->finishProcedure(peer, tid);
            else if (!hasNumberedData(peer, tid) && !hasObsoleteActionInFlight(peer, tid, true)) {
                auto service = check_and_cast<OriginatorQosMacDataService *>(originatorDataService);
                originatorBlockAckAgreementHandler->startSetup(peer, tid, service->getNextSequenceNumber(peer, tid),
                    originatorBlockAckAgreementPolicy, this, service->hasAMsduAggregation());
            }
        }
        else if (state == State::REQUEST_PENDING && !queued) {
            if (entry.second.requestAttempted)
                beginTeardown(peer, tid, true, 37);
            else {
                originatorBlockAckAgreementHandler->finishProcedure(peer, tid);
                retireAgreement(peer, tid, true);
            }
        }
        else if (state == State::RECOVERY_NEEDED && queued)
            beginTeardown(peer, tid, true, 37);
    }
    updatingProcedures = false;
    scheduleAddbaResponseTimer();
}

void Hcf::updateContention()
{
    if (handlingAgreementAction)
        return;
    for (int ac = 0; ac < AC_NUMCATEGORIES; ac++) {
        auto edcaf = edca->getEdcaf(static_cast<AccessCategory>(ac));
        if (edcaf->isOwning())
            continue;
        if (hasFrameToTransmit(static_cast<AccessCategory>(ac)))
            edcaf->requestChannel(this);
    }
}

void Hcf::advanceOriginatorWindows()
{
    if (!originatorBlockAckAgreementHandler)
        return;
    auto service = check_and_cast<IOriginatorQosMacDataService *>(originatorDataService);
    for (const auto& entry : originatorBlockAckAgreementHandler->getAgreements()) {
        auto agreement = entry.second;
        if (!agreement->getIsAddbaResponseReceived())
            continue;
        auto next = service->getNextSequenceNumber(entry.first.first, entry.first.second);
        std::vector<SequenceNumberCyclic> unresolvedSequences;
        for (int ac = 0; ac < AC_NUMCATEGORIES; ac++) {
            auto frames = edca->getEdcaf(static_cast<AccessCategory>(ac))->getInProgressFrames();
            for (int i = 0; i < frames->getLength(); i++) {
                auto data = dynamicPtrCast<const Ieee80211DataHeader>(frames->getFrames(i)->peekAtFront<Ieee80211MacHeader>());
                if (data && data->getType() == ST_DATA_WITH_QOS &&
                    data->getReceiverAddress() == entry.first.first && data->getTid() == entry.first.second)
                    unresolvedSequences.push_back(data->getSequenceNumber());
            }
        }
        // IEEE Std 802.11-2024, 10.25.6.7.1: resolved positions permit transmit-window progress.
        agreement->advanceWindow(next, unresolvedSequences);
    }
}

void Hcf::completeTransfers()
{
    if (!originatorBlockAckAgreementHandler)
        return;
    std::vector<std::pair<MacAddress, Tid>> completed;
    for (const auto& entry : originatorBlockAckAgreementHandler->getAgreements()) {
        auto agreement = entry.second;
        if (agreement->getIsAddbaResponseReceived() && agreement->getFirstDataTransmitted() &&
            !hasNumberedData(entry.first.first, entry.first.second) && !hasQueuedData(entry.first.first, entry.first.second))
            completed.push_back(entry.first);
    }
    // IEEE Std 802.11-2024, 10.25.4; INET uses the accepted queue predicate at an exchange boundary.
    for (const auto& id : completed)
        beginTeardown(id.first, id.second, true, 37);
}

void Hcf::discardAgreementData(MacAddress peer, Tid tid, bool all, int64_t highestTransmittedDataIndex)
{
    for (int ac = 0; ac < AC_NUMCATEGORIES; ac++) {
        auto edcaf = edca->getEdcaf(static_cast<AccessCategory>(ac));
        auto frames = edcaf->getInProgressFrames();
        std::vector<Packet *> discard;
        for (int i = 0; i < frames->getLength(); i++) {
            auto packet = frames->getFrames(i);
            auto data = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
            bool transmitted = data && edcaf->getAckHandler()->getQoSDataAckStatus(data) != QosAckHandler::Status::FRAME_NOT_YET_TRANSMITTED;
            if (data && frameSequenceHandler->isSequenceRunning()) {
                auto context = frameSequenceHandler->getContext();
                for (int j = 0; j < context->getNumSteps(); j++) {
                    if (auto step = dynamic_cast<ITransmitStep *>(context->getStep(j)))
                        transmitted |= step->getFrameToTransmit() == packet;
                }
            }
            // IEEE Std 802.11-2024, 11.5.3.4: the next MPDU must follow the highest transmitted sequence.
            // Include unsent fragments even if ACK processing already removed an earlier fragment.
            // Local allocation indices preserve order across gaps and wire sequence wraparound.
            if (data && data->getReceiverAddress() == peer && data->getTid() == tid &&
                (all || transmitted || (highestTransmittedDataIndex >= 0 &&
                    packet->getTag<Ieee80211DataSequenceTag>()->getAllocationIndex() <= highestTransmittedDataIndex)))
                discard.push_back(packet);
        }
        for (auto packet : discard) {
            auto data = packet->peekAtFront<Ieee80211DataHeader>();
            edcaf->getAckHandler()->dropFrame(data);
            edcaf->getRecoveryProcedure()->dropFrame(data);
            frames->dropFrame(packet);
            PacketDropDetails details;
            details.setReason(OTHER_PACKET_DROP);
            emit(packetDroppedSignal, packet, &details);
        }
        if (all) {
            auto queue = edcaf->getPendingQueue();
            discard.clear();
            for (int i = 0; i < queue->getNumPackets(); i++) {
                auto packet = queue->getPacket(i);
                auto data = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
                if (data && data->getReceiverAddress() == peer && data->getTid() == tid)
                    discard.push_back(packet);
            }
            for (auto packet : discard) {
                queue->removePacket(packet);
                take(packet);
                edca->getLifetimeHandler()->forgetFrame(packet->getId());
                PacketDropDetails details;
                details.setReason(OTHER_PACKET_DROP);
                emit(packetDroppedSignal, packet, &details);
                delete packet;
            }
        }
        if (!frameSequenceHandler->isSequenceRunning() || frameSequenceHandler->getContext()->getInProgressFrames() != frames)
            frames->clearDroppedFrames();
    }
}

void Hcf::firstDataDiscarded(const Ptr<const Ieee80211DataHeader>& header)
{
    if (!originatorBlockAckAgreementHandler)
        return;
    auto agreement = originatorBlockAckAgreementHandler->getAgreement(header->getReceiverAddress(), header->getTid());
    if (agreement && agreement->getIsAddbaResponseReceived() && !agreement->getFirstDataTransmitted() &&
        agreement->getStartingSequenceNumber() == header->getSequenceNumber())
        beginTeardown(header->getReceiverAddress(), header->getTid(), true, 37);
}

void Hcf::invalidateActions(MacAddress peer, Tid tid, bool originator)
{
    std::set<int64_t> invalid;
    std::set<int64_t> attempted;
    std::set<int64_t> rtsRecoveryOwned;
    for (auto it = ownedActions.begin(); it != ownedActions.end();) {
        if (it->second.peer == peer && it->second.tid == tid && it->second.originator == originator) {
            invalid.insert(it->first);
            if (it->second.attempted)
                attempted.insert(it->first);
            if (it->second.rtsRecoveryOwned)
                rtsRecoveryOwned.insert(it->first);
            obsoleteActions.insert(it->first);
            it = ownedActions.erase(it);
        }
        else
            ++it;
    }
    for (int ac = 0; ac < AC_NUMCATEGORIES; ac++) {
        auto edcaf = edca->getEdcaf(static_cast<AccessCategory>(ac));
        auto queue = edcaf->getPendingQueue();
        std::vector<Packet *> removed;
        for (int i = 0; i < queue->getNumPackets(); i++) {
            if (invalid.count(queue->getPacket(i)->getId()))
                removed.push_back(queue->getPacket(i));
        }
        for (auto packet : removed) {
            queue->removePacket(packet);
            take(packet);
            obsoleteActions.erase(packet->getId());
            delete packet;
        }
        auto frames = edcaf->getInProgressFrames();
        removed.clear();
        for (int i = 0; i < frames->getLength(); i++) {
            if (invalid.count(frames->getFrames(i)->getId()))
                removed.push_back(frames->getFrames(i));
        }
        for (auto packet : removed) {
            auto header = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
            edcaf->getAckHandler()->dropFrame(header);
            if (attempted.count(packet->getId()))
                edcaf->getMgmtAndNonQoSRecoveryProcedure()->dropFrame(packet, header);
            if (rtsRecoveryOwned.count(packet->getId()))
                edcaf->getMgmtAndNonQoSRecoveryProcedure()->dropRtsProtectedFrame(header);
            frames->dropFrame(packet);
            if (!frameSequenceHandler->isSequenceRunning() || frameSequenceHandler->getContext()->getInProgressFrames() != frames)
                obsoleteActions.erase(packet->getId());
        }
        if (!frameSequenceHandler->isSequenceRunning() || frameSequenceHandler->getContext()->getInProgressFrames() != frames)
            frames->clearDroppedFrames();
    }
}

void Hcf::retireAgreement(MacAddress peer, Tid tid, bool originator, bool associationReset)
{
    if (originator) {
        auto agreement = originatorBlockAckAgreementHandler->detachAgreement(peer, tid);
        invalidateActions(peer, tid, originator);
        discardAgreementData(peer, tid, associationReset, agreement ? agreement->getHighestTransmittedDataIndex() : -1);
        if (agreement) {
            if (agreement->getIsAddbaResponseReceived())
                emit(blockAckAgreementDeletedSignal, agreement);
            delete agreement;
        }
    }
    else {
        auto agreement = recipientBlockAckAgreementHandler->detachAgreement(peer, tid);
        invalidateActions(peer, tid, originator);
        if (agreement) {
            sendUp(recipientDataService->retireAgreement(agreement, associationReset));
            emit(blockAckAgreementDeletedSignal, agreement);
            delete agreement;
        }
    }
    scheduleInactivityTimer();
}

void Hcf::beginTeardown(MacAddress peer, Tid tid, bool originator, int reasonCode)
{
    if (!originatorBlockAckAgreementHandler || !recipientBlockAckAgreementHandler)
        return;
    if (originator) {
        const auto& procedures = originatorBlockAckAgreementHandler->getProcedures();
        auto it = procedures.find({peer, tid});
        if (it != procedures.end() && it->second.state == IOriginatorBlockAckAgreementHandler::ProcedureState::TEARDOWN_PENDING)
            return;
        originatorBlockAckAgreementHandler->beginTeardown(peer, tid);
    }
    else {
        if (recipientBlockAckAgreementHandler->isTeardownPending(peer, tid))
            return;
        recipientBlockAckAgreementHandler->beginTeardown(peer, tid);
    }
    retireAgreement(peer, tid, originator);
    auto delba = makeShared<Ieee80211Delba>();
    delba->setReceiverAddress(peer);
    delba->setTid(tid);
    delba->setInitiator(originator);
    delba->setReasonCode(reasonCode);
    processMgmtFrame(new Packet("Delba", delba), delba);
    scheduleAddbaResponseTimer();
}

void Hcf::actionTerminated(Packet *packet, bool acknowledged)
{
    auto it = ownedActions.find(packet->getId());
    if (it == ownedActions.end())
        return;
    auto action = it->second;
    ownedActions.erase(it);
    if (action.rtsRecoveryOwned)
        edca->getEdcaf(AC_VO)->getMgmtAndNonQoSRecoveryProcedure()->dropRtsProtectedFrame(packet->peekAtFront<Ieee80211DataOrMgmtHeader>());
    if (action.kind == OwnedAction::REQUEST) {
        auto agreement = originatorBlockAckAgreementHandler->getAgreement(action.peer, action.tid);
        if (!agreement || agreement->getIsAddbaResponseReceived())
            return;
        if (acknowledged)
            originatorBlockAckAgreementHandler->acknowledgeRequest(action.peer, action.tid,
                originatorBlockAckAgreementPolicy->computeAddbaFailureTimeout());
        else if (action.attempted)
            // INET recovery policy confirms peer teardown before another setup.
            beginTeardown(action.peer, action.tid, true, 37);
        else {
            originatorBlockAckAgreementHandler->finishProcedure(action.peer, action.tid);
            retireAgreement(action.peer, action.tid, true);
        }
    }
    else if (action.kind == OwnedAction::DELBA) {
        if (action.originator) {
            if (acknowledged)
                originatorBlockAckAgreementHandler->finishProcedure(action.peer, action.tid);
            else {
                // INET recovery policy discards this batch after failed DELBA delivery.
                originatorBlockAckAgreementHandler->failTeardown(action.peer, action.tid);
                discardAgreementData(action.peer, action.tid, true);
            }
        }
        else
            recipientBlockAckAgreementHandler->finishTeardown(action.peer, action.tid);
    }
    scheduleAddbaResponseTimer();
    if (action.kind == OwnedAction::DELBA || (action.kind == OwnedAction::REQUEST && !acknowledged)) {
        updateAgreementProcedures();
        updateContention();
    }
}

void Hcf::resetPeer(MacAddress peer)
{
    Enter_Method("resetPeer");
    if (!originatorBlockAckAgreementHandler)
        return;
    bool previous = handlingAgreementAction;
    handlingAgreementAction = true;
    auto originators = originatorBlockAckAgreementHandler->detachPeer(peer);
    auto recipients = recipientBlockAckAgreementHandler->detachPeer(peer);
    for (int tid = 0; tid < 16; tid++) {
        invalidateActions(peer, tid, true);
        invalidateActions(peer, tid, false);
        discardAgreementData(peer, tid, true);
    }
    for (auto agreement : recipients)
        sendUp(recipientDataService->retireAgreement(agreement, true));
    recipientDataService->clearPeer(peer);
    for (auto agreement : originators) {
        if (agreement->getIsAddbaResponseReceived())
            emit(blockAckAgreementDeletedSignal, agreement);
        delete agreement;
    }
    for (auto agreement : recipients) {
        emit(blockAckAgreementDeletedSignal, agreement);
        delete agreement;
    }
    handlingAgreementAction = previous;
    scheduleInactivityTimer();
    scheduleAddbaResponseTimer();
}

void Hcf::receiveSignal(cComponent *source, simsignal_t signalID, bool value, cObject *details)
{
    if (signalID != Ieee80211Mib::rateStateChangedSignal)
        return;
    Enter_Method("rateStateChanged");
    // This cleanup prepares Compressed support. Basic agreements do not depend on HT admission state.
    if (blockAckVariant == BlockAckVariant::BASIC || !originatorBlockAckAgreementHandler)
        return;
    std::vector<std::tuple<MacAddress, Tid, bool>> retired;
    for (const auto& entry : originatorBlockAckAgreementHandler->getAgreements()) {
        if (!isProfileAllowed(entry.first.first, true))
            retired.push_back({entry.first.first, entry.first.second, true});
    }
    for (const auto& entry : recipientBlockAckAgreementHandler->getAgreements()) {
        if (!isProfileAllowed(entry.first.first, false))
            retired.push_back({entry.first.first, entry.first.second, false});
    }
    for (const auto& entry : retired)
        beginTeardown(std::get<0>(entry), std::get<1>(entry), std::get<2>(entry), 37);
    updateContention();
}

void Hcf::processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method("processLowerFrame(%s)", packet->getName());
    take(packet);
    EV_INFO << "Processing lower frame: " << packet->getName() << endl;
    auto edcaf = edca->getChannelOwner();
    auto action = dynamicPtrCast<const Ieee80211ActionFrame>(header);
    bool blockAckAction = dynamicPtrCast<const Ieee80211AddbaRequest>(header) ||
        dynamicPtrCast<const Ieee80211AddbaResponse>(header) || dynamicPtrCast<const Ieee80211Delba>(header);
    if (action && blockAckAction && isForUs(header) && edcaf && frameSequenceHandler->isSequenceRunning() &&
        dynamic_cast<IReceiveStep *>(frameSequenceHandler->getContext()->getLastStep())) {
        handlingAgreementAction = true;
        emit(packetReceivedFromPeerSignal, packet);
        std::unique_ptr<Packet> responseSource(packet->dup());
        auto accepted = recipientDataService->managementFrameReceived(packet, action);
        for (auto frame : accepted) {
            take(frame);
            dispatchAgreementAction(frame->peekAtFront<Ieee80211MgmtHeader>());
        }
        frameSequenceHandler->interruptFrameSequence();
        recipientAckProcedure->processReceivedFrame(responseSource.get(), action, check_and_cast<IRecipientAckPolicy *>(recipientAckPolicy), this);
        for (auto frame : accepted)
            delete frame;
        handlingAgreementAction = false;
        updateAgreementProcedures();
        updateContention();
        return;
    }
    if (edcaf && frameSequenceHandler->isSequenceRunning()) {
        // TODO always call processResponse?
        if ((!isForUs(header) && !startRxTimer->isScheduled()) || isForUs(header)) {
            frameSequenceHandler->processResponse(packet);
            // Only cancel RxTimer when the current running sequence has been handled by frameSequenceHandler->processResponse().
            // If the received frame is not for us, we are still waiting to receive our ACK. In that case, don't cancel the timer.
            // Otherwise, current frame sequence stucks in RX step and runs longer than intendeed, preventing sequence from
            // another access category (AC) to start running (RuntimeError("Channel access granted while a frame sequence is running")).
            cancelEvent(startRxTimer);
        }
        else {
            EV_INFO << "This frame is not for us" << std::endl;
            PacketDropDetails details;
            details.setReason(NOT_ADDRESSED_TO_US);
            emit(packetDroppedSignal, packet, &details);
            delete packet;
        }
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
    else if (isForUs(header))
        recipientProcessReceivedFrame(packet, header);
    else {
        EV_INFO << "This frame is not for us" << std::endl;
        PacketDropDetails details;
        details.setReason(NOT_ADDRESSED_TO_US);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

void Hcf::channelGranted(IChannelAccess *channelAccess)
{
    Enter_Method("channelGranted");
    auto edcaf = check_and_cast<Edcaf *>(channelAccess);
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        EV_DETAIL << "Channel access granted to the " << printAccessCategory(ac) << " queue" << std::endl;
        edcaf->getTxopProcedure()->startTxop(ac);
        auto internallyCollidedEdcafs = edca->getInternallyCollidedEdcafs();
        if (internallyCollidedEdcafs.size() > 0) {
            EV_INFO << "Internal collision happened with the following queues:" << std::endl;
            handleInternalCollision(internallyCollidedEdcafs);
            emit(edcaCollisionDetectedSignal, (unsigned long)internallyCollidedEdcafs.size());
        }
        // DATA can expire while the EDCAF waits for its previously requested grant.
        if (!hasFrameToTransmit(ac)) {
            edcaf->releaseChannel(this);
            edcaf->getTxopProcedure()->endTxop();
            mac->sendDownPendingRadioConfigMsg();
            return;
        }
        // Preserve same-time collision accounting before the shared contention gate changes.
        rx->setContentionBlocked(true);
        startFrameSequence(ac);
    }
    else
        throw cRuntimeError("Channel access granted but channel owner not found!");
}

FrameSequenceContext *Hcf::buildContext(AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    auto qosContext = new QoSContext(originatorAckPolicy, originatorBlockAckProcedure, originatorBlockAckAgreementHandler, edcaf->getTxopProcedure());
    return new FrameSequenceContext(mac->getAddress(), modeSet, edcaf->getInProgressFrames(), rtsProcedure, rtsPolicy, nullptr, qosContext);
}

void Hcf::startFrameSequence(AccessCategory ac)
{
    frameSequenceHandler->startFrameSequence(new HcfFs(), buildContext(ac), this);
}

void Hcf::frameSequenceStarted()
{
    Enter_Method("frameSequenceStarted");
    emit(IFrameSequenceHandler::frameSequenceStartedSignal, frameSequenceHandler->getContext());
}

void Hcf::handleInternalCollision(std::vector<Edcaf *> internallyCollidedEdcafs)
{
    for (auto edcaf : internallyCollidedEdcafs) {
        AccessCategory ac = edcaf->getAccessCategory();
        auto dataRecoveryProcedure = edcaf->getRecoveryProcedure();
        if (originatorAckPolicy->isBlockAckReqNeeded(edcaf->getInProgressFrames(), edcaf->getTxopProcedure(), originatorBlockAckAgreementHandler)) {
            dataRecoveryProcedure->controlFrameTransmissionFailed();
            edcaf->requestChannel(this);
            continue;
        }
        Packet *internallyCollidedFrame = edcaf->getInProgressFrames()->getFrameToTransmit();
        if (internallyCollidedFrame == nullptr)
            continue;
        auto internallyCollidedHeader = internallyCollidedFrame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        EV_INFO << printAccessCategory(ac) << " (" << internallyCollidedFrame->getName() << ")" << endl;
        bool retryLimitReached = false;
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(internallyCollidedHeader)) { // TODO QoSDataFrame
            dataRecoveryProcedure->dataFrameTransmissionFailed(internallyCollidedFrame, dataHeader);
            retryLimitReached = dataRecoveryProcedure->isRetryLimitReached(internallyCollidedFrame, dataHeader);
        }
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(internallyCollidedHeader)) {
            edcaf->getMgmtAndNonQoSRecoveryProcedure()->dataOrMgmtFrameTransmissionFailed(internallyCollidedFrame, mgmtHeader, edcaf->getStationRetryCounters());
            retryLimitReached = edcaf->getMgmtAndNonQoSRecoveryProcedure()->isRetryLimitReached(internallyCollidedFrame, mgmtHeader);
        }
        else // TODO + NonQoSDataFrame
            throw cRuntimeError("Unknown frame");
        if (retryLimitReached) {
            EV_DETAIL << "The frame has reached its retry limit. Dropping it" << std::endl;
            if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(internallyCollidedHeader))
                dataRecoveryProcedure->retryLimitReached(internallyCollidedFrame, dataHeader);
            else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(internallyCollidedHeader))
                edcaf->getMgmtAndNonQoSRecoveryProcedure()->retryLimitReached(internallyCollidedFrame, mgmtHeader);
            else ; // TODO + NonQoSDataFrame
            edcaf->getInProgressFrames()->dropFrame(internallyCollidedFrame);
            edcaf->getAckHandler()->dropFrame(internallyCollidedHeader);
            PacketDropDetails details;
            details.setReason(RETRY_LIMIT_REACHED);
            details.setLimit(-1); // TODO
            emit(packetDroppedSignal, internallyCollidedFrame, &details);
            emit(linkBrokenSignal, internallyCollidedFrame);
            if (dynamicPtrCast<const Ieee80211MgmtHeader>(internallyCollidedHeader)) {
                FrameTransmissionDetails transmissionDetails;
                transmissionDetails.setStatus(FRAME_TRANSMISSION_STATUS_RETRY_LIMIT_REACHED);
                emit(Ieee80211Mac::frameTransmissionOutcomeSignal, internallyCollidedFrame, &transmissionDetails);
            }
            else if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(internallyCollidedHeader))
                firstDataDiscarded(dataHeader);
            if (hasFrameToTransmit(ac))
                edcaf->requestChannel(this);
        }
        else
            edcaf->requestChannel(this);
    }
}

/*
 * TODO  If a PHY-RXSTART.indication primitive does not occur during the ACKTimeout interval,
 * the STA concludes that the transmission of the MPDU has failed, and this STA shall invoke its
 * backoff procedure **upon expiration of the ACKTimeout interval**.
 */

void Hcf::frameSequenceFinished()
{
    Enter_Method("frameSequenceFinished");
    cancelEvent(startRxTimer);
    finishingSequence = true;
    emit(IFrameSequenceHandler::frameSequenceFinishedSignal, frameSequenceHandler->getContext());
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        // The context still owns references. dropFrame defers deletion until context disposal.
        expireDataFrames(false);
        advanceOriginatorWindows();
        completeTransfers();
        updateAgreementProcedures();
        bool startContention = !handlingAgreementAction && hasFrameToTransmit();
        edcaf->releaseChannel(this);
        mac->sendDownPendingRadioConfigMsg(); // TODO review
        edcaf->getTxopProcedure()->endTxop();
        rx->setContentionBlocked(false);
        if (startContention)
            edcaf->requestChannel(this);
    }
    else if (hcca->isOwning()) {
        hcca->releaseChannel(this);
        mac->sendDownPendingRadioConfigMsg(); // TODO review
        throw cRuntimeError("Hcca is unimplemented!");
    }
    else
        throw cRuntimeError("Frame sequence finished but channel owner not found!");
    obsoleteActions.clear();
    finishingSequence = false;
}

void Hcf::recipientProcessReceivedFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    EV_INFO << "Processing received frame " << packet->getName() << " as recipient.\n";
    emit(packetReceivedFromPeerSignal, packet);
    if (recipientBlockAckAgreementHandler) {
        // IEEE Std 802.11-2024, 11.5.4: valid Block Ack traffic without an agreement requires DELBA.
        // Reject malformed DATA and unsupported BAR fields before this recovery path.
        if (auto data = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
            if (data->getType() == ST_DATA_WITH_QOS && data->getAckPolicy() == BLOCK_ACK &&
                (data->getTid() < 0 || data->getTid() > 15 || !data->getSequenceNumber().isSpecified() ||
                    data->getTransmitterAddress().isUnspecified() || data->getTransmitterAddress().isMulticast())) {
                delete packet;
                return;
            }
            if (data->getType() == ST_DATA_WITH_QOS && data->getAckPolicy() == BLOCK_ACK &&
                !recipientBlockAckAgreementHandler->getAgreement(data->getTid(), data->getTransmitterAddress())) {
                beginTeardown(data->getTransmitterAddress(), data->getTid(), false, 38);
                PacketDropDetails details;
                details.setReason(OTHER_PACKET_DROP);
                emit(packetDroppedSignal, packet, &details);
                delete packet;
                return;
            }
        }
        else if (auto bar = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header)) {
            if (bar->getFragmentNumber() != 0 || bar->getMultiTid() || bar->getCompressedBitmap() ||
                bar->getTidInfo() < 0 || bar->getTidInfo() > 15 || !bar->getStartingSequenceNumber().isSpecified() ||
                bar->getTransmitterAddress().isUnspecified() || bar->getTransmitterAddress().isMulticast()) {
                delete packet;
                return;
            }
            auto agreement = recipientBlockAckAgreementHandler->getAgreement(bar->getTidInfo(), bar->getTransmitterAddress());
            if (!agreement) {
                beginTeardown(bar->getTransmitterAddress(), bar->getTidInfo(), false, 38);
                delete packet;
                return;
            }
            agreement->calculateExpirationTime();
            scheduleInactivityTimer();
        }
    }
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        recipientAckProcedure->processReceivedFrame(packet, dataOrMgmtHeader, check_and_cast<IRecipientAckPolicy *>(recipientAckPolicy), this);
    if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
        if (dataHeader->getType() == ST_DATA_WITH_QOS && recipientBlockAckAgreementHandler)
            recipientBlockAckAgreementHandler->qosFrameReceived(dataHeader, this);
        sendUp(recipientDataService->dataFrameReceived(packet, dataHeader, recipientBlockAckAgreementHandler));
    }
    else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(header)) {
        auto accepted = recipientDataService->managementFrameReceived(packet, mgmtHeader);
        for (auto frame : accepted) {
            take(frame);
            auto completeHeader = frame->peekAtFront<Ieee80211MgmtHeader>();
            if (dynamicPtrCast<const Ieee80211ActionFrame>(completeHeader)) {
                recipientProcessReceivedManagementFrame(completeHeader);
                delete frame;
            }
            else
                sendUp({frame});
        }
        updateAgreementProcedures();
        updateContention();
    }
    else { // TODO else if (auto ctrlFrame = dynamic_cast<Ieee80211ControlFrame*>(frame))
        sendUp(recipientDataService->controlFrameReceived(packet, header, recipientBlockAckAgreementHandler));
        recipientProcessReceivedControlFrame(packet, header);
        delete packet;
    }
}

void Hcf::recipientProcessReceivedControlFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(header))
        ctsProcedure->processReceivedRts(packet, rtsFrame, ctsPolicy, this);
    else if (auto blockAckRequest = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header)) {
        if (recipientBlockAckProcedure)
            recipientBlockAckProcedure->processReceivedBlockAckReq(packet, blockAckRequest, recipientAckPolicy, recipientBlockAckAgreementHandler, this);
    }
    else if (dynamicPtrCast<const Ieee80211AckFrame>(header))
        EV_WARN << "ACK frame received after timeout, ignoring it.\n"; // drop it, it is an ACK frame that is received after the ACKTimeout
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::recipientProcessReceivedManagementFrame(const Ptr<const Ieee80211MgmtHeader>& header)
{
    dispatchAgreementAction(header);
}

bool Hcf::dispatchAgreementAction(const Ptr<const Ieee80211MgmtHeader>& header)
{
    if (recipientBlockAckAgreementHandler && originatorBlockAckAgreementHandler) {
        if (auto addbaRequest = dynamicPtrCast<const Ieee80211AddbaRequest>(header)) {
            auto peer = addbaRequest->getTransmitterAddress();
            auto tid = addbaRequest->getTid();
            auto previous = recipientBlockAckAgreementHandler->getAgreement(tid, peer);
            if (!isProfileAllowed(peer, false) || hasObsoleteActionInFlight(peer, tid, false)) {
                auto response = makeShared<Ieee80211AddbaResponse>();
                response->setReceiverAddress(peer);
                response->setTid(tid);
                response->setDialogToken(addbaRequest->getDialogToken());
                response->setStatusCode(SC_UNSPECIFIED);
                processMgmtFrame(new Packet("AddbaResponse", response), response);
                return false;
            }
            recipientBlockAckAgreementHandler->processReceivedAddbaRequest(addbaRequest, recipientBlockAckAgreementPolicy, this);
            auto agreement = recipientBlockAckAgreementHandler->getAgreement(addbaRequest->getTid(), addbaRequest->getTransmitterAddress());
            if (!previous && agreement)
                emit(blockAckAgreementAddedSignal, agreement);
            scheduleInactivityTimer();
        }
        else if (auto addbaResp = dynamicPtrCast<const Ieee80211AddbaResponse>(header)) {
            auto peer = addbaResp->getTransmitterAddress();
            auto tid = addbaResp->getTid();
            auto pending = originatorBlockAckAgreementHandler->getAgreement(peer, tid);
            if (!pending || pending->getIsAddbaResponseReceived() || pending->getDialogToken() != addbaResp->getDialogToken())
                return false;
            if (addbaResp->getStatusCode() != SC_SUCCESSFUL) {
                originatorBlockAckAgreementHandler->finishProcedure(peer, tid);
                retireAgreement(peer, tid, true);
                scheduleAddbaResponseTimer();
                return true;
            }
            if (!isProfileAllowed(peer, true) || !originatorBlockAckAgreementPolicy->isAddbaReqAccepted(addbaResp, pending))
                return false;
            originatorBlockAckAgreementHandler->processReceivedAddbaResp(addbaResp, originatorBlockAckAgreementPolicy, this);
            auto agreement = originatorBlockAckAgreementHandler->getAgreement(addbaResp->getTransmitterAddress(), addbaResp->getTid());
            originatorBlockAckAgreementHandler->finishProcedure(peer, tid);
            invalidateActions(peer, tid, true);
            emit(blockAckAgreementAddedSignal, agreement);
            scheduleAddbaResponseTimer();
            if (!hasQueuedData(peer, tid) && !hasNumberedData(peer, tid))
                beginTeardown(peer, tid, true, 37);
            return true;
        }
        else if (auto delba = dynamicPtrCast<const Ieee80211Delba>(header)) {
            auto peer = delba->getTransmitterAddress();
            auto tid = delba->getTid();
            bool originator = !delba->getInitiator();
            if (tid < 0 || tid > 15 || peer.isMulticast() || peer.isUnspecified())
                return false;
            if (originator)
                originatorBlockAckAgreementHandler->finishProcedure(peer, tid);
            else
                recipientBlockAckAgreementHandler->finishTeardown(peer, tid);
            retireAgreement(peer, tid, originator);
            scheduleAddbaResponseTimer();
            return true;
        }
        else
            ; // Beacon, etc
    }
    else
        ; // Optional modules
    return false;
}

void Hcf::transmissionComplete(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method("transmissionComplete");
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        auto step = check_and_cast<TransmitStep *>(frameSequenceHandler->getContext()->getLastStep());
        step->setPpduEnd(simTime());
        edcaf->getTxopProcedure()->recordTransmittedDuration(header->getDurationField());
        frameSequenceHandler->transmissionComplete();
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented!");
    else {
        recipientProcessTransmittedControlResponseFrame(packet, header);
        mac->sendDownPendingRadioConfigMsg();
    }
}

void Hcf::originatorProcessRtsProtectionFailed(Packet *packet)
{
    Enter_Method("originatorProcessRtsProtectionFailed");
    auto protectedHeader = packet->peekAtFront<Ieee80211DataOrMgmtHeader>();
    auto edcaf = edca->getChannelOwner();
    if (obsoleteActions.count(packet->getId()) || !edcaf->getInProgressFrames()->containsFrame(packet))
        return;
    if (edcaf) {
        EV_INFO << "RTS frame transmission failed\n";
        bool retryLimitReached = false;
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(protectedHeader)) {
            edcaf->getRecoveryProcedure()->rtsFrameTransmissionFailed(dataHeader);
            retryLimitReached = edcaf->getRecoveryProcedure()->isRtsFrameRetryLimitReached(packet, dataHeader);
        }
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(protectedHeader)) {
            edcaf->getMgmtAndNonQoSRecoveryProcedure()->rtsFrameTransmissionFailed(mgmtHeader, edcaf->getStationRetryCounters());
            auto action = ownedActions.find(packet->getId());
            if (action != ownedActions.end())
                action->second.rtsRecoveryOwned = true;
            retryLimitReached = edcaf->getMgmtAndNonQoSRecoveryProcedure()->isRtsFrameRetryLimitReached(packet, mgmtHeader);
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO QoSDataFrame, NonQoSDataFrame
        if (retryLimitReached) {
            if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(protectedHeader))
                edcaf->getRecoveryProcedure()->retryLimitReached(packet, dataHeader);
            else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(protectedHeader))
                edcaf->getMgmtAndNonQoSRecoveryProcedure()->retryLimitReached(packet, mgmtHeader);
            else ; // TODO nonqos data
            edcaf->getInProgressFrames()->dropFrame(packet);
            edcaf->getAckHandler()->dropFrame(protectedHeader);
            EV_INFO << "Dropping RTS/CTS protected frame " << packet->getName() << ", because retry limit is reached.\n";
            PacketDropDetails details;
            details.setReason(RETRY_LIMIT_REACHED);
            details.setLimit(-1); // TODO
            emit(packetDroppedSignal, packet, &details);
            emit(linkBrokenSignal, packet);
            if (auto data = dynamicPtrCast<const Ieee80211DataHeader>(protectedHeader))
                firstDataDiscarded(data);
            if (dynamicPtrCast<const Ieee80211MgmtHeader>(protectedHeader)) {
                actionTerminated(packet, false);
                FrameTransmissionDetails transmissionDetails;
                transmissionDetails.setStatus(FRAME_TRANSMISSION_STATUS_RETRY_LIMIT_REACHED);
                emit(Ieee80211Mac::frameTransmissionOutcomeSignal, packet, &transmissionDetails);
            }
        }
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessTransmittedFrame(Packet *packet)
{
    Enter_Method("originatorProcessTransmittedFrame");
    auto edcaf = edca->getChannelOwner();
    if (edcaf)
        edcaf->emit(packetSentToPeerSignal, packet);
    if (obsoleteActions.count(packet->getId()) ||
        (dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(packet->peekAtFront<Ieee80211MacHeader>()) &&
            !edca->getChannelOwner()->getInProgressFrames()->containsFrame(packet)))
        return;
    EV_INFO << "Processing transmitted frame " << packet->getName() << " as originator in frame sequence.\n";
    auto transmittedHeader = packet->peekAtFront<Ieee80211MacHeader>();
    if (auto dataOrMgmt = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(transmittedHeader)) {
        if (!recipientAckPolicy->isAckNeeded(dataOrMgmt)) {
            auto step = check_and_cast<TransmitStep *>(frameSequenceHandler->getContext()->getLastStep());
            rx->successfulFrameTransmitted(step->getPpduEnd(), transmittedHeader->getDurationField());
        }
    }
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        if (transmittedHeader->getReceiverAddress().isMulticast()) {
            if (dynamicPtrCast<const Ieee80211MgmtHeader>(transmittedHeader))
                edcaf->getMgmtAndNonQoSRecoveryProcedure()->multicastFrameTransmitted(edcaf->getStationRetryCounters());
            else
                edcaf->getRecoveryProcedure()->multicastFrameTransmitted();
            if (auto transmittedDataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(transmittedHeader))
                edcaf->getInProgressFrames()->dropFrame(packet);
        }
        else if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(transmittedHeader))
            originatorProcessTransmittedDataFrame(packet, dataHeader, ac);
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(transmittedHeader))
            originatorProcessTransmittedManagementFrame(mgmtHeader, ac);
        else // TODO Ieee80211ControlFrame
            originatorProcessTransmittedControlFrame(transmittedHeader, ac);
    }
    else if (hcca->isOwning())
        throw cRuntimeError("Hcca is unimplemented");
    else
        throw cRuntimeError("Frame transmitted but channel owner not found");
}

void Hcf::originatorProcessTransmittedDataFrame(Packet *packet, const Ptr<const Ieee80211DataHeader>& dataHeader, AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    edcaf->getAckHandler()->processTransmittedDataOrMgmtFrame(dataHeader);
    if (originatorBlockAckAgreementHandler)
        originatorBlockAckAgreementHandler->processTransmittedDataFrame(packet, dataHeader, originatorBlockAckAgreementPolicy, this);
    if (originatorBlockAckAgreementHandler && dataHeader->getType() == ST_DATA_WITH_QOS &&
        !dataHeader->getReceiverAddress().isMulticast() && isProfileAllowed(dataHeader->getReceiverAddress(), true) &&
        originatorBlockAckAgreementPolicy->isAddbaReqNeeded(packet, dataHeader) &&
        !originatorBlockAckAgreementHandler->getAgreement(dataHeader->getReceiverAddress(), dataHeader->getTid())) {
        originatorBlockAckAgreementHandler->noteEligibleDataTransmission(dataHeader->getReceiverAddress(), dataHeader->getTid());
        if (hasQueuedData(dataHeader->getReceiverAddress(), dataHeader->getTid()))
            originatorBlockAckAgreementHandler->beginSetupIntent(dataHeader->getReceiverAddress(), dataHeader->getTid());
    }
    if (dataHeader->getAckPolicy() == NO_ACK)
        edcaf->getInProgressFrames()->dropFrame(packet);
}

void Hcf::originatorProcessTransmittedManagementFrame(const Ptr<const Ieee80211MgmtHeader>& mgmtHeader, AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    if (originatorAckPolicy->isAckNeeded(mgmtHeader))
        edcaf->getAckHandler()->processTransmittedDataOrMgmtFrame(mgmtHeader);
    if (auto addbaReq = dynamicPtrCast<const Ieee80211AddbaRequest>(mgmtHeader)) {
        if (originatorBlockAckAgreementHandler)
            originatorBlockAckAgreementHandler->processTransmittedAddbaReq(addbaReq);
    }
    else if (auto addbaResp = dynamicPtrCast<const Ieee80211AddbaResponse>(mgmtHeader))
        recipientBlockAckAgreementHandler->processTransmittedAddbaResp(addbaResp, this);
    else if (auto delba = dynamicPtrCast<const Ieee80211Delba>(mgmtHeader)) {
        if (delba->getInitiator())
            originatorBlockAckAgreementHandler->processTransmittedDelba(delba);
        else
            recipientBlockAckAgreementHandler->processTransmittedDelba(delba);
    }
    else ; // TODO other mgmt frames if needed
}

void Hcf::originatorProcessTransmittedControlFrame(const Ptr<const Ieee80211MacHeader>& controlHeader, AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    if (auto blockAckReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(controlHeader)) {
        auto agreement = originatorBlockAckAgreementHandler ?
            originatorBlockAckAgreementHandler->getAgreement(blockAckReq->getReceiverAddress(), blockAckReq->getTidInfo()) : nullptr;
        if (agreement)
            edcaf->getAckHandler()->processTransmittedBlockAckReq(blockAckReq, agreement);
    }
    else if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(controlHeader))
        rtsProcedure->processTransmittedRts(rtsFrame);
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::originatorProcessFailedFrame(Packet *failedPacket)
{
    Enter_Method("originatorProcessFailedFrame");
    auto failedHeader = failedPacket->peekAtFront<Ieee80211MacHeader>();
    auto edcaf = edca->getChannelOwner();
    if (obsoleteActions.count(failedPacket->getId()) ||
        (dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(failedHeader) && !edcaf->getInProgressFrames()->containsFrame(failedPacket)))
        return;
    if (edcaf) {
        bool retryLimitReached = false;
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(failedHeader)) {
            ASSERT(dataHeader->getAckPolicy() == NORMAL_ACK);
            EV_INFO << "Data frame transmission failed\n";
            edcaf->getRecoveryProcedure()->dataFrameTransmissionFailed(failedPacket, dataHeader);
            retryLimitReached = edcaf->getRecoveryProcedure()->isRetryLimitReached(failedPacket, dataHeader);
            if (dataAndMgmtRateControl) {
                int retryCount = edcaf->getRecoveryProcedure()->getRetryCount(failedPacket, dataHeader);
                dataAndMgmtRateControl->frameTransmitted(failedPacket, retryCount, false, retryLimitReached);
            }
            edcaf->getAckHandler()->processFailedFrame(dataHeader);
        }
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(failedHeader)) { // TODO + NonQoS frames
            EV_INFO << "Management frame transmission failed\n";
            edcaf->getMgmtAndNonQoSRecoveryProcedure()->dataOrMgmtFrameTransmissionFailed(failedPacket, mgmtHeader, edcaf->getStationRetryCounters());
            retryLimitReached = edcaf->getMgmtAndNonQoSRecoveryProcedure()->isRetryLimitReached(failedPacket, mgmtHeader);
            if (dataAndMgmtRateControl) {
                int retryCount = edcaf->getMgmtAndNonQoSRecoveryProcedure()->getRetryCount(failedPacket, mgmtHeader);
                dataAndMgmtRateControl->frameTransmitted(failedPacket, retryCount, false, retryLimitReached);
            }
            edcaf->getAckHandler()->processFailedFrame(mgmtHeader);
        }
        else if (auto blockAckReq = dynamicPtrCast<const Ieee80211BlockAckReq>(failedHeader)) {
            edcaf->getAckHandler()->processFailedBlockAckReq(blockAckReq);
            edcaf->getRecoveryProcedure()->controlFrameTransmissionFailed();
            return;
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO qos, nonqos
        if (retryLimitReached) {
            if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(failedHeader))
                edcaf->getRecoveryProcedure()->retryLimitReached(failedPacket, dataHeader);
            else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(failedHeader))
                edcaf->getMgmtAndNonQoSRecoveryProcedure()->retryLimitReached(failedPacket, mgmtHeader);
            edcaf->getInProgressFrames()->dropFrame(failedPacket);
            edcaf->getAckHandler()->dropFrame(dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(failedHeader));
            EV_INFO << "Dropping frame " << failedPacket->getName() << ", because retry limit is reached.\n";
            PacketDropDetails details;
            details.setReason(RETRY_LIMIT_REACHED);
            details.setLimit(-1); // TODO
            emit(packetDroppedSignal, failedPacket, &details);
            emit(linkBrokenSignal, failedPacket);
            if (dynamicPtrCast<const Ieee80211MgmtHeader>(failedHeader)) {
                actionTerminated(failedPacket, false);
                FrameTransmissionDetails transmissionDetails;
                transmissionDetails.setStatus(FRAME_TRANSMISSION_STATUS_RETRY_LIMIT_REACHED);
                emit(Ieee80211Mac::frameTransmissionOutcomeSignal, failedPacket, &transmissionDetails);
            }
        }
        else {
            EV_INFO << "Retrying frame " << failedPacket->getName() << ".\n";
            auto h = failedPacket->removeAtFront<Ieee80211DataOrMgmtHeader>();
            h->setRetry(true);
            failedPacket->insertAtFront(h);
        }
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessReceivedFrame(Packet *receivedPacket, Packet *lastTransmittedPacket)
{
    Enter_Method("originatorProcessReceivedFrame");
    emit(packetReceivedFromPeerSignal, receivedPacket);
    if (obsoleteActions.count(lastTransmittedPacket->getId()) ||
        (dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(lastTransmittedPacket->peekAtFront<Ieee80211MacHeader>()) &&
            !edca->getChannelOwner()->getInProgressFrames()->containsFrame(lastTransmittedPacket)))
        return;
    EV_INFO << "Processing received frame " << receivedPacket->getName() << " as originator in frame sequence.\n";
    auto receivedHeader = receivedPacket->peekAtFront<Ieee80211MacHeader>();
    auto lastTransmittedHeader = lastTransmittedPacket->peekAtFront<Ieee80211MacHeader>();
    auto step = check_and_cast<TransmitStep *>(frameSequenceHandler->getContext()->getStepBeforeLast());
    rx->successfulFrameTransmitted(step->getPpduEnd(), lastTransmittedHeader->getDurationField());
    auto edcaf = edca->getChannelOwner();
    if (edcaf) {
        AccessCategory ac = edcaf->getAccessCategory();
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(receivedHeader))
            originatorProcessReceivedDataFrame(dataHeader, lastTransmittedHeader, ac);
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(receivedHeader))
            originatorProcessReceivedManagementFrame(mgmtHeader, lastTransmittedHeader, ac);
        else
            originatorProcessReceivedControlFrame(receivedPacket, receivedHeader, lastTransmittedPacket, lastTransmittedHeader, ac);
    }
    else
        throw cRuntimeError("Hcca is unimplemented!");
}

void Hcf::originatorProcessReceivedManagementFrame(const Ptr<const Ieee80211MgmtHeader>& header, const Ptr<const Ieee80211MacHeader>& lastTransmittedHeader, AccessCategory ac)
{
    throw cRuntimeError("Unknown management frame");
}

void Hcf::originatorProcessReceivedControlFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, Packet *lastTransmittedPacket, const Ptr<const Ieee80211MacHeader>& lastTransmittedHeader, AccessCategory ac)
{
    auto edcaf = edca->getEdcaf(ac);
    if (auto ackFrame = dynamicPtrCast<const Ieee80211AckFrame>(header)) {
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(lastTransmittedHeader)) {
            if (dataAndMgmtRateControl) {
                int retryCount;
                if (dataHeader->getRetry())
                    retryCount = edcaf->getRecoveryProcedure()->getRetryCount(lastTransmittedPacket, dataHeader);
                else
                    retryCount = 0;
                dataAndMgmtRateControl->frameTransmitted(lastTransmittedPacket, retryCount, true, false);
            }
            edcaf->getRecoveryProcedure()->ackFrameReceived(lastTransmittedPacket, dataHeader);
        }
        else if (auto mgmtHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(lastTransmittedHeader)) {
            if (dataAndMgmtRateControl) {
                int retryCount = edcaf->getMgmtAndNonQoSRecoveryProcedure()->getRetryCount(lastTransmittedPacket, mgmtHeader);
                dataAndMgmtRateControl->frameTransmitted(lastTransmittedPacket, retryCount, true, false);
            }
            edcaf->getMgmtAndNonQoSRecoveryProcedure()->ackFrameReceived(lastTransmittedPacket, mgmtHeader, edcaf->getStationRetryCounters());
        }
        else
            throw cRuntimeError("Unknown frame"); // TODO qos, nonqos frame
        auto lastTransmittedDataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(lastTransmittedHeader);
        edcaf->getAckHandler()->processReceivedAck(ackFrame, lastTransmittedDataOrMgmtHeader);
        edcaf->getInProgressFrames()->dropFrame(lastTransmittedPacket);
        advanceOriginatorWindows();
        edcaf->getAckHandler()->dropFrame(lastTransmittedDataOrMgmtHeader);
        if (dynamicPtrCast<const Ieee80211MgmtHeader>(lastTransmittedHeader)) {
            actionTerminated(lastTransmittedPacket, true);
            FrameTransmissionDetails transmissionDetails;
            transmissionDetails.setStatus(FRAME_TRANSMISSION_STATUS_ACKNOWLEDGED);
            emit(Ieee80211Mac::frameTransmissionOutcomeSignal, lastTransmittedPacket, &transmissionDetails);
        }
        completeTransfers();
    }
    else if (auto blockAck = dynamicPtrCast<const Ieee80211BasicBlockAck>(header)) {
        if (!originatorBlockAckAgreementHandler ||
            !originatorBlockAckAgreementHandler->getAgreement(blockAck->getTransmitterAddress(), blockAck->getTidInfo()))
            return;
        EV_INFO << "BasicBlockAck has arrived" << std::endl;
        edcaf->getRecoveryProcedure()->blockAckFrameReceived();
        auto ackedSeqAndFragNums = edcaf->getAckHandler()->processReceivedBlockAck(blockAck);
        if (originatorBlockAckAgreementHandler)
            originatorBlockAckAgreementHandler->processReceivedBlockAck(blockAck, this);
        EV_TRACE << "It has acknowledged the following frames:" << std::endl;
        for (auto it : ackedSeqAndFragNums)
            EV_TRACE << "   sequenceNumber = " << it.second.second.getSequenceNumber() << ", fragmentNumber = " << (int)it.second.second.getFragmentNumber() << std::endl;
        edcaf->getInProgressFrames()->dropFrames(ackedSeqAndFragNums);
        advanceOriginatorWindows();
        edcaf->getAckHandler()->dropFrames(ackedSeqAndFragNums);
        completeTransfers();
    }
    else if (dynamicPtrCast<const Ieee80211RtsFrame>(header))
        ; // void
    else if (dynamicPtrCast<const Ieee80211CtsFrame>(header)) {
        // IEEE Std 802.11-2024, 10.3.3: reception of CTS in response to RTS resets the SSRC to 0.
        // Dispatch the reset to the recovery procedure for the protected frame.
        // The preceding RTS step retains the actual protected frame throughout
        // the CTS receive callback; the RTS header itself cannot classify it.
        auto rtsStep = check_and_cast<RtsTransmitStep *>(frameSequenceHandler->getContext()->getStepBeforeLast());
        auto protectedHeader = rtsStep->getProtectedFrame()->peekAtFront<Ieee80211DataOrMgmtHeader>();
        if (dynamicPtrCast<const Ieee80211MgmtHeader>(protectedHeader))
            edcaf->getMgmtAndNonQoSRecoveryProcedure()->ctsFrameReceived(edcaf->getStationRetryCounters());
        else
            edcaf->getRecoveryProcedure()->ctsFrameReceived();
    }
    else if (header->getType() == ST_DATA_WITH_QOS)
        ; // void
    else if (dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header))
        ; // void
    else
        throw cRuntimeError("Unknown control frame");
}

void Hcf::originatorProcessReceivedDataFrame(const Ptr<const Ieee80211DataHeader>& header, const Ptr<const Ieee80211MacHeader>& lastTransmittedHeader, AccessCategory ac)
{
    throw cRuntimeError("Unknown data frame");
}

bool Hcf::hasFrameToTransmit(AccessCategory ac)
{
    expireDataFrames();
    return hasEligibleFrameToTransmit(ac);
}

bool Hcf::hasEligibleFrameToTransmit(AccessCategory ac) const
{
    auto edcaf = edca->getEdcaf(ac);
    if (edcaf)
        return (originatorBlockAckAgreementHandler &&
            originatorAckPolicy->isBlockAckReqNeeded(edcaf->getInProgressFrames(), edcaf->getTxopProcedure(), originatorBlockAckAgreementHandler)) ||
            edcaf->getInProgressFrames()->hasEligibleInProgressFrames() ||
            check_and_cast<OriginatorQosMacDataService *>(originatorDataService)->peekEligibleFrame(edcaf->getPendingQueue()) != nullptr;
    else
        throw cRuntimeError("Hcca is unimplemented");
}

bool Hcf::hasFrameToTransmit()
{
    expireDataFrames();
    auto edcaf = edca->getChannelOwner();
    if (edcaf)
        return hasFrameToTransmit(edcaf->getAccessCategory());
    else
        throw cRuntimeError("Hcca is unimplemented");
}

void Hcf::sendUp(const std::vector<Packet *>& completeFrames)
{
    for (auto frame : completeFrames)
        mac->sendUpFrame(frame);
}

simtime_t Hcf::encodeDuration(simtime_t duration, bool holder)
{
    // IEEE Std 802.11-2024, 9.2.5.1 and Table 9-9: clamp, then ceiling to whole microseconds.
    auto microsecond = SimTime(1, SIMTIME_US).raw();
    auto raw = std::max(SIMTIME_ZERO, duration).raw();
    auto microseconds = raw / microsecond + (raw % microsecond != 0);
    if (holder && microseconds > 32767)
        throw cRuntimeError("Protection duration exceeds 32767 microseconds");
    return SimTime(microseconds, SIMTIME_US);
}

simtime_t Hcf::computeMultipleProtectionDuration(simtime_t limit, simtime_t remaining,
        simtime_t reservation, simtime_t ppduDuration, simtime_t exchangeDuration, bool reservationEstablished)
{
    // IEEE Std 802.11-2024, 9.2.5.2(b): select the upper bound for a positive limit.
    simtime_t duration;
    if (limit == 0)
        duration = (reservation == 0 ? exchangeDuration : reservation) - ppduDuration;
    // An expired positive-limit reservation must not select the initial full-limit field again.
    else if (!reservationEstablished)
        duration = limit - ppduDuration;
    else {
        if (reservation - ppduDuration > remaining - ppduDuration)
            throw cRuntimeError("Multiple protection duration has an empty raw interval");
        duration = remaining - ppduDuration;
    }
    return encodeDuration(duration, true);
}

simtime_t Hcf::estimateZeroLimitExchange(Packet *packet, const Ptr<const Ieee80211MacHeader>& header,
        TxopProcedure *txop, simtime_t ppduDuration)
{
    auto sifs = modeSet->getSifsTime();
    if (auto rts = dynamicPtrCast<const Ieee80211RtsFrame>(header)) {
        auto step = check_and_cast<RtsTransmitStep *>(frameSequenceHandler->getContext()->getLastStep());
        // The step refers to the selected protected frame. Prediction must not extract another queued unit.
        std::unique_ptr<Packet> predicted(step->getProtectedFrame()->dup());
        auto protectedHeader = predicted->peekAtFront<Ieee80211DataOrMgmtHeader>();
        if (auto data = dynamicPtrCast<const Ieee80211DataHeader>(protectedHeader)) {
            OriginatorBlockAckAgreement *agreement = nullptr;
            if (originatorBlockAckAgreementHandler)
                agreement = originatorBlockAckAgreementHandler->getAgreement(data->getReceiverAddress(), data->getTid());
            auto policy = originatorAckPolicy->computeAckPolicy(predicted.get(), data, agreement);
            auto updated = predicted->removeAtFront<Ieee80211DataHeader>();
            updated->setAckPolicy(policy);
            predicted->insertAtFront(updated);
            protectedHeader = predicted->peekAtFront<Ieee80211DataOrMgmtHeader>();
        }
        // Rate queries retain their existing effects. The local copy isolates only frame changes.
        auto mode = rateSelection->computeMode(predicted.get(), protectedHeader, txop);
        setFrameMode(predicted.get(), protectedHeader, mode);
        auto protectedDuration = mode->getDuration(predicted->getDataLength());
        auto ctsDuration = rateSelection->computeResponseCtsFrameMode(packet, rts, ResponseQueryRole::PREDICT)->getDuration(LENGTH_CTS);
        return ppduDuration + sifs + ctsDuration + sifs +
            estimateZeroLimitExchange(predicted.get(), protectedHeader, txop, protectedDuration);
    }
    if (auto bar = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header))
        return ppduDuration + sifs + rateSelection->computeResponseBlockAckFrameMode(packet, bar, ResponseQueryRole::PREDICT)->getDuration(LENGTH_BASIC_BLOCKACK);
    if (auto dataOrMgmt = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header)) {
        if (recipientAckPolicy->isAckNeeded(dataOrMgmt))
            return ppduDuration + sifs + rateSelection->computeResponseAckFrameMode(packet, dataOrMgmt, ResponseQueryRole::PREDICT)->getDuration(LENGTH_ACK);
        return ppduDuration;
    }
    throw cRuntimeError("Unsupported zero-limit multiple protection frame");
}

simtime_t Hcf::computeImmediateResponseDuration(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header)
{
    return recipientAckPolicy->isAckNeeded(header) ?
        rateSelection->computeResponseAckFrameMode(packet, header, ResponseQueryRole::PREDICT)->getDuration(LENGTH_ACK) : SIMTIME_ZERO;
}

simtime_t Hcf::forecastSingleProtectionSuccessor(Packet *packet, const Ptr<const Ieee80211DataOrMgmtHeader>& header,
        TxopProcedure *txop, InProgressFrames *frames, simtime_t completionTime)
{
    // The current group sequence cannot continue. A zero limit permits only the initial exchange.
    if (header->getReceiverAddress().isMulticast() || txop->getLimit() == 0)
        return SIMTIME_ZERO;
    auto candidate = frames->getPendingFrameFor(packet);
    if (!candidate || candidate->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress().isMulticast())
        return SIMTIME_ZERO;
    std::unique_ptr<Packet> forecast(candidate->dup());
    auto nextHeader = forecast->peekAtFront<Ieee80211DataOrMgmtHeader>();
    if (auto data = dynamicPtrCast<const Ieee80211DataHeader>(nextHeader)) {
        auto policy = originatorAckPolicy->computeAckPolicy(forecast.get(), data, nullptr);
        auto updated = forecast->removeAtFront<Ieee80211DataHeader>();
        updated->setAckPolicy(policy);
        forecast->insertAtFront(updated);
        nextHeader = forecast->peekAtFront<Ieee80211DataOrMgmtHeader>();
    }
    auto dataMode = rateSelection->computeMode(forecast.get(), nextHeader, txop);
    setFrameMode(forecast.get(), nextHeader, dataMode);
    auto sifs = modeSet->getSifsTime();
    auto responseDuration = computeImmediateResponseDuration(forecast.get(), nextHeader);
    auto completeCost = dataMode->getDuration(forecast->getDataLength());
    if (recipientAckPolicy->isAckNeeded(nextHeader))
        completeCost += sifs + responseDuration;
    auto protectionCost = completeCost;
    if (rtsPolicy->isRtsNeeded(forecast.get(), nextHeader)) {
        Packet rts("forecast-RTS", rtsProcedure->buildRtsFrame(nextHeader));
        rts.insertAtBack(makeShared<Ieee80211MacTrailer>());
        auto rtsHeader = rts.peekAtFront<Ieee80211RtsFrame>();
        auto rtsMode = rateSelection->computeMode(&rts, rtsHeader, txop);
        setFrameMode(&rts, rtsHeader, rtsMode);
        auto ctsDuration = rateSelection->computeResponseCtsFrameMode(&rts, rtsHeader, ResponseQueryRole::PREDICT)->getDuration(LENGTH_CTS);
        // The field estimates the next holder frame and its response, not its whole protected exchange.
        protectionCost = rtsMode->getDuration(rts.getDataLength()) + sifs + ctsDuration;
        completeCost += protectionCost + sifs;
    }
    return completionTime + sifs + completeCost <= txop->getStart() + txop->getLimit() ?
        protectionCost : SIMTIME_ZERO;
}

bool Hcf::transmitFrame(Packet *packet, simtime_t ifs)
{
    Enter_Method("transmitFrame");
    auto channelOwner = edca->getChannelOwner();
    if (channelOwner) {
        auto header = packet->peekAtFront<Ieee80211MacHeader>();
        if (dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header) &&
            !channelOwner->getInProgressFrames()->containsFrame(packet))
            return false;
        if (!isFrameEligible(packet, true))
            return false;
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
            auto status = channelOwner->getAckHandler()->getQoSDataAckStatus(dataHeader);
            if (dataHeader->getType() == ST_DATA_WITH_QOS &&
                (status == QosAckHandler::Status::BLOCK_ACK_NOT_ARRIVED || status == QosAckHandler::Status::BLOCK_ACK_ARRIVED_UNACKED)) {
                // IEEE Std 802.11-2024, 9.2.4.1.6: DATA after a failed Block Ack exchange is a retransmission.
                auto updated = packet->removeAtFront<Ieee80211DataHeader>();
                updated->setRetry(true);
                packet->insertAtFront(updated);
                header = updated;
            }
        }
        auto lifetimeHandler = edca->getLifetimeHandler();
        auto step = frameSequenceHandler->getContext()->getLastStep();
        auto protectedPacket = dynamic_cast<RtsTransmitStep *>(step);
        auto data = protectedPacket ? protectedPacket->getProtectedFrame() : packet;
        // IEEE Std 802.11-2024, 10.23.2.12.1: a delayed start cannot exceed DATA lifetime.
        if (lifetimeHandler->isExpired(data, simTime() + ifs))
            return false;
        auto txop = channelOwner->getTxopProcedure();
        auto context = frameSequenceHandler->getContext();
        bool scopedSingle = txop->getProtectionMechanism() == TxopProcedure::SINGLE_PROTECTION && !par("isBlockAckSupported").boolValue();
        Packet *retainedPacket = packet;
        auto retainedModeTag = scopedSingle ? retainedPacket->findTag<Ieee80211ModeReq>() : nullptr;
        auto retainedMode = retainedModeTag ? retainedModeTag->getMode() : nullptr;
        bool dataAfterCts = false;
        if (scopedSingle && context->getNumSteps() >= 3) {
            auto rtsStep = dynamic_cast<RtsTransmitStep *>(context->getStep(context->getNumSteps() - 3));
            auto receiveStep = dynamic_cast<IReceiveStep *>(context->getStepBeforeLast());
            dataAfterCts = rtsStep && rtsStep->getProtectedFrame() == packet && receiveStep &&
                receiveStep->getCompletion() == IFrameSequenceStep::Completion::ACCEPTED &&
                receiveStep->getReceivedFrame()->peekAtFront<Ieee80211MacHeader>()->getType() == ST_CTS;
        }
        auto mode = dataAfterCts ? packet->getTag<Ieee80211ModeReq>()->getMode() : rateSelection->computeMode(packet, header, txop);
        setFrameMode(packet, header, mode);
        if (auto bar = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(header)) {
            auto agreement = originatorBlockAckAgreementHandler ?
                originatorBlockAckAgreementHandler->getAgreement(bar->getReceiverAddress(), bar->getTidInfo()) : nullptr;
            if (!agreement || !agreement->getIsAddbaResponseReceived())
                return false;
            auto response = rateSelection->computeResponseBlockAckFrameMode(packet, bar, ResponseQueryRole::PREDICT)->getDuration(LENGTH_BASIC_BLOCKACK);
            auto cost = mode->getDuration(packet->getDataLength()) + modeSet->getSifsTime() + response;
            // IEEE Std 802.11-2024, 10.23.2.8 and 10.23.2.9: a later BAR must fit both bounds.
            if (context->getNumSteps() > 1 &&
                ((txop->getLimit() > 0 && ifs + cost > txop->getRemaining()) ||
                 (txop->getProtectionMechanism() == TxopProcedure::SINGLE_PROTECTION && cost >= rx->getTxnavRemaining())))
                return false;
        }
        if (scopedSingle) {
            auto sifs = modeSet->getSifsTime();
            auto cost = mode->getDuration(packet->getDataLength());
            simtime_t responseDuration = SIMTIME_ZERO;
            simtime_t nextFrameDuration = SIMTIME_ZERO;
            bool finalFrame = true;
            if (auto rts = dynamicPtrCast<const Ieee80211RtsFrame>(header)) {
                auto step = check_and_cast<RtsTransmitStep *>(context->getLastStep());
                auto protectedPacket = const_cast<Packet *>(step->getProtectedFrame());
                retainedPacket = protectedPacket;
                retainedModeTag = retainedPacket->findTag<Ieee80211ModeReq>();
                retainedMode = retainedModeTag ? retainedModeTag->getMode() : nullptr;
                auto protectedHeader = protectedPacket->peekAtFront<Ieee80211DataOrMgmtHeader>();
                auto dataMode = rateSelection->computeMode(protectedPacket, protectedHeader, txop);
                setFrameMode(protectedPacket, protectedHeader, dataMode);
                responseDuration = rateSelection->computeResponseCtsFrameMode(packet, rts, ResponseQueryRole::PREDICT)->getDuration(LENGTH_CTS);
                nextFrameDuration = dataMode->getDuration(protectedPacket->getDataLength());
                auto ackDuration = computeImmediateResponseDuration(protectedPacket, protectedHeader);
                if (recipientAckPolicy->isAckNeeded(protectedHeader))
                    nextFrameDuration += sifs + ackDuration;
                cost += sifs + responseDuration + sifs + nextFrameDuration;
            }
            else if (auto dataOrMgmt = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header)) {
                responseDuration = computeImmediateResponseDuration(packet, dataOrMgmt);
                if (recipientAckPolicy->isAckNeeded(dataOrMgmt))
                    cost += sifs + responseDuration;
            }
            else
                throw cRuntimeError("Unsupported single protection holder frame");
            // IEEE Std 802.11-2024, 10.23.2.8: compare the exchange duration strictly against TXNAV.
            // INET samples at the preceding exchange's completion, before the initial SIFS wait.
            // The separate TXOP limit includes that wait (10.23.2.9).
            if (!dataAfterCts && context->getNumSteps() > 1 &&
                (txop->getLimit() == 0 || cost >= rx->getTxnavRemaining() || ifs + cost > txop->getRemaining())) {
                // The response estimate needs the selected mode; refusal must retain the frame's prior tag.
                if (retainedModeTag)
                    retainedPacket->addTagIfAbsent<Ieee80211ModeReq>()->setMode(retainedMode);
                else
                    retainedPacket->removeTagIfPresent<Ieee80211ModeReq>();
                return false;
            }
            if (auto dataOrMgmt = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header)) {
                nextFrameDuration = forecastSingleProtectionSuccessor(packet, dataOrMgmt, txop,
                    channelOwner->getInProgressFrames(), simTime() + ifs + cost);
                finalFrame = nextFrameDuration == 0;
            }
            auto duration = singleProtectionMechanism->computeDurationField(header, responseDuration, nextFrameDuration, finalFrame);
            auto updated = packet->removeAtFront<Ieee80211MacHeader>();
            updated->setDurationField(encodeDuration(duration, true));
            packet->insertAtFront(updated);
        }
        else if (txop->getProtectionMechanism() == TxopProcedure::ProtectionMechanism::SINGLE_PROTECTION) {
            auto pendingPacket = dynamicPtrCast<const Ieee80211BlockAckReq>(header) ? nullptr :
                channelOwner->getInProgressFrames()->getPendingFrameFor(packet, false);
            if (dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header) && txop->getLimit() == 0) {
                // The zero-limit frame sequence ends after this exchange; it has no successor reservation.
                pendingPacket = nullptr;
            }
            else if (pendingPacket && !channelOwner->getInProgressFrames()->containsFrame(pendingPacket)) {
                // HcfFs ends this TXOP before extraction can change the successor's size or setup state.
                pendingPacket = nullptr;
            }
            const auto& pendingHeader = pendingPacket == nullptr ? nullptr : pendingPacket->peekAtFront<Ieee80211DataOrMgmtHeader>();
            auto duration = singleProtectionMechanism->computeDurationField(packet, header, pendingPacket, pendingHeader, txop, recipientAckPolicy);
            auto header = packet->removeAtFront<Ieee80211MacHeader>();
            header->setDurationField(duration);
            EV_DEBUG << "Duration for " << packet->getName() << " is set to " << duration << " s.\n";
            packet->insertAtFront(header);
        }
        else if (txop->getProtectionMechanism() == TxopProcedure::ProtectionMechanism::MULTIPLE_PROTECTION) {
            header = packet->peekAtFront<Ieee80211MacHeader>();
            auto ppduDuration = mode->getDuration(packet->getDataLength());
            auto projectedStart = simTime() + ifs;
            auto reservation = std::max(SIMTIME_ZERO, txop->getReservationEnd() - projectedStart);
            auto remaining = std::max(SIMTIME_ZERO, txop->getStart() + txop->getLimit() - projectedStart);
            auto exchangeDuration = txop->getLimit() == 0 && reservation == 0 ?
                estimateZeroLimitExchange(packet, header, txop, ppduDuration) : SIMTIME_ZERO;
            auto duration = computeMultipleProtectionDuration(txop->getLimit(), remaining, reservation,
                ppduDuration, exchangeDuration, txop->getReservationEnd() != 0);
            auto updated = packet->removeAtFront<Ieee80211MacHeader>();
            updated->setDurationField(duration);
            packet->insertAtFront(updated);
        }
        else
            throw cRuntimeError("Undefined protection mechanism");
        RateSelection::emitDatarateSelected(this, header, mode);
        EV_DEBUG << "Datarate for " << packet->getName() << " is set to " << mode->getDataMode()->getNetBitrate() << ".\n";
        tx->transmitFrame(packet, packet->peekAtFront<Ieee80211MacHeader>(), ifs, false, this);
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
            if (originatorBlockAckAgreementHandler) {
                auto agreement = originatorBlockAckAgreementHandler->getAgreement(dataHeader->getReceiverAddress(), dataHeader->getTid());
                if (agreement && agreement->getIsAddbaResponseReceived())
                    agreement->recordTransmittedData(packet->getTag<Ieee80211DataSequenceTag>()->getAllocationIndex());
            }
        }
        auto action = ownedActions.find(packet->getId());
        if (action != ownedActions.end()) {
            action->second.attempted = true;
            if (action->second.kind == OwnedAction::REQUEST)
                originatorBlockAckAgreementHandler->markRequestAttempted(action->second.peer, action->second.tid);
        }
        return true;
    }
    else
        throw cRuntimeError("Hcca is unimplemented");
}

void Hcf::transmitControlResponseFrame(Packet *responsePacket, const Ptr<const Ieee80211MacHeader>& responseHeader, Packet *receivedPacket, const Ptr<const Ieee80211MacHeader>& receivedHeader)
{
    Enter_Method("transmitControlResponseFrame");
    responsePacket->insertAtBack(makeShared<Ieee80211MacTrailer>());
    const IIeee80211Mode *responseMode = nullptr;
    if (auto rtsFrame = dynamicPtrCast<const Ieee80211RtsFrame>(receivedHeader))
        responseMode = rateSelection->computeResponseCtsFrameMode(receivedPacket, rtsFrame, ResponseQueryRole::RESPOND);
    else if (auto blockAckReq = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(receivedHeader))
        responseMode = rateSelection->computeResponseBlockAckFrameMode(receivedPacket, blockAckReq, ResponseQueryRole::RESPOND);
    else if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(receivedHeader))
        responseMode = rateSelection->computeResponseAckFrameMode(receivedPacket, dataOrMgmtHeader, ResponseQueryRole::RESPOND);
    else
        throw cRuntimeError("Unknown received frame type");
    setFrameMode(responsePacket, responseHeader, responseMode);
    RateSelection::emitDatarateSelected(this, responseHeader, responseMode);
    EV_DEBUG << "Datarate for " << responsePacket->getName() << " is set to " << responseMode->getDataMode()->getNetBitrate() << ".\n";
    auto updated = responsePacket->removeAtFront<Ieee80211MacHeader>();
    updated->setDurationField(encodeDuration(updated->getDurationField(), false));
    responsePacket->insertAtFront(updated);
    tx->transmitFrame(responsePacket, responsePacket->peekAtFront<Ieee80211MacHeader>(), modeSet->getSifsTime(), false, this);
    delete responsePacket;
}

void Hcf::recipientProcessTransmittedControlResponseFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    emit(packetSentToPeerSignal, packet);
    if (auto ctsFrame = dynamicPtrCast<const Ieee80211CtsFrame>(header))
        ctsProcedure->processTransmittedCts(ctsFrame);
    else if (auto blockAck = dynamicPtrCast<const Ieee80211BlockAck>(header)) {
        if (recipientBlockAckProcedure)
            recipientBlockAckProcedure->processTransmittedBlockAck(blockAck);
    }
    else if (auto ackFrame = dynamicPtrCast<const Ieee80211AckFrame>(header))
        recipientAckProcedure->processTransmittedAck(ackFrame);
    else
        throw cRuntimeError("Unknown control response frame");
}

void Hcf::processMgmtFrame(Packet *mgmtPacket, const Ptr<const Ieee80211MgmtHeader>& mgmtHeader)
{
    Enter_Method("processMgmtFrame");
    OwnedAction action;
    action.peer = mgmtHeader->getReceiverAddress();
    bool owned = true;
    if (auto request = dynamicPtrCast<const Ieee80211AddbaRequest>(mgmtHeader)) {
        action.tid = request->getTid();
        action.originator = true;
        action.kind = OwnedAction::REQUEST;
    }
    else if (auto response = dynamicPtrCast<const Ieee80211AddbaResponse>(mgmtHeader)) {
        action.tid = response->getTid();
        action.originator = false;
        action.kind = OwnedAction::RESPONSE;
    }
    else if (auto delba = dynamicPtrCast<const Ieee80211Delba>(mgmtHeader)) {
        action.tid = delba->getTid();
        action.originator = delba->getInitiator();
        action.kind = OwnedAction::DELBA;
        bool pending = action.originator ?
            originatorBlockAckAgreementHandler->getProcedures().count({action.peer, action.tid}) != 0 &&
                originatorBlockAckAgreementHandler->getProcedures().at({action.peer, action.tid}).state == IOriginatorBlockAckAgreementHandler::ProcedureState::TEARDOWN_PENDING :
            recipientBlockAckAgreementHandler->isTeardownPending(action.peer, action.tid);
        if (!pending) {
            beginTeardown(action.peer, action.tid, action.originator, delba->getReasonCode());
            delete mgmtPacket;
            return;
        }
    }
    else
        owned = false;
    if (owned)
        ownedActions.emplace(mgmtPacket->getId(), action);
    mgmtPacket->insertAtBack(makeShared<Ieee80211MacTrailer>());
    processUpperFrame(mgmtPacket, mgmtHeader);
}

void Hcf::setFrameMode(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, const IIeee80211Mode *mode) const
{
    ASSERT(mode != nullptr);
    packet->addTagIfAbsent<Ieee80211ModeReq>()->setMode(mode);
}

bool Hcf::isReceptionInProgress()
{
    return rx->isReceptionInProgress();
}

bool Hcf::isForUs(const Ptr<const Ieee80211MacHeader>& header) const
{
    return header->getReceiverAddress() == mac->getAddress() || (header->getReceiverAddress().isMulticast() && !isSentByUs(header));
}

bool Hcf::isSentByUs(const Ptr<const Ieee80211MacHeader>& header) const
{
    // FIXME
    // Check the roles of the Addr3 field when aggregation is applied
    // Table 8-19—Address field contents
    if (auto dataOrMgmtHeader = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header))
        return dataOrMgmtHeader->getAddress3() == mac->getAddress();
    else
        return false;
}

void Hcf::corruptedFrameReceived()
{
    Enter_Method("corruptedFrameReceived");
    if (frameSequenceHandler->isSequenceRunning() && !startRxTimer->isScheduled()) {
        frameSequenceHandler->handleStartRxTimeout();
    }
    else
        EV_DEBUG << "Ignoring received corrupt frame.\n";
}

Hcf::~Hcf()
{
    cancelAndDelete(addbaResponseTimer);
    cancelAndDelete(dataLifetimeTimer);
    cancelAndDelete(startRxTimer);
    cancelAndDelete(inactivityTimer);
    delete recipientAckProcedure;
    delete ctsProcedure;
    delete rtsProcedure;
    delete originatorBlockAckAgreementHandler;
    delete recipientBlockAckAgreementHandler;
    delete originatorBlockAckProcedure;
    delete recipientBlockAckProcedure;
    delete frameSequenceHandler;
}

} // namespace ieee80211
} // namespace inet
