//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211DataSequenceTag_m.h"
#include "inet/linklayer/ieee80211/mac/contract/FrameTransmissionDetails_m.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckProcedure.h"
#include "inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"
#include "inet/linklayer/ieee80211/mac/framesequence/HcfFs.h"
#include "inet/physicallayer/wireless/ieee80211/mode/Ieee80211HtMode.h"
#include "inet/linklayer/ieee80211/mac/rateselection/RateSelection.h"
#include "inet/linklayer/ieee80211/mac/rateselection/Ieee80211PeerModeSelection.h"
#include "inet/linklayer/ieee80211/mac/recipient/RecipientAckProcedure.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtTransactionTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"
#include "inet/queueing/contract/ISelectivePacketCollection.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Transmission.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211PhyHeader_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"

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
        WATCH_EXPR("numPendingInterframeReceptions", pendingReceptions.size());
        originatorDataService = check_and_cast<IOriginatorMacDataService *>(getSubmodule(("originatorMacDataService")));
        recipientDataService = check_and_cast<IRecipientQosMacDataService *>(getSubmodule("recipientMacDataService"));
        originatorAckPolicy = check_and_cast<IOriginatorQoSAckPolicy *>(getSubmodule("originatorAckPolicy"));
        originatorAckPolicy->setProbeEligibility([this](InProgressFrames *frames, Tid tid) {
            auto header = makeShared<Ieee80211DataHeader>();
            header->setType(ST_DATA_WITH_QOS);
            header->setTid(tid);
            return edca->getEdcaf(edca->classifyFrame(header))->getInProgressFrames() == frames;
        });
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
        if (check_and_cast<IOriginatorQosMacDataService *>(originatorDataService)->hasAMpduAggregation() && dataAndMgmtRateControl)
            throw cRuntimeError("HT A-MPDU transmission requires fixed DATA rates without a rate-control submodule");
        if (blockAckVariant == BlockAckVariant::COMPRESSED && par("isBlockAckSupported").boolValue()) {
            interframeRadio = check_and_cast<IRadio *>(mac->gate("lowerLayerOut")->getNextGate()->getOwnerModule());
            auto radioModule = check_and_cast<cModule *>(interframeRadio.get());
            for (auto signal : {receptionStartedSignal, receptionEndedSignal, transmissionStartedSignal, transmissionEndedSignal, IRadio::receptionStateChangedSignal})
                radioModule->subscribe(signal, this);
        }
        if (par("isBlockAckSupported").boolValue()) {
            EV_INFO << "Block Ack profile: " << (blockAckVariant == BlockAckVariant::BASIC ? "basic" : "compressed") << endl;
            if (blockAckVariant == BlockAckVariant::COMPRESSED) {
                if (originatorBlockAckAgreementPolicy && originatorBlockAckAgreementPolicy->isDelayedAckPolicySupported())
                    throw cRuntimeError("Compressed Block Ack does not support delayedAckPolicySupported in %s.originatorBlockAckAgreementPolicy", getFullPath().c_str());
                if (recipientBlockAckAgreementPolicy && recipientBlockAckAgreementPolicy->delayedBlockAckPolicySupported())
                    throw cRuntimeError("Compressed Block Ack does not support delayedAckPolicySupported in %s.recipientBlockAckAgreementPolicy", getFullPath().c_str());
                if (!mib->isHtOperationSupported())
                    EV_WARN << "Compressed Block Ack has no usable local HT support; DATA uses Normal Ack. Set blockAckVariant = basic for Basic compatibility.\n";
            }
        }
        // Edca resolves its Edcaf array at the link-layer stage. Install the
        // queue signal listeners after all child initialization has completed.
        auto lifetimeHandler = edca->getLifetimeHandler();
        lifetimeHandler->setDeadlineChangedCallback([this]() { scheduleDataLifetimeTimer(); });
        check_and_cast<OriginatorQosMacDataService *>(originatorDataService)->setLifetimeHandler(
            lifetimeHandler, [this]() { expireDataFrames(); });
        check_and_cast<OriginatorQosMacDataService *>(originatorDataService)->setEligibility(
            [this](Packet *packet) { return isFrameEligible(packet, false); },
            [this](Packet *packet) { return isAMsduPermitted(packet); });
        check_and_cast<OriginatorQosMacDataService *>(originatorDataService)->setFragmentationPermission([this](Packet *packet) {
            auto data = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
            if (!data || data->getType() != ST_DATA_WITH_QOS || !originatorBlockAckAgreementHandler)
                return true;
            auto agreement = originatorBlockAckAgreementHandler->getAgreement(data->getReceiverAddress(), data->getTid());
            auto peer = mib->findPeerHtState(data->getReceiverAddress());
            bool htAgreement = blockAckVariant == BlockAckVariant::COMPRESSED ||
                (mib->isHtOperationSupported() && peer && peer->valid);
            return !agreement || !agreement->getIsAddbaResponseReceived() || !htAgreement;
        });
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
            if (auto aggregate = dynamic_cast<AmpduTransmitStep *>(step)) {
                for (auto member : aggregate->getPreparedData()->batch->members)
                    if (context->getInProgressFrames()->containsFrame(member))
                        frames.insert(member);
            }
            else if (auto transmit = dynamic_cast<ITransmitStep *>(step))
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
            if (edcaf->getRecoveryProcedure()->hasMsduRetryRecord(packet))
                disposeMsdu(edcaf, packet);
            else
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
    Enter_Method_Silent();
    if (interframeRadio && source == dynamic_cast<cComponent *>(interframeRadio.get()) && signalID == receptionStartedSignal) {
        auto reception = check_and_cast<const IReception *>(obj);
        auto transmission = reception->getTransmission();
        std::set<int> active;
        for (auto ongoing : interframeRadio->getReceptionsInProgress())
            active.insert(ongoing->getId());
        for (auto it = pendingReceptions.begin(); it != pendingReceptions.end();) {
            if (!it->second.completed && !active.count(it->first))
                it = pendingReceptions.erase(it);
            else
                ++it;
        }
        PendingReception pending;
        for (auto& entry : pendingReceptions) {
            if (!entry.second.completed) {
                pending.overlapped = true;
                entry.second.overlapped = true;
                entry.second.previousAggregate.reset();
            }
        }
        auto medium = interframeRadio->getMedium();
        pending.timing = medium->getTransmissionTimingSnapshot(transmission->getId());
        auto wifi = dynamic_cast<const Ieee80211Transmission *>(transmission);
        pending.ht = wifi && wifi->getMode()->getModulationClass() == IIeee80211Mode::ModulationClass::HT;
        if (wifi)
            pending.extension = wifi->getSignalExtensionDuration();
        auto gap = medium->computePrecedingSignalGap(transmission->getId(), interframeHistory.timing);
        if (!pending.overlapped && gap && (interframeHistory.mpdu || interframeHistory.ack ||
            interframeHistory.aggregate || interframeHistory.blockAck)) {
            // IEEE Std 802.11-2024, 10.3.2.3.3: this model uses coverage class zero.
            pending.sifs = std::abs((*gap - modeSet->getSifsTime()).raw()) <= modeSet->getSlotTime().raw() / 10;
            pending.precedingAggregate = bool(interframeHistory.aggregate);
            pending.precedingIndividual = interframeHistory.mpdu || interframeHistory.ack;
            pending.previousAggregate = interframeHistory.aggregate;
            if ((interframeHistory.mpdu || interframeHistory.aggregate) && interframeHistory.ht && pending.ht && interframeHistory.noSifsResponse &&
                interframeHistory.timing.transmitterRadioId == pending.timing.transmitterRadioId && interframeHistory.extension) {
                auto rifs = wifi->getMode()->getRifsTime();
                // IEEE Std 802.11-2024, 10.3.2.3.2 and 10.3.8: RIFS excludes the actual preceding extension.
                pending.rifs = std::abs((*gap + *interframeHistory.extension - rifs).raw()) <= rifs.raw() / 10;
                pending.precedingReceiver = interframeHistory.receiver;
            }
        }
        interframeHistory = {};
        pendingReceptions.insert_or_assign(transmission->getId(), std::move(pending));
    }
    else if (interframeRadio && source == dynamic_cast<cComponent *>(interframeRadio.get()) && signalID == receptionEndedSignal) {
        auto reception = check_and_cast<const IReception *>(obj);
        auto pending = pendingReceptions.find(reception->getTransmission()->getId());
        if (pending != pendingReceptions.end())
            pending->second.completed = true;
    }
    else if (interframeRadio && source == dynamic_cast<cComponent *>(interframeRadio.get()) && signalID == transmissionStartedSignal) {
        auto transmission = check_and_cast<const ITransmission *>(obj);
        auto topology = interframeHistory.aggregate;
        bool compatible = false;
        auto packet = transmission->getPacket();
        if (topology && packet && packet->hasAtFront<Ieee80211PhyHeader>()) {
            auto phy = packet->peekAtFront<Ieee80211PhyHeader>();
            auto ht = dynamicPtrCast<const Ieee80211HtPhyHeader>(phy);
            if (!ht || !ht->getAggregation()) {
                auto header = packet->peekDataAt<Ieee80211MacHeader>(phy->getChunkLength());
                if (auto blockAck = dynamicPtrCast<const Ieee80211CompressedBlockAck>(header))
                    compatible = blockAck->getReceiverAddress() == topology->transmitter && blockAck->getTidInfo() == topology->tid;
            }
        }
        clearInterframeHistory();
        if (compatible)
            transmittedBlockAckAggregate = std::move(topology);
    }
    else if (interframeRadio && source == dynamic_cast<cComponent *>(interframeRadio.get()) && signalID == transmissionEndedSignal) {
        auto topology = transmittedBlockAckAggregate;
        clearInterframeHistory();
        auto transmission = check_and_cast<const ITransmission *>(obj);
        if (completedTransmissionWasAck) {
            interframeHistory.timing = interframeRadio->getMedium()->getTransmissionTimingSnapshot(transmission->getId());
            interframeHistory.ack = true;
        }
        else if (completedTransmissionWasBlockAck && topology) {
            interframeHistory.timing = interframeRadio->getMedium()->getTransmissionTimingSnapshot(transmission->getId());
            interframeHistory.blockAck = true;
            interframeHistory.aggregate = std::move(topology);
        }
        completedTransmissionWasAck = false;
        completedTransmissionWasBlockAck = false;
    }
    else if (signalID == InProgressFrames::packetDequeuedSignal) {
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

void Hcf::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method_Silent();
    if (interframeRadio && source == dynamic_cast<cComponent *>(interframeRadio.get()) && signalID == IRadio::receptionStateChangedSignal &&
        value != IRadio::RECEPTION_STATE_RECEIVING) {
        bool incomplete = false;
        for (auto it = pendingReceptions.begin(); it != pendingReceptions.end();) {
            if (!it->second.completed) {
                incomplete = true;
                it = pendingReceptions.erase(it);
            }
            else
                ++it;
        }
        // Tx owns a prepared response during the switch to transmitter mode. Its start identifies the actual frame.
        if (incomplete || value == IRadio::RECEPTION_STATE_BUSY ||
            (value == IRadio::RECEPTION_STATE_UNDEFINED && !tx->hasTransmission()))
            interframeHistory = {};
    }
}

void Hcf::clearInterframeHistory()
{
    interframeHistory = {};
    pendingReceptions.clear();
    transmittedBlockAckAggregate.reset();
}

bool Hcf::receiveInterframe(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, BlockAckReceiveContext *context)
{
    auto time = packet->findTag<SignalTimeInd>();
    if (!time)
        return false;
    auto entry = pendingReceptions.find(time->getTransmissionId());
    if (entry == pendingReceptions.end() || !entry->second.completed)
        return false;
    auto pending = std::move(entry->second);
    pendingReceptions.erase(entry);
    if (pending.overlapped)
        return false;
    // IEEE Std 802.11-2024, 10.27.3.1, 10.27.3.3, and 11.38.1: the BSS controls RIFS permission.
    bool rifsPermitted = pending.rifs && modeSet->getPhyType() != Ieee80211ModeSet::PhyType::VHT &&
        (mib->mode == Ieee80211Mib::INDEPENDENT || mib->mode == Ieee80211Mib::MESH || mib->getHtOperation().rifsMode);
    bool related = pending.sifs || (pending.rifs && rifsPermitted && pending.precedingReceiver == header->getReceiverAddress());
    if (context) {
        context->followsAggregate = related && pending.precedingAggregate;
        context->followsIndividual = related && pending.precedingIndividual;
        context->previousAggregate = std::move(pending.previousAggregate);
    }
    interframeHistory.timing = std::move(pending.timing);
    interframeHistory.mpdu = true;
    interframeHistory.ht = pending.ht;
    interframeHistory.receiver = header->getReceiverAddress();
    interframeHistory.extension = pending.extension;
    auto dataOrMgmt = dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header);
    interframeHistory.noSifsResponse = dataOrMgmt ? !recipientAckPolicy->isAckNeeded(dataOrMgmt) :
        !dynamicPtrCast<const Ieee80211RtsFrame>(header) && !dynamicPtrCast<const Ieee80211BlockAckReq>(header);
    return related;
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
    bool cancelledAggregate = false;
    if (frameSequenceHandler->isSequenceRunning()) {
        auto prepared = frameSequenceHandler->getContext()->getQoSContext()->preparedData.get();
        if (prepared) {
            auto header = prepared->frame->peekAtFront<Ieee80211DataHeader>();
            if (header->getReceiverAddress() == peer && header->getTid() == tid) {
                prepared->cancelled = true;
                cancelledAggregate = prepared->batch && tx->cancelPendingAggregate(this);
            }
        }
    }
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
            if (!frames->containsFrame(packet))
                continue;
            bool msdu = edcaf->getRecoveryProcedure()->hasMsduRetryRecord(packet);
            auto members = msdu ? frames->getAllocationFrames(packet) : std::vector<Packet *>{packet};
            if (msdu)
                edcaf->getRecoveryProcedure()->retireMsdu(packet);
            for (auto member : members) {
                auto data = member->peekAtFront<Ieee80211DataHeader>();
                edcaf->getAckHandler()->dropFrame(data);
                edcaf->getRecoveryProcedure()->dropFrame(data);
                frames->dropFrame(member);
                PacketDropDetails details;
                details.setReason(OTHER_PACKET_DROP);
                emit(packetDroppedSignal, member, &details);
            }
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
    if (cancelledAggregate)
        aggregateTransmissionCancelled();
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
    if (!originator)
        clearInterframeHistory();
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
    clearInterframeHistory();
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

void Hcf::processLowerAggregate(Packet *packet, AmpduReception received)
{
    Enter_Method("processLowerAggregate");
    auto reception = std::move(received);
    take(packet);
    std::unique_ptr<Packet> envelope(packet);
    for (auto& member : reception.members)
        take(member.packet.get());
    if (edca->getChannelOwner() && frameSequenceHandler->isSequenceRunning() &&
        dynamic_cast<IReceiveStep *>(frameSequenceHandler->getContext()->getLastStep())) {
        bool addressed = false;
        for (const auto& member : reception.members)
            addressed |= isForUs(member.packet->peekAtFront<Ieee80211MacHeader>());
        // An aggregate cannot satisfy a control-response step. Resolve an elapsed or addressed wait first.
        if (!startRxTimer->isScheduled() || addressed) {
            frameSequenceHandler->handleStartRxTimeout();
            cancelEvent(startRxTimer);
        }
    }
    if (!reception.anyCorrect || reception.members.empty() || !reception.topology) {
        clearInterframeHistory();
        return;
    }
    Ptr<const Ieee80211DataHeader> common;
    // IEEE Std 802.11-2024, 9.7.1 and 10.25.6.5: validate the complete supported context before state changes.
    for (const auto& member : reception.members) {
        auto header = dynamicPtrCast<const Ieee80211DataHeader>(member.packet->peekAtFront<Ieee80211MacHeader>());
        if (!header || header->getType() != ST_DATA_WITH_QOS || header->getAckPolicy() != NORMAL_ACK ||
            header->getTid() < 0 || header->getTid() > 15 || !header->getSequenceNumber().isSpecified() ||
            header->getReceiverAddress().isUnspecified() || header->getReceiverAddress().isMulticast() ||
            header->getTransmitterAddress().isUnspecified() || header->getTransmitterAddress().isMulticast() ||
            header->getFragmentNumber() != 0 || header->getMoreFragments() || header->getDurationField() < SIMTIME_ZERO ||
            (common && (common->getReceiverAddress() != header->getReceiverAddress() ||
                common->getTransmitterAddress() != header->getTransmitterAddress() || common->getTid() != header->getTid() ||
                common->getDurationField() != header->getDurationField()))) {
            clearInterframeHistory();
            return;
        }
        common = header;
    }
    if (!isForUs(common) || !recipientBlockAckAgreementHandler) {
        clearInterframeHistory();
        return;
    }
    auto agreement = recipientBlockAckAgreementHandler->getAgreement(common->getTid(), common->getTransmitterAddress());
    // ADDBA has no variant field. An active Basic agreement cannot accept an implicit Compressed request.
    if (agreement && (blockAckVariant != BlockAckVariant::COMPRESSED || agreement->getVariant() != BlockAckVariant::COMPRESSED))
        throw cRuntimeError("Block Ack profile conflict at %s: peer=%s TID=%d local=basic received=implicit-compressed; ADDBA does not negotiate the profile. Configure matching blockAckVariant values and start a new run",
                getFullPath().c_str(), common->getTransmitterAddress().str().c_str(), common->getTid());
    // INET policy: implicit DATA without an agreement causes silent discard and no receive state.
    if (!agreement || recipientBlockAckAgreementHandler->isTeardownPending(common->getTransmitterAddress(), common->getTid()) ||
        !isProfileAllowed(common->getTransmitterAddress(), false)) {
        clearInterframeHistory();
        return;
    }
    auto topology = std::make_shared<AmpduTopology>(*reception.topology);
    topology->transmitter = common->getTransmitterAddress();
    topology->receiver = common->getReceiverAddress();
    topology->tid = common->getTid();
    BlockAckReceiveContext context;
    if (interframeRadio)
        receiveInterframe(packet, common, &context);
    context.aggregate = topology;
    // The PPDU consumes physical history once. Members never consume receiveInterframe separately.
    if (interframeHistory.mpdu) {
        interframeHistory.mpdu = false;
        interframeHistory.aggregate = topology;
        interframeHistory.noSifsResponse = false;
    }
    recipientBlockAckAgreementHandler->implicitBlockAckRequested(common, this);
    for (auto& member : reception.members) {
        auto header = member.packet->peekAtFront<Ieee80211DataHeader>();
        context.memberOffset = member.offset;
        context.firstMember = false;
        for (const auto& position : topology->members) {
            if (position.offset == member.offset) {
                context.firstMember = position.first;
                break;
            }
        }
        take(member.packet.get());
        emit(packetReceivedFromPeerSignal, member.packet.get());
        sendUp(recipientDataService->dataFrameReceived(member.packet.release(), header,
                recipientBlockAckAgreementHandler, false, &context));
    }
    sendImplicitBlockAck(packet, common, agreement);
}

void Hcf::sendImplicitBlockAck(Packet *envelope, const Ptr<const Ieee80211DataHeader>& header,
        RecipientBlockAckAgreement *agreement)
{
    auto mode = rateSelection->computeCompressedBlockAckResponseMode(envelope->getTag<Ieee80211ModeInd>()->getMode());
    auto response = recipientBlockAckProcedure->buildCompressedBlockAck(header->getTransmitterAddress(), header->getTid(), agreement);
    response->setDurationField(encodeDuration(std::max(SIMTIME_ZERO,
        header->getDurationField() - modeSet->getSifsTime() - mode->getDuration(LENGTH_COMPRESSED_BLOCKACK)), false));
    auto packet = new Packet("ImplicitBlockAck", response);
    packet->insertAtBack(makeShared<Ieee80211MacTrailer>());
    setFrameMode(packet, response, mode);
    tx->transmitFrame(packet, response, modeSet->getSifsTime(), false, this);
    delete packet;
}

void Hcf::processLowerFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header)
{
    Enter_Method("processLowerFrame(%s)", packet->getName());
    take(packet);
    if (auto bar = dynamicPtrCast<const Ieee80211BlockAckReq>(header)) {
        if (!validateBlockAckRequest(packet, bar)) {
            interframeHistory = {};
            if (auto time = packet->findTag<SignalTimeInd>())
                pendingReceptions.erase(time->getTransmissionId());
            delete packet;
            return;
        }
    }
    BlockAckReceiveContext receiveContext;
    bool interframe = interframeRadio && receiveInterframe(packet, header, &receiveContext);
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
        recipientProcessReceivedFrame(packet, header, interframe, &receiveContext);
    else {
        EV_INFO << "This frame is not for us" << std::endl;
        PacketDropDetails details;
        details.setReason(NOT_ADDRESSED_TO_US);
        emit(packetDroppedSignal, packet, &details);
        delete packet;
    }
}

bool Hcf::validateBlockAckRequest(Packet *packet, const Ptr<const Ieee80211BlockAckReq>& bar) const
{
    // IEEE Std 802.11-2024, 9.3.1.7.2: the supported Compressed BAR has a zero Fragment Number.
    auto basic = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(bar);
    auto compressed = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(bar);
    auto tid = basic ? basic->getTidInfo() : compressed ? compressed->getTidInfo() : -1;
    bool valid = basic || compressed;
    if (basic)
        valid &= basic->getFragmentNumber() == 0 && !basic->getMultiTid() && !basic->getCompressedBitmap() &&
            basic->getStartingSequenceNumber().isSpecified() && (basic->getReserved() & 3) == 0;
    if (compressed)
        valid &= compressed->getFragmentNumber() == 0 && !compressed->getMultiTid() && compressed->getCompressedBitmap() &&
            compressed->getStartingSequenceNumber().isSpecified();
    if (!valid || tid < 0 || tid > 15 || bar->getChunkLength() != LENGTH_BLOCKACKREQ - B(4) ||
        packet->getDataLength() != LENGTH_BLOCKACKREQ || bar->getTransmitterAddress().isUnspecified() ||
        bar->getTransmitterAddress().isMulticast() || bar->getReceiverAddress().isUnspecified() || bar->getReceiverAddress().isMulticast())
        return false;
    auto agreement = recipientBlockAckAgreementHandler ?
        recipientBlockAckAgreementHandler->getAgreement(tid, bar->getTransmitterAddress()) : nullptr;
    // ADDBA has no variant field. INET stops before any protocol update for an active conflicting profile.
    if (bar->getReceiverAddress() == mac->getAddress() && agreement && bool(compressed) != (blockAckVariant == BlockAckVariant::COMPRESSED))
        throw cRuntimeError("Block Ack profile conflict at %s: peer=%s TID=%d local=%s received=%s; ADDBA does not negotiate the profile. Configure matching blockAckVariant values and start a new run",
            getFullPath().c_str(), bar->getTransmitterAddress().str().c_str(), tid,
            blockAckVariant == BlockAckVariant::COMPRESSED ? "compressed" : "basic", compressed ? "compressed" : "basic");
    return true;
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
    auto qosContext = new QoSContext(originatorAckPolicy, originatorBlockAckProcedure, originatorBlockAckAgreementHandler, edcaf->getTxopProcedure(), blockAckVariant);
    qosContext->prepareData = [this](FrameSequenceContext *context) { return prepareDataTransmission(context); };
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
        auto frames = edcaf->getInProgressFrames();
        Packet *internallyCollidedFrame = nullptr;
        bool scopedQueuedData = false;
        {
            auto candidate = frames->peekFrameToTransmit();
            auto data = candidate ? dynamicPtrCast<const Ieee80211DataHeader>(candidate->peekAtFront<Ieee80211MacHeader>()) : nullptr;
            auto agreement = data && originatorBlockAckAgreementHandler ?
                originatorBlockAckAgreementHandler->getAgreement(data->getReceiverAddress(), data->getTid()) : nullptr;
            if (data && data->getType() == ST_DATA_WITH_QOS && !data->getReceiverAddress().isMulticast() &&
                (usesMsduRecovery(candidate) || (agreement && agreement->getIsAddbaResponseReceived())) && !frames->containsFrame(candidate)) {
                scopedQueuedData = true;
                // Collision recovery needs a numbered frame, but it must not form an unadmitted aggregate.
                auto service = check_and_cast<IOriginatorQosMacDataService *>(originatorDataService);
                auto prepared = service->prepareFrame(edcaf->getPendingQueue(), false);
                if (prepared) {
                    auto selected = prepared->packet->peekAtFront<Ieee80211DataHeader>();
                    if (selected->getReceiverAddress() == data->getReceiverAddress() && selected->getTid() == data->getTid())
                        internallyCollidedFrame = service->commitPreparedFrame(*prepared);
                }
                if (internallyCollidedFrame)
                    frames->adoptPreparedFrame(internallyCollidedFrame);
            }
        }
        if (!internallyCollidedFrame && !scopedQueuedData)
            internallyCollidedFrame = frames->getFrameToTransmit();
        if (internallyCollidedFrame == nullptr)
            continue;
        auto internallyCollidedHeader = internallyCollidedFrame->peekAtFront<Ieee80211DataOrMgmtHeader>();
        EV_INFO << printAccessCategory(ac) << " (" << internallyCollidedFrame->getName() << ")" << endl;
        bool retryLimitReached = false;
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(internallyCollidedHeader)) { // TODO QoSDataFrame
            registerMsduRecovery(edcaf, internallyCollidedFrame);
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
            if (usesMsduRecovery(internallyCollidedFrame))
                disposeMsdu(edcaf, internallyCollidedFrame);
            else
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

void Hcf::recipientProcessReceivedFrame(Packet *packet, const Ptr<const Ieee80211MacHeader>& header, bool interframe,
        const BlockAckReceiveContext *context)
{
    EV_INFO << "Processing received frame " << packet->getName() << " as recipient.\n";
    emit(packetReceivedFromPeerSignal, packet);
    if (recipientBlockAckAgreementHandler) {
        // IEEE Std 802.11-2024, 11.5.4: valid Block Ack traffic without an agreement requires DELBA.
        // Reject malformed DATA and unsupported BAR fields before this recovery path.
        if (auto data = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
            auto agreement = data->getType() == ST_DATA_WITH_QOS ?
                recipientBlockAckAgreementHandler->getAgreement(data->getTid(), data->getTransmitterAddress()) : nullptr;
            if (agreement && (data->getTid() < 0 || data->getTid() > 15 || !data->getSequenceNumber().isSpecified())) {
                delete packet;
                return;
            }
            if (agreement && blockAckVariant == BlockAckVariant::COMPRESSED &&
                (data->getFragmentNumber() != 0 || data->getMoreFragments())) {
                delete packet;
                return;
            }
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
        else if (auto bar = dynamicPtrCast<const Ieee80211BlockAckReq>(header)) {
            if (!validateBlockAckRequest(packet, bar)) {
                delete packet;
                return;
            }
            auto basic = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(bar);
            auto compressed = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(bar);
            auto tid = basic ? basic->getTidInfo() : compressed->getTidInfo();
            auto agreement = recipientBlockAckAgreementHandler->getAgreement(tid, bar->getTransmitterAddress());
            if (!agreement) {
                beginTeardown(bar->getTransmitterAddress(), tid, false, 38);
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
        sendUp(recipientDataService->dataFrameReceived(packet, dataHeader, recipientBlockAckAgreementHandler, interframe, context));
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
    else if (auto blockAckRequest = dynamicPtrCast<const Ieee80211BlockAckReq>(header)) {
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
                agreement->setVariant(blockAckVariant);
            if (!previous && agreement)
                clearInterframeHistory();
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
    if (!header) {
        completedTransmissionWasAck = completedTransmissionWasBlockAck = false;
        auto owner = edca->getChannelOwner();
        auto prepared = frameSequenceHandler->getContext()->getQoSContext()->preparedData.get();
        auto step = check_and_cast<AmpduTransmitStep *>(frameSequenceHandler->getContext()->getLastStep());
        step->setPpduEnd(simTime());
        owner->getTxopProcedure()->recordTransmittedDuration(prepared->duration);
        frameSequenceHandler->transmissionComplete();
        return;
    }
    completedTransmissionWasAck = header->getType() == ST_ACK;
    completedTransmissionWasBlockAck = bool(dynamicPtrCast<const Ieee80211CompressedBlockAck>(header));
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
    auto context = frameSequenceHandler->getContext();
    auto prepared = context->getQoSContext()->preparedData.get();
    if (prepared && prepared->frame == packet && (prepared->preparation || prepared->batch)) {
        // IEEE Std 802.11-2024, 10.23.2.2 and 10.23.2.12.1: BA RTS failure updates QSRC/CW.
        // The uncommitted sources retain their lifetimes. No provisional sequence owns an SRC entry.
        if (validatePreparedData(*prepared, simTime()))
            edcaf->getRecoveryProcedure()->controlFrameTransmissionFailed();
        return;
    }
    if (obsoleteActions.count(packet->getId()) || !edcaf->getInProgressFrames()->containsFrame(packet))
        return;
    if (edcaf) {
        EV_INFO << "RTS frame transmission failed\n";
        bool retryLimitReached = false;
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(protectedHeader)) {
            if (edcaf->getRecoveryProcedure()->hasMsduRetryRecord(packet))
                edcaf->getRecoveryProcedure()->msduTransmissionFailed(packet, true);
            else
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
            if (usesMsduRecovery(packet))
                disposeMsdu(edcaf, packet);
            else
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
    auto prepared = frameSequenceHandler->getContext()->getQoSContext()->preparedData.get();
    if (prepared && prepared->batch && prepared->envelope.get() == packet) {
        if (prepared->cancelled)
            return;
        for (auto member : prepared->batch->members) {
            if (!edcaf->getInProgressFrames()->containsFrame(member))
                continue;
            edcaf->emit(packetSentToPeerSignal, member);
            edcaf->getAckHandler()->processTransmittedImplicitBlockAckMember(member->peekAtFront<Ieee80211DataHeader>());
        }
        return;
    }
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
    if (auto blockAckReq = dynamicPtrCast<const Ieee80211BlockAckReq>(controlHeader)) {
        auto basic = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(blockAckReq);
        auto compressed = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(blockAckReq);
        auto tid = basic ? basic->getTidInfo() : compressed->getTidInfo();
        auto agreement = originatorBlockAckAgreementHandler ?
            originatorBlockAckAgreementHandler->getAgreement(blockAckReq->getReceiverAddress(), tid) : nullptr;
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
    auto prepared = frameSequenceHandler->getContext()->getQoSContext()->preparedData.get();
    if (prepared && prepared->batch && prepared->envelope.get() == failedPacket) {
        processAggregateOutcome(nullptr);
        return;
    }
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
            if (usesMsduRecovery(failedPacket))
                disposeMsdu(edcaf, failedPacket);
            else
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
    auto prepared = frameSequenceHandler->getContext()->getQoSContext()->preparedData.get();
    if (prepared && prepared->batch && prepared->envelope.get() == lastTransmittedPacket) {
        processAggregateOutcome(receivedPacket->peekAtFront<Ieee80211CompressedBlockAck>());
        return;
    }
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
    else if (auto blockAck = dynamicPtrCast<const Ieee80211BlockAck>(header)) {
        auto basic = dynamicPtrCast<const Ieee80211BasicBlockAck>(blockAck);
        auto compressed = dynamicPtrCast<const Ieee80211CompressedBlockAck>(blockAck);
        auto tid = basic ? basic->getTidInfo() : compressed->getTidInfo();
        auto agreement = originatorBlockAckAgreementHandler ?
            originatorBlockAckAgreementHandler->getAgreement(blockAck->getTransmitterAddress(), tid) : nullptr;
        if (!agreement)
            return;
        EV_INFO << "BlockAck has arrived" << std::endl;
        edcaf->getRecoveryProcedure()->blockAckFrameReceived();
        auto ackedSeqAndFragNums = compressed ? edcaf->getAckHandler()->processReceivedCompressedBlockAck(compressed, agreement) :
            edcaf->getAckHandler()->processReceivedBlockAck(blockAck);
        if (originatorBlockAckAgreementHandler)
            originatorBlockAckAgreementHandler->processReceivedBlockAck(blockAck, this);
        EV_TRACE << "It has acknowledged the following frames:" << std::endl;
        for (auto it : ackedSeqAndFragNums)
            EV_TRACE << "   sequenceNumber = " << it.second.second.getSequenceNumber() << ", fragmentNumber = " << (int)it.second.second.getFragmentNumber() << std::endl;
        for (int i = 0; i < edcaf->getInProgressFrames()->getLength(); i++) {
            auto member = edcaf->getInProgressFrames()->getFrames(i);
            auto data = dynamicPtrCast<const Ieee80211DataHeader>(member->peekAtFront<Ieee80211MacHeader>());
            if (data && edcaf->getAckHandler()->getQoSDataAckStatus(data) == QosAckHandler::Status::BLOCK_ACK_ARRIVED_ACKED) {
                edcaf->getRecoveryProcedure()->dropFrame(data);
                auto siblings = edcaf->getInProgressFrames()->getAllocationFrames(member);
                bool allAcked = std::all_of(siblings.begin(), siblings.end(), [edcaf](Packet *sibling) {
                    return edcaf->getAckHandler()->getQoSDataAckStatus(sibling->peekAtFront<Ieee80211DataHeader>()) ==
                        QosAckHandler::Status::BLOCK_ACK_ARRIVED_ACKED;
                });
                if (allAcked)
                    edcaf->getRecoveryProcedure()->retireMsdu(member);
            }
        }
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
    else if (dynamicPtrCast<const Ieee80211BlockAckReq>(header))
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
            throw cRuntimeError("Multiple protection duration has an empty raw interval: reservation=%s, remaining=%s, PPDU=%s",
                reservation.str().c_str(), remaining.str().c_str(), ppduDuration.str().c_str());
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
    if (auto bar = dynamicPtrCast<const Ieee80211BlockAckReq>(header))
        return ppduDuration + sifs + rateSelection->computeResponseBlockAckFrameMode(packet, bar, ResponseQueryRole::PREDICT)->getDuration(
            dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(bar) ? LENGTH_COMPRESSED_BLOCKACK : LENGTH_BASIC_BLOCKACK);
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

bool Hcf::isInitialDataException(const FrameSequenceContext *context, const PreparedDataTransmission& prepared) const
{
    if (prepared.batch && (prepared.batch->members.size() != 1 || !prepared.batch->queued.empty()))
        return false;
    auto header = prepared.frame->peekAtFront<Ieee80211DataHeader>();
    if (!prepared.agreement) {
        auto frames = context->getInProgressFrames();
        bool permitted = frames->isSameSizeDataRetry(prepared.frame) ||
            frames->hasRetriedPreviousFragment(prepared.frame) || frames->getFragmentCount(prepared.frame) == 16 ||
            (prepared.fragments && prepared.fragments->fragments.size() == 16);
        if (!permitted)
            return false;
    }
    // IEEE Std 802.11-2024, 10.23.2.9: the same-size single-MPDU retry exception also permits an A-MSDU.
    // Committed Compressed MPDUs retain their payload and length throughout retry; this path forms no A-MPDU.
    bool committedRetry = !prepared.agreement || (!prepared.preparation && header->getRetry());
    if (prepared.batch && !committedRetry)
        return false;
    if (!committedRetry && (header->getAMsduPresent() || header->getRetry()))
        return false;
    for (int i = 0; i < context->getNumSteps(); i++) {
        auto step = dynamic_cast<TransmitStep *>(context->getStep(i));
        if (step && step->getPpduEnd() >= SIMTIME_ZERO && (dynamic_cast<AmpduTransmitStep *>(step) ||
            dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(step->getFrameToTransmit()->peekAtFront<Ieee80211MacHeader>())))
            return false;
    }
    return true;
}

simtime_t Hcf::computePreparedDataCost(const PreparedDataTransmission& prepared, bool protect) const
{
    auto sifs = modeSet->getSifsTime();
    auto cost = prepared.dataMode->getDuration(prepared.batch ? prepared.envelope->getDataLength() : prepared.frame->getDataLength());
    if (prepared.ackMode)
        cost += sifs + prepared.ackMode->getDuration(prepared.batch ? LENGTH_COMPRESSED_BLOCKACK : LENGTH_ACK);
    if (protect)
        cost += prepared.rtsMode->getDuration(prepared.rtsPrediction->getDataLength()) +
            sifs + prepared.ctsMode->getDuration(LENGTH_CTS) + sifs;
    return cost;
}

bool Hcf::canAdmitPreparedData(const FrameSequenceContext *context, const PreparedDataTransmission& prepared,
        simtime_t ifs, bool protect, simtime_t elapsedIfs) const
{
    auto txop = context->getQoSContext()->txopProcedure;
    auto cost = computePreparedDataCost(prepared, protect);
    int previousData = 0;
    for (int i = 0; i < context->getNumSteps(); i++) {
        auto step = dynamic_cast<TransmitStep *>(context->getStep(i));
        if (step && step->getPpduEnd() >= SIMTIME_ZERO && (dynamic_cast<AmpduTransmitStep *>(step) ||
            dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(step->getFrameToTransmit()->peekAtFront<Ieee80211MacHeader>())))
            previousData++;
    }
    // IEEE Std 802.11-2024, 10.23.2.9: zero permits one SU DATA PPDU, including an A-MSDU.
    if (txop->getLimit() == 0) {
        if (previousData != 0) {
            auto allocation = prepared.frame->findTag<Ieee80211DataSequenceTag>();
            if ((prepared.agreement && blockAckVariant != BlockAckVariant::BASIC) || !allocation ||
                    context->getInProgressFrames()->getFragmentCount(prepared.frame) == 1)
                return false;
            for (int i = 0; i < context->getNumSteps(); i++) {
                auto step = dynamic_cast<TransmitStep *>(context->getStep(i));
                if (step && step->getPpduEnd() >= SIMTIME_ZERO &&
                        dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(step->getFrameToTransmit()->peekAtFront<Ieee80211MacHeader>())) {
                    auto other = step->getFrameToTransmit()->findTag<Ieee80211DataSequenceTag>();
                    if (!other || other->getAllocationIndex() != allocation->getAllocationIndex())
                        return false;
                }
            }
        }
    }
    else {
        bool initialOverrun = ifs + cost > txop->getRemaining() && isInitialDataException(context, prepared);
        if (ifs + cost > txop->getRemaining() && !initialOverrun)
            return false;
        // IEEE Std 802.11-2024, 9.2.5.2(b)(4): a rounded reservation can exceed the TXOP deadline.
        if (txop->getProtectionMechanism() == TxopProcedure::MULTIPLE_PROTECTION &&
            txop->getReservationEnd() > txop->getStart() + txop->getLimit() && !initialOverrun)
            return false;
    }
    bool later = context->getNumSteps() > 0;
    // The current TransmitStep is already present at the final submission check.
    if (later) {
        auto last = dynamic_cast<TransmitStep *>(context->getLastStep());
        later = !last || last->getPpduEnd() >= SIMTIME_ZERO || context->getNumSteps() > 1;
    }
    // 10.23.2.8 samples TXNAV at exchange completion, before the leading IFS.
    if (later && txop->getProtectionMechanism() == TxopProcedure::SINGLE_PROTECTION && cost >= rx->getTxnavRemaining() + elapsedIfs)
        return false;
    if (later && protect && txop->getLimit() > 0 && txop->getProtectionMechanism() == TxopProcedure::MULTIPLE_PROTECTION &&
        (simTime() + ifs - txop->getStart() - txop->getLimit()).raw() % SimTime(1, SIMTIME_US).raw() != 0)
        return false;
    return true;
}

bool Hcf::validatePreparedData(const PreparedDataTransmission& prepared, simtime_t start) const
{
    if (prepared.cancelled || prepared.modeSet != modeSet)
        return false;
    auto header = prepared.frame->peekAtFront<Ieee80211DataHeader>();
    auto agreement = originatorBlockAckAgreementHandler ?
        originatorBlockAckAgreementHandler->getAgreement(header->getReceiverAddress(), header->getTid()) : nullptr;
    if (agreement && !agreement->getIsAddbaResponseReceived())
        agreement = nullptr;
    if (agreement != prepared.agreement || (agreement && !isProfileAllowed(header->getReceiverAddress(), true)))
        return false;
    auto service = check_and_cast<IOriginatorQosMacDataService *>(originatorDataService);
    if (prepared.batch) {
        if (!service->isBatchValid(*prepared.batch, start))
            return false;
        auto frames = edca->getChannelOwner()->getInProgressFrames();
        for (auto member : prepared.batch->members) {
            bool queued = false;
            for (const auto& preview : prepared.batch->queued)
                queued |= preview->packet.get() == member;
            if (!queued && (!frames->containsFrame(member) || !isFrameEligible(member, true) ||
                    edca->getLifetimeHandler()->isExpired(member, start)))
                return false;
            if (!agreement->containsSequence(member->peekAtFront<Ieee80211DataHeader>()->getSequenceNumber()))
                return false;
        }
        auto peer = mib->findPeerHtState(header->getReceiverAddress());
        if (!peer || !isPeerCompatibleMode(modeSet, peer, prepared.dataMode) ||
            (prepared.rtsMode && !isPeerCompatibleMode(modeSet, peer, prepared.rtsMode)) ||
            (prepared.ctsMode && rateSelection->computeResponseCtsFrameMode(prepared.rtsPrediction.get(),
                prepared.rtsPrediction->peekAtFront<Ieee80211RtsFrame>(), ResponseQueryRole::PREDICT) != prepared.ctsMode))
            return false;
        const auto& caps = peer->negotiatedCapabilities.localTxPeerRx;
        return caps.valid && prepared.limits.maxLength == B((1 << (13 + caps.receiverMaxAmpduLengthExponent)) - 1) &&
            prepared.limits.minimumMpduStartSpacing == caps.receiverMinimumMpduStartSpacing &&
            rateSelection->computeCompressedBlockAckResponseMode(prepared.dataMode) == prepared.ackMode;
    }
    if (prepared.fragments) {
        if (!service->isFragmentPreparationValid(*prepared.fragments, findFragmentSource(*prepared.fragments), start))
            return false;
    }
    else if (prepared.preparation) {
        if (!service->isPreparationValid(*prepared.preparation, start))
            return false;
    }
    else if (!edca->getChannelOwner()->getInProgressFrames()->containsFrame(prepared.frame) ||
        !isFrameEligible(prepared.frame, true) || edca->getLifetimeHandler()->isExpired(prepared.frame, start))
        return false;
    auto peer = mib->findPeerHtState(header->getReceiverAddress());
    if (!isPeerCompatibleMode(modeSet, peer, prepared.dataMode) ||
        (prepared.rtsMode && !isPeerCompatibleMode(modeSet, peer, prepared.rtsMode)))
        return false;
    if (originatorAckPolicy->computeAckPolicy(prepared.frame, header, agreement) != header->getAckPolicy())
        return false;
    if (prepared.ackMode && (!modeSet->containsMode(prepared.ackMode) ||
        rateSelection->computeResponseAckFrameMode(prepared.frame, header, ResponseQueryRole::PREDICT) != prepared.ackMode))
        return false;
    if (prepared.ctsMode && (!modeSet->containsMode(prepared.ctsMode) ||
        rateSelection->computeResponseCtsFrameMode(prepared.rtsPrediction.get(),
            prepared.rtsPrediction->peekAtFront<Ieee80211RtsFrame>(), ResponseQueryRole::PREDICT) != prepared.ctsMode))
        return false;
    return true;
}

bool Hcf::usesModernMsduRecovery(Packet *packet) const
{
    if (originatorBlockAckAgreementHandler && blockAckVariant == BlockAckVariant::BASIC)
        return false;
    auto header = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
    if (!header || header->getType() != ST_DATA_WITH_QOS || header->getReceiverAddress().isMulticast())
        return false;
    auto agreement = originatorBlockAckAgreementHandler ?
        originatorBlockAckAgreementHandler->getAgreement(header->getReceiverAddress(), header->getTid()) : nullptr;
    return !agreement || !agreement->getIsAddbaResponseReceived();
}

bool Hcf::usesMsduRecovery(Packet *packet) const
{
    auto header = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
    return header && header->getType() == ST_DATA_WITH_QOS && !header->getReceiverAddress().isMulticast() &&
        ((originatorBlockAckAgreementHandler && blockAckVariant == BlockAckVariant::BASIC) || usesModernMsduRecovery(packet));
}

void Hcf::registerMsduRecovery(Edcaf *edcaf, Packet *packet)
{
    if (!usesMsduRecovery(packet))
        return;
    auto header = packet->peekAtFront<Ieee80211DataHeader>();
    bool basic = originatorBlockAckAgreementHandler && blockAckVariant == BlockAckVariant::BASIC;
    auto agreement = basic ? originatorBlockAckAgreementHandler->getAgreement(header->getReceiverAddress(), header->getTid()) : nullptr;
    edcaf->getRecoveryProcedure()->registerMsdu(packet, basic ? QosRecoveryProcedure::MsduRetryContract::BASIC_2016 :
        QosRecoveryProcedure::MsduRetryContract::IEEE80211_2024, agreement && agreement->getIsAddbaResponseReceived());
}

Packet *Hcf::findFragmentSource(const PreparedQosFragments& preparation) const
{
    auto frames = edca->getChannelOwner()->getInProgressFrames();
    for (int i = 0; i < frames->getLength(); i++)
        if (frames->getFrames(i)->getId() == preparation.retainedId)
            return frames->getFrames(i);
    return nullptr;
}

void Hcf::prepareDataModes(PreparedDataTransmission& prepared, TxopProcedure *txop,
        const IIeee80211Mode *&selectedRtsMode)
{
    auto packet = prepared.frame;
    auto header = packet->peekAtFront<Ieee80211DataHeader>();
    setFrameMode(packet, header, prepared.dataMode);
    prepared.ackMode = recipientAckPolicy->isAckNeeded(header) ?
        rateSelection->computeResponseAckFrameMode(packet, header, ResponseQueryRole::PREDICT) : nullptr;
    prepared.setRtsPrediction(nullptr);
    prepared.rtsMode = prepared.ctsMode = nullptr;
    bool fragmentContinuation = false;
    auto allocation = packet->findTag<Ieee80211DataSequenceTag>();
    auto context = frameSequenceHandler->getContext();
    if ((!prepared.agreement || blockAckVariant == BlockAckVariant::BASIC) && allocation && header->getFragmentNumber() > 0) {
        for (int i = 0; i < context->getNumSteps(); i++) {
            auto step = dynamic_cast<TransmitStep *>(context->getStep(i));
            auto other = step && step->getPpduEnd() >= SIMTIME_ZERO ?
                step->getFrameToTransmit()->findTag<Ieee80211DataSequenceTag>() : nullptr;
            fragmentContinuation |= other && other->getAllocationIndex() == allocation->getAllocationIndex();
        }
    }
    // IEEE Std 802.11-2024, 10.3.2.6: a fragment burst protects its first fragment with RTS/CTS.
    if (!fragmentContinuation && rtsPolicy->isRtsNeeded(packet, header)) {
        prepared.setRtsPrediction(new Packet("prepared-RTS", rtsProcedure->buildRtsFrame(header)));
        prepared.rtsPrediction->insertAtBack(makeShared<Ieee80211MacTrailer>());
        auto rts = prepared.rtsPrediction->peekAtFront<Ieee80211RtsFrame>();
        if (!selectedRtsMode)
            selectedRtsMode = rateSelection->computeMode(prepared.rtsPrediction.get(), rts, txop);
        prepared.rtsMode = selectedRtsMode;
        setFrameMode(prepared.rtsPrediction.get(), rts, selectedRtsMode);
        prepared.ctsMode = rateSelection->computeResponseCtsFrameMode(prepared.rtsPrediction.get(), rts, ResponseQueryRole::PREDICT);
    }
}

int Hcf::computeFreshMpduBound(PreparedDataTransmission& prepared, TxopProcedure *txop,
        const IIeee80211Mode *&selectedRtsMode)
{
    auto original = prepared.frame;
    auto header = original->peekAtFront<Ieee80211DataHeader>();
    int overhead = B(header->getChunkLength()).get() + 4;
    int low = overhead;
    int high = original->getByteLength();
    if (txop->getLimit() == 0)
        return high;
    // The policy still owns the partition. This search supplies only the complete-exchange airtime bound.
    while (low < high) {
        int size = low + (high - low + 1) / 2;
        Packet candidate("bounded-DATA", header);
        candidate.insertAtBack(makeShared<ByteCountChunk>(B(size - overhead)));
        candidate.insertAtBack(makeShared<Ieee80211MacTrailer>());
        prepared.frame = &candidate;
        prepareDataModes(prepared, txop, selectedRtsMode);
        if (computePreparedDataCost(prepared, prepared.rtsMode != nullptr) <= txop->getLimit())
            low = size;
        else
            high = size - 1;
    }
    prepared.frame = original;
    prepareDataModes(prepared, txop, selectedRtsMode);
    return low;
}

bool Hcf::commitPreparedData(const FrameSequenceContext *context, PreparedDataTransmission& prepared)
{
    auto service = check_and_cast<IOriginatorQosMacDataService *>(originatorDataService);
    auto owner = edca->getChannelOwner();
    auto frames = context->getInProgressFrames();
    if (prepared.fragments) {
        auto source = findFragmentSource(*prepared.fragments);
        std::vector<Packet *> fragments;
        if (!service->commitPreparedFragments(*prepared.fragments, source, fragments))
            return false;
        ASSERT(fragments.front() == prepared.frame);
        if (source)
            frames->replaceWithPreparedFragments(source, fragments);
        else
            frames->adoptPreparedFragments(fragments);
        prepared.fragments.reset();
    }
    else if (prepared.preparation) {
        auto committed = service->commitPreparedFrame(*prepared.preparation);
        if (!committed)
            return false;
        ASSERT(committed == prepared.frame);
        frames->adoptPreparedFrame(committed);
        prepared.preparation.reset();
    }
    registerMsduRecovery(owner, prepared.frame);
    return true;
}

void Hcf::disposeMsdu(Edcaf *edcaf, Packet *packet)
{
    auto recovery = edcaf->getRecoveryProcedure();
    recovery->retireMsdu(packet);
    edcaf->getInProgressFrames()->dropAllocation(packet);
}

simtime_t Hcf::forecastPreparedSuccessor(const FrameSequenceContext *context,
        const PreparedDataTransmission& prepared, simtime_t ifs)
{
    auto txop = context->getQoSContext()->txopProcedure;
    if (txop->getLimit() == 0)
        return SIMTIME_ZERO;
    Packet *candidate = nullptr;
    for (auto frame : context->getInProgressFrames()->getEligibleFrames())
        if (frame != prepared.frame) {
            candidate = frame;
            break;
        }
    auto queue = edca->getChannelOwner()->getPendingQueue();
    if (!candidate) {
        for (int i = 0; i < queue->getNumPackets(); i++) {
            auto frame = queue->getPacket(i);
            if (prepared.preparation && std::find(prepared.preparation->sourceIds.begin(),
                    prepared.preparation->sourceIds.end(), frame->getId()) != prepared.preparation->sourceIds.end())
                continue;
            if (isFrameEligible(frame, false)) {
                candidate = frame;
                break;
            }
        }
    }
    if (!candidate || candidate->peekAtFront<Ieee80211MacHeader>()->getReceiverAddress().isMulticast())
        return SIMTIME_ZERO;
    std::unique_ptr<Packet> forecast(candidate->dup());
    auto header = forecast->peekAtFront<Ieee80211DataOrMgmtHeader>();
    if (!isPeerCompatibleMode(modeSet, mib->findPeerHtState(header->getReceiverAddress()), prepared.dataMode))
        return SIMTIME_ZERO;
    if (auto data = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
        auto policy = originatorAckPolicy->computeAckPolicy(forecast.get(), data, nullptr);
        auto updated = forecast->removeAtFront<Ieee80211DataHeader>();
        updated->setAckPolicy(policy);
        forecast->insertAtFront(updated);
        header = updated;
    }
    // Duration is an estimate. The next exchange selects its actual mode once at its own admission boundary.
    setFrameMode(forecast.get(), header, prepared.dataMode);
    auto sifs = modeSet->getSifsTime();
    auto cost = prepared.dataMode->getDuration(forecast->getDataLength());
    if (recipientAckPolicy->isAckNeeded(header))
        cost += sifs + rateSelection->computeResponseAckFrameMode(forecast.get(), header,
            ResponseQueryRole::PREDICT)->getDuration(LENGTH_ACK);
    auto reservation = cost;
    if (rtsPolicy->isRtsNeeded(forecast.get(), header)) {
        Packet rts("forecast-RTS", rtsProcedure->buildRtsFrame(header));
        rts.insertAtBack(makeShared<Ieee80211MacTrailer>());
        auto rtsHeader = rts.peekAtFront<Ieee80211RtsFrame>();
        auto mode = prepared.rtsMode ? prepared.rtsMode : rateSelection->computeMode(&rts, rtsHeader, txop);
        setFrameMode(&rts, rtsHeader, mode);
        reservation = mode->getDuration(rts.getDataLength()) + sifs +
            rateSelection->computeResponseCtsFrameMode(&rts, rtsHeader, ResponseQueryRole::PREDICT)->getDuration(LENGTH_CTS);
        cost += reservation + sifs;
    }
    return ifs + computePreparedDataCost(prepared, false) + sifs + cost <= txop->getRemaining() ? reservation : SIMTIME_ZERO;
}

DataPreparationResult Hcf::prepareDataTransmission(FrameSequenceContext *context)
{
    Enter_Method("prepareDataTransmission");
    auto frames = context->getInProgressFrames();
    auto candidate = frames->peekFrameToTransmit();
    auto header = candidate ? dynamicPtrCast<const Ieee80211DataHeader>(candidate->peekAtFront<Ieee80211MacHeader>()) : nullptr;
    if (!header || header->getType() != ST_DATA_WITH_QOS || header->getReceiverAddress().isMulticast())
        return DataPreparationResult::LEGACY;
    auto agreement = originatorBlockAckAgreementHandler ?
        originatorBlockAckAgreementHandler->getAgreement(header->getReceiverAddress(), header->getTid()) : nullptr;
    if (agreement && !agreement->getIsAddbaResponseReceived())
        agreement = nullptr;
    auto prepared = std::make_unique<PreparedDataTransmission>();
    prepared->modeSet = modeSet;
    prepared->agreement = agreement;
    if (agreement && blockAckVariant == BlockAckVariant::COMPRESSED && frames->containsFrame(candidate))
        edca->getChannelOwner()->getRecoveryProcedure()->retireMsdu(candidate);
    auto service = check_and_cast<IOriginatorQosMacDataService *>(originatorDataService);
    if (frames->containsFrame(candidate))
        prepared->frame = candidate;
    else {
        auto peer = mib->findPeerHtState(header->getReceiverAddress());
        // IEEE Std 802.11-2024, 10.11: Normal Ack preparation still requires peer HT capabilities for A-MSDU.
        bool aggregate = mib->isHtOperationSupported() && peer && peer->valid &&
            (!agreement || !service->hasAMpduAggregation());
        prepared->preparation = service->prepareFrame(edca->getChannelOwner()->getPendingQueue(), aggregate);
        if (!prepared->preparation)
            return DataPreparationResult::DEFERRED;
        prepared->frame = prepared->preparation->packet.get();
    }
    auto selected = prepared->frame->peekAtFront<Ieee80211DataHeader>();
    if (selected->getReceiverAddress() != header->getReceiverAddress() || selected->getTid() != header->getTid())
        return DataPreparationResult::DEFERRED;
    auto txop = context->getQoSContext()->txopProcedure;
    prepared->dataMode = rateSelection->computeMode(prepared->frame,
        prepared->frame->peekAtFront<Ieee80211DataHeader>(), txop);
    if (agreement && blockAckVariant == BlockAckVariant::COMPRESSED && service->hasAMpduAggregation() && prepareAggregateData(context, *prepared))
        return DataPreparationResult::READY;
    const IIeee80211Mode *selectedRtsMode = nullptr;
    bool fragmentsChecked = false;
    for (;;) {
        auto packet = prepared->frame;
        auto policy = originatorAckPolicy->computeAckPolicy(packet, packet->peekAtFront<Ieee80211DataHeader>(), agreement);
        auto updated = packet->removeAtFront<Ieee80211DataHeader>();
        bool retry = false;
        if (!prepared->preparation && !prepared->fragments) {
            auto status = edca->getChannelOwner()->getAckHandler()->getQoSDataAckStatus(updated);
            retry = status == QosAckHandler::Status::NORMAL_ACK_NOT_ARRIVED ||
                status == QosAckHandler::Status::BLOCK_ACK_NOT_ARRIVED || status == QosAckHandler::Status::BLOCK_ACK_ARRIVED_UNACKED;
        }
        // Forwarded MSDUs can carry the preceding hop's Retry bit. Local ACK state owns this hop's retry decision.
        updated->setRetry(retry);
        updated->setAckPolicy(policy);
        packet->insertAtFront(updated);
        prepareDataModes(*prepared, txop, selectedRtsMode);
        bool protect = prepared->rtsMode != nullptr;
        if ((!agreement || blockAckVariant == BlockAckVariant::BASIC) && !fragmentsChecked && !updated->getAMsduPresent() &&
                !frames->isSameSizeDataRetry(packet) && !updated->getMoreFragments() && updated->getFragmentNumber() == 0) {
            fragmentsChecked = true;
            int bound = agreement ?
                packet->getByteLength() : computeFreshMpduBound(*prepared, txop, selectedRtsMode);
            auto fragments = service->prepareFragments(edca->getChannelOwner()->getPendingQueue(),
                frames->containsFrame(candidate) ? candidate : nullptr, bound);
            if (fragments && fragments->status == FragmentPartitionStatus::IMPOSSIBLE) {
                auto configured = service->prepareFragments(edca->getChannelOwner()->getPendingQueue(),
                    frames->containsFrame(candidate) ? candidate : nullptr, packet->getByteLength());
                // IEEE Std 802.11-2024, 10.23.2.9: the sixteen-fragment exception can permit this partition.
                if (configured && configured->status == FragmentPartitionStatus::FRAGMENTED && configured->fragments.size() == 16)
                    fragments = std::move(configured);
            }
            if (fragments && fragments->status == FragmentPartitionStatus::FRAGMENTED) {
                prepared->fragments = std::move(fragments);
                prepared->preparation.reset();
                prepared->frame = prepared->fragments->fragments.front();
                continue;
            }
            if ((fragments && fragments->status == FragmentPartitionStatus::IMPOSSIBLE) ||
                    (!agreement && txop->getLimit() > 0 && computePreparedDataCost(*prepared, protect) > txop->getLimit()))
                throw cRuntimeError("TXOP admission: no permitted form fits peer %s TID %d mode %s limit %s (MPDU bound %d bytes)",
                    updated->getReceiverAddress().str().c_str(), int(updated->getTid()), prepared->dataMode->getName(),
                    txop->getLimit().str().c_str(), bound);
        }
        if (validatePreparedData(*prepared, simTime() + context->getIfs()) &&
            canAdmitPreparedData(context, *prepared, context->getIfs(), protect)) {
            context->getQoSContext()->preparedData = std::move(prepared);
            return DataPreparationResult::READY;
        }
        if (!prepared->preparation || !prepared->preparation->aggregate)
            return DataPreparationResult::DEFERRED;
        prepared->preparation = service->prepareFrame(edca->getChannelOwner()->getPendingQueue(), false);
        if (!prepared->preparation)
            return DataPreparationResult::DEFERRED;
        prepared->frame = prepared->preparation->packet.get();
    }
}

bool Hcf::prepareAggregateData(FrameSequenceContext *context, const PreparedDataTransmission& selected)
{
    auto ht = dynamic_cast<const Ieee80211HtMode *>(selected.dataMode);
    if (!ht || ht->getDataMode()->getBandwidth() != MHz(20) || ht->isHtShortGuardInterval() ||
        ht->getPreambleMode()->getPreambleFormat() != Ieee80211HtPreambleMode::HT_PREAMBLE_MIXED)
        return false;
    auto header = selected.frame->peekAtFront<Ieee80211DataHeader>();
    if (header->getAMsduPresent() || header->getMoreFragments() || header->getFragmentNumber() != 0)
        return false;
    auto peer = mib->findPeerHtState(header->getReceiverAddress());
    if (!peer || !peer->negotiatedCapabilities.localTxPeerRx.valid)
        return false;
    auto prepared = std::make_unique<PreparedDataTransmission>();
    prepared->modeSet = modeSet;
    prepared->agreement = selected.agreement;
    prepared->dataMode = selected.dataMode;
    auto& limits = prepared->limits;
    const auto& caps = peer->negotiatedCapabilities.localTxPeerRx;
    limits.receiver = header->getReceiverAddress();
    limits.transmitter = mac->getAddress();
    limits.tid = header->getTid();
    limits.windowStart = selected.agreement->getStartingSequenceNumber();
    limits.windowSize = selected.agreement->getWindowSize();
    limits.maxLength = B((1 << (13 + caps.receiverMaxAmpduLengthExponent)) - 1);
    limits.minimumMpduStartSpacing = caps.receiverMinimumMpduStartSpacing;
    limits.mode = selected.dataMode;
    auto owner = edca->getChannelOwner();
    std::vector<Packet *> retained;
    for (auto packet : context->getInProgressFrames()->getEligibleFrames()) {
        auto data = dynamicPtrCast<const Ieee80211DataHeader>(packet->peekAtFront<Ieee80211MacHeader>());
        if (data && data->getReceiverAddress() == limits.receiver && data->getTid() == limits.tid &&
            !data->getAMsduPresent() && !data->getMoreFragments() && data->getFragmentNumber() == 0 &&
            selected.agreement->containsSequence(data->getSequenceNumber()))
            retained.push_back(packet);
    }
    auto isRetry = [owner](Packet *packet) {
        return owner->getAckHandler()->getQoSDataAckStatus(packet->peekAtFront<Ieee80211DataHeader>()) !=
            QosAckHandler::Status::FRAME_NOT_YET_TRANSMITTED;
    };
    std::stable_sort(retained.begin(), retained.end(), [&](Packet *left, Packet *right) {
        if (isRetry(left) != isRetry(right))
            return isRetry(left);
        return OriginatorBlockAckAgreement::computeSequenceDistance(limits.windowStart,
            left->peekAtFront<Ieee80211DataHeader>()->getSequenceNumber()) <
            OriginatorBlockAckAgreement::computeSequenceDistance(limits.windowStart,
                right->peekAtFront<Ieee80211DataHeader>()->getSequenceNumber());
    });
    auto service = check_and_cast<IOriginatorQosMacDataService *>(originatorDataService);
    prepared->batch = service->prepareBatch(owner->getPendingQueue(), retained, limits);
    if (!prepared->batch)
        return false;
    auto& batch = *prepared->batch;
    const IIeee80211Mode *rtsMode = nullptr;
    while (!batch.members.empty()) {
        if (batch.members.size() == 1 && (!batch.queued.empty() || !isRetry(batch.members.front())))
            break;
        prepared->frame = batch.members.front();
        prepared->setEnvelope(service->buildAggregate(batch, limits));
        if (!prepared->envelope)
            break;
        prepared->ackMode = rateSelection->computeCompressedBlockAckResponseMode(prepared->dataMode);
        auto data = prepared->frame->peekAtFront<Ieee80211DataHeader>();
        bool protect = rtsPolicy->isRtsNeeded(prepared->envelope.get(), data);
        prepared->setRtsPrediction(nullptr);
        prepared->rtsMode = prepared->ctsMode = nullptr;
        if (protect) {
            prepared->setRtsPrediction(new Packet("prepared-RTS", rtsProcedure->buildRtsFrame(data)));
            prepared->rtsPrediction->insertAtBack(makeShared<Ieee80211MacTrailer>());
            auto rts = prepared->rtsPrediction->peekAtFront<Ieee80211RtsFrame>();
            if (!rtsMode)
                rtsMode = rateSelection->computeMode(prepared->rtsPrediction.get(), rts, context->getQoSContext()->txopProcedure);
            prepared->rtsMode = rtsMode;
            setFrameMode(prepared->rtsPrediction.get(), rts, rtsMode);
            prepared->ctsMode = rateSelection->computeResponseCtsFrameMode(prepared->rtsPrediction.get(), rts, ResponseQueryRole::PREDICT);
            auto legalControl = [](const IIeee80211Mode *mode) {
                return mode->getModulationClass() == IIeee80211Mode::ModulationClass::OFDM ||
                    mode->getModulationClass() == IIeee80211Mode::ModulationClass::ERP_OFDM;
            };
            if (!legalControl(rtsMode) || !legalControl(prepared->ctsMode))
                throw cRuntimeError("HT A-MPDU protection requires non-HT OFDM or ERP-OFDM RTS and CTS modes");
        }
        if (validatePreparedData(*prepared, simTime() + context->getIfs()) &&
            canAdmitPreparedData(context, *prepared, context->getIfs(), protect)) {
            context->getQoSContext()->preparedData = std::move(prepared);
            return true;
        }
        auto removed = batch.members.back();
        batch.members.pop_back();
        batch.queued.erase(std::remove_if(batch.queued.begin(), batch.queued.end(),
            [removed](const std::unique_ptr<PreparedQosData>& preview) { return preview->packet.get() == removed; }), batch.queued.end());
    }
    return false;
}

Packet *Hcf::prepareAggregateTransmission()
{
    Enter_Method("prepareAggregateTransmission");
    auto context = frameSequenceHandler->getContext();
    auto prepared = context->getQoSContext()->preparedData.get();
    if (!prepared || !prepared->batch || !validatePreparedData(*prepared, simTime()) ||
        !canAdmitPreparedData(context, *prepared, SIMTIME_ZERO, false,
            check_and_cast<AmpduTransmitStep *>(context->getLastStep())->getIfs()))
        return nullptr;
    auto owner = edca->getChannelOwner();
    auto service = check_and_cast<IOriginatorQosMacDataService *>(originatorDataService);
    std::vector<Packet *> committed;
    if (!service->commitPreparedBatch(*prepared->batch, committed))
        return nullptr;
    for (auto packet : committed)
        owner->getInProgressFrames()->adoptPreparedFrame(packet);
    auto txop = owner->getTxopProcedure();
    auto response = modeSet->getSifsTime() + prepared->ackMode->getDuration(LENGTH_COMPRESSED_BLOCKACK);
    if (txop->getProtectionMechanism() == TxopProcedure::SINGLE_PROTECTION)
        prepared->duration = encodeDuration(response, true);
    else {
        auto ppduDuration = prepared->dataMode->getDuration(prepared->envelope->getDataLength());
        auto reservation = std::max(SIMTIME_ZERO, txop->getReservationEnd() - simTime());
        auto remaining = std::max(SIMTIME_ZERO, txop->getStart() + txop->getLimit() - simTime());
        bool overrun = txop->getLimit() > 0 && ppduDuration + response > txop->getRemaining() &&
            isInitialDataException(context, *prepared);
        prepared->duration = overrun ? computeMultipleProtectionDuration(0, 0, 0, ppduDuration, ppduDuration + response, false) :
            computeMultipleProtectionDuration(txop->getLimit(), remaining, reservation, ppduDuration,
                txop->getLimit() == 0 ? ppduDuration + response : SIMTIME_ZERO, txop->getReservationEnd() != 0);
    }
    for (auto packet : prepared->batch->members) {
        auto data = packet->removeAtFront<Ieee80211DataHeader>();
        auto status = owner->getAckHandler()->getQoSDataAckStatus(data);
        data->setRetry(status != QosAckHandler::Status::FRAME_NOT_YET_TRANSMITTED);
        data->setAckPolicy(NORMAL_ACK);
        data->setDurationField(prepared->duration);
        packet->insertAtFront(data);
        tx->finalizeMpdu(packet);
    }
    prepared->setEnvelope(service->buildAggregate(*prepared->batch, prepared->limits));
    ASSERT(prepared->envelope);
    prepared->envelope->addTagIfAbsent<Ieee80211ModeReq>()->setMode(prepared->dataMode);
    prepared->submitted = true;
    for (auto packet : prepared->batch->members) {
        prepared->agreement->recordTransmittedData(packet->getTag<Ieee80211DataSequenceTag>()->getAllocationIndex());
        RateSelection::emitDatarateSelected(this, packet->peekAtFront<Ieee80211DataHeader>(), prepared->dataMode);
    }
    return prepared->envelope.get();
}

void Hcf::aggregateTransmissionCancelled()
{
    Enter_Method("aggregateTransmissionCancelled");
    frameSequenceHandler->transmissionCancelled();
    mac->sendDownPendingRadioConfigMsg();
}

void Hcf::processAggregateOutcome(const Ptr<const Ieee80211CompressedBlockAck>& response)
{
    auto context = frameSequenceHandler->getContext();
    auto prepared = context->getQoSContext()->preparedData.get();
    auto owner = edca->getChannelOwner();
    auto agreement = originatorBlockAckAgreementHandler->getAgreement(prepared->limits.receiver, prepared->limits.tid);
    if (prepared->cancelled || !prepared->submitted || !agreement || agreement != prepared->agreement)
        return;
    auto frames = owner->getInProgressFrames();
    std::set<std::pair<MacAddress, std::pair<Tid, SequenceControlField>>> acked;
    if (response) {
        owner->getRecoveryProcedure()->blockAckFrameReceived();
        acked = owner->getAckHandler()->processReceivedCompressedBlockAck(response, agreement);
        originatorBlockAckAgreementHandler->processReceivedBlockAck(response, this);
        auto step = check_and_cast<AmpduTransmitStep *>(context->getStepBeforeLast());
        rx->successfulFrameTransmitted(step->getPpduEnd(), prepared->duration);
    }
    else {
        owner->getRecoveryProcedure()->controlFrameTransmissionFailed();
        agreement->setPendingProbe(true);
    }
    for (auto packet : prepared->batch->members) {
        if (!frames->containsFrame(packet))
            continue;
        auto data = packet->peekAtFront<Ieee80211DataHeader>();
        bool acknowledged = response && owner->getAckHandler()->getQoSDataAckStatus(data) == QosAckHandler::Status::BLOCK_ACK_ARRIVED_ACKED;
        FrameTransmissionDetails details;
        details.setStatus(acknowledged ? FRAME_TRANSMISSION_STATUS_ACKNOWLEDGED : FRAME_TRANSMISSION_STATUS_UNACKNOWLEDGED);
        emit(Ieee80211Mac::frameTransmissionOutcomeSignal, packet, &details);
        if (!acknowledged) {
            owner->getRecoveryProcedure()->implicitBlockAckMemberFailed(packet, data);
            if (!response)
                owner->getAckHandler()->processFailedImplicitBlockAckMember(data);
            auto updated = packet->removeAtFront<Ieee80211DataHeader>();
            updated->setRetry(true);
            packet->insertAtFront(updated);
        }
    }
    for (int i = 0; i < frames->getLength(); i++) {
        auto data = dynamicPtrCast<const Ieee80211DataHeader>(frames->getFrames(i)->peekAtFront<Ieee80211MacHeader>());
        if (data && owner->getAckHandler()->getQoSDataAckStatus(data) == QosAckHandler::Status::BLOCK_ACK_ARRIVED_ACKED)
            owner->getRecoveryProcedure()->dropFrame(data);
    }
    frames->dropFrames(acked);
    owner->getAckHandler()->dropFrames(acked);
    advanceOriginatorWindows();
    completeTransfers();
}

void Hcf::frameTransmissionStarted(Packet *packet)
{
    Enter_Method("frameTransmissionStarted");
    auto bar = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(packet->peekAtFront<Ieee80211MacHeader>());
    if (bar && originatorBlockAckAgreementHandler) {
        auto agreement = originatorBlockAckAgreementHandler->getAgreement(bar->getReceiverAddress(), bar->getTidInfo());
        if (agreement)
            agreement->setPendingProbe(false);
    }
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
        auto context = frameSequenceHandler->getContext();
        auto prepared = context->getQoSContext()->preparedData.get();
        if (dynamic_cast<AmpduTransmitStep *>(context->getLastStep())) {
            if (!prepared || !validatePreparedData(*prepared, simTime() + ifs) ||
                !canAdmitPreparedData(context, *prepared, ifs, false))
                return false;
            tx->transmitAggregate(ifs, this);
            return true;
        }
        auto header = packet->peekAtFront<Ieee80211MacHeader>();
        auto rtsStep = dynamic_cast<RtsTransmitStep *>(context->getLastStep());
        bool preparedStep = prepared && (prepared->frame == packet ||
            (rtsStep && rtsStep->getProtectedFrame() == prepared->frame));
        if (preparedStep && (!validatePreparedData(*prepared, simTime() + ifs) ||
            !canAdmitPreparedData(context, *prepared, ifs, rtsStep != nullptr)))
            return false;
        if (dynamicPtrCast<const Ieee80211DataOrMgmtHeader>(header) &&
            !(preparedStep && (prepared->preparation || prepared->fragments || prepared->batch)) &&
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
        auto compressedBar = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(header);
        auto mode = preparedStep ? (rtsStep ? prepared->rtsMode : prepared->dataMode) :
            compressedBar ? rateSelection->computeCompressedBlockAckReqMode(packet, compressedBar, context->getNumSteps() == 1) :
            dataAfterCts ? packet->getTag<Ieee80211ModeReq>()->getMode() : rateSelection->computeMode(packet, header, txop);
        setFrameMode(packet, header, mode);
        if (auto bar = dynamicPtrCast<const Ieee80211BlockAckReq>(header)) {
            auto basic = dynamicPtrCast<const Ieee80211BasicBlockAckReq>(bar);
            auto compressed = dynamicPtrCast<const Ieee80211CompressedBlockAckReq>(bar);
            auto tid = basic ? basic->getTidInfo() : compressed->getTidInfo();
            auto agreement = originatorBlockAckAgreementHandler ?
                originatorBlockAckAgreementHandler->getAgreement(bar->getReceiverAddress(), tid) : nullptr;
            if (!agreement || !agreement->getIsAddbaResponseReceived())
                return false;
            auto response = rateSelection->computeResponseBlockAckFrameMode(packet, bar, ResponseQueryRole::PREDICT)->getDuration(
                compressed ? LENGTH_COMPRESSED_BLOCKACK : LENGTH_BASIC_BLOCKACK);
            auto cost = mode->getDuration(packet->getDataLength()) + modeSet->getSifsTime() + response;
            // IEEE Std 802.11-2024, 10.23.2.8 and 10.23.2.9: a later BAR must fit both bounds.
            if (context->getNumSteps() > 1 &&
                ((txop->getLimit() > 0 && ifs + cost > txop->getRemaining()) ||
                 (txop->getProtectionMechanism() == TxopProcedure::SINGLE_PROTECTION && cost >= rx->getTxnavRemaining())))
                return false;
        }
        if (preparedStep) {
            auto response = rtsStep ? prepared->ctsMode->getDuration(LENGTH_CTS) :
                prepared->ackMode ? prepared->ackMode->getDuration(LENGTH_ACK) : SIMTIME_ZERO;
            simtime_t successor = SIMTIME_ZERO;
            if (!rtsStep && (!prepared->agreement || blockAckVariant == BlockAckVariant::BASIC)) {
                auto dataHeader = prepared->frame->peekAtFront<Ieee80211DataHeader>();
                std::vector<Packet *> members = prepared->fragments ? prepared->fragments->fragments :
                    channelOwner->getInProgressFrames()->getAllocationFrames(prepared->frame);
                for (auto member : members) {
                    auto next = member->peekAtFront<Ieee80211DataHeader>();
                    if (next->getFragmentNumber() == dataHeader->getFragmentNumber() + 1) {
                        auto nextCost = prepared->dataMode->getDuration(member->getDataLength());
                        if (prepared->ackMode)
                            nextCost += modeSet->getSifsTime() + response;
                        if (txop->getLimit() == 0 || ifs + computePreparedDataCost(*prepared, false) +
                                modeSet->getSifsTime() + nextCost <= txop->getRemaining())
                            successor = nextCost;
                    }
                }
                if (successor == 0 && !dataHeader->getMoreFragments() && dataHeader->getFragmentNumber() == 0)
                    successor = forecastPreparedSuccessor(context, *prepared, ifs);
            }
            simtime_t duration;
            if (txop->getProtectionMechanism() == TxopProcedure::SINGLE_PROTECTION) {
                // Reserve this exchange only. A later BAR obtains contention through the existing arbitration path.
                duration = encodeDuration(singleProtectionMechanism->computeDurationField(header, response,
                    rtsStep ? computePreparedDataCost(*prepared, false) : successor, rtsStep || successor == 0), true);
            }
            else {
                auto ppduDuration = mode->getDuration(packet->getDataLength());
                auto projectedStart = simTime() + ifs;
                auto reservation = std::max(SIMTIME_ZERO, txop->getReservationEnd() - projectedStart);
                auto remaining = std::max(SIMTIME_ZERO, txop->getStart() + txop->getLimit() - projectedStart);
                auto cost = computePreparedDataCost(*prepared, rtsStep != nullptr);
                if (successor > 0)
                    cost += modeSet->getSifsTime() + successor;
                bool singleDataOverrun = txop->getLimit() > 0 && ifs + cost > txop->getRemaining() &&
                    isInitialDataException(context, *prepared);
                // An allowed single-DATA overrun protects its actual exchange beyond the nominal limit.
                duration = (singleDataOverrun || (txop->getLimit() == 0 && successor > 0)) ?
                    computeMultipleProtectionDuration(0, 0, 0, ppduDuration, cost, false) :
                    computeMultipleProtectionDuration(txop->getLimit(), remaining, reservation, ppduDuration,
                        txop->getLimit() == 0 ? cost : SIMTIME_ZERO, txop->getReservationEnd() != 0);
            }
            auto updated = packet->removeAtFront<Ieee80211MacHeader>();
            updated->setDurationField(duration);
            packet->insertAtFront(updated);
        }
        else if (scopedSingle) {
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
        if (preparedStep && (packet == prepared->frame || (rtsStep && (!prepared->agreement || blockAckVariant == BlockAckVariant::BASIC))) &&
                (prepared->preparation || prepared->fragments)) {
            if (!commitPreparedData(context, *prepared))
                return false;
            header = packet->peekAtFront<Ieee80211MacHeader>();
        }
        if (preparedStep && rtsStep)
            registerMsduRecovery(channelOwner, prepared->frame);
        RateSelection::emitDatarateSelected(this, header, mode);
        EV_DEBUG << "Datarate for " << packet->getName() << " is set to " << mode->getDataMode()->getNetBitrate() << ".\n";
        tx->transmitFrame(packet, packet->peekAtFront<Ieee80211MacHeader>(), ifs, false, this);
        if (auto dataHeader = dynamicPtrCast<const Ieee80211DataHeader>(header)) {
            if (usesMsduRecovery(packet)) {
                registerMsduRecovery(channelOwner, packet);
                channelOwner->getInProgressFrames()->recordDataTransmission(packet);
            }
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
    else if (auto blockAckReq = dynamicPtrCast<const Ieee80211BlockAckReq>(receivedHeader))
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
    clearInterframeHistory();
    if (frameSequenceHandler->isSequenceRunning() && !startRxTimer->isScheduled()) {
        frameSequenceHandler->handleStartRxTimeout();
    }
    else
        EV_DEBUG << "Ignoring received corrupt frame.\n";
}

Hcf::~Hcf()
{
    if (tx)
        tx->cancelPendingAggregate(this);
    if (interframeRadio) {
        auto radioModule = check_and_cast<cModule *>(interframeRadio.get());
        for (auto signal : {receptionStartedSignal, receptionEndedSignal, transmissionStartedSignal, transmissionEndedSignal, IRadio::receptionStateChangedSignal})
            radioModule->unsubscribe(signal, this);
    }
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
