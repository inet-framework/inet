//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#ifdef INET_WITH_ETHERNET
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#endif // ifdef INET_WITH_ETHERNET

#include "inet/linklayer/ieee80211/mac/contract/IFrameSequenceHandler.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h"
#include "inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211SubtypeTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAp.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211HtMgmtElements.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtTransactionTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211Radio.h"

namespace inet {

namespace ieee80211 {

using namespace physicallayer;

Define_Module(Ieee80211MgmtAp);
Register_Class(Ieee80211MgmtAp::NotificationInfoSta);

static std::ostream& operator<<(std::ostream& os, const Ieee80211MgmtAp::StaInfo& sta)
{
    os << "address:" << sta.address;
    return os;
}

const Packet *Ieee80211MgmtAp::getAssociationResponseFrame(ITransmitStep *transmitStep)
{
    if (transmitStep == nullptr)
        return nullptr;
    if (auto rtsTransmitStep = dynamic_cast<RtsTransmitStep *>(transmitStep))
        return rtsTransmitStep->getProtectedFrame();
    return transmitStep->getFrameToTransmit();
}

Ieee80211MgmtAp::AssociationResponseDisposition Ieee80211MgmtAp::getAssociationResponseDisposition(const Packet *responseFrame,
        uint64_t pendingTransactionId, bool exchangeSucceeded, bool retryPending)
{
    if (responseFrame == nullptr || pendingTransactionId == 0)
        return AssociationResponseDisposition::IGNORE;
    auto responseHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(responseFrame->peekAtFront<Ieee80211MacHeader>());
    if (responseHeader == nullptr || (responseHeader->getType() != ST_ASSOCIATIONRESPONSE && responseHeader->getType() != ST_REASSOCIATIONRESPONSE))
        return AssociationResponseDisposition::IGNORE;
    const auto& transactionTag = responseFrame->findTag<Ieee80211MgmtTransactionTag>();
    if (transactionTag == nullptr || transactionTag->getTransactionId() != pendingTransactionId)
        return AssociationResponseDisposition::IGNORE;
    if ((exchangeSucceeded && responseHeader->getMoreFragments()) || (!exchangeSucceeded && retryPending))
        return AssociationResponseDisposition::RETAIN;
    return AssociationResponseDisposition::COMPLETE;
}

Ieee80211MgmtAp::~Ieee80211MgmtAp()
{
    cancelAndDelete(beaconTimer);
    cancelAndDelete(associationResponseTimeoutTimer);
}

void Ieee80211MgmtAp::initialize(int stage)
{
    Ieee80211MgmtApBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read params and init vars
        ssid = par("ssid").stdstringValue();
        beaconInterval = par("beaconInterval");
        associationResponseTimeout = par("associationResponseTimeout");
        if (associationResponseTimeout < SIMTIME_ZERO)
            throw cRuntimeError("parameter 'associationResponseTimeout' must not be negative");
        numAuthSteps = par("numAuthSteps");
        if (numAuthSteps != 2 && numAuthSteps != 4)
            throw cRuntimeError("parameter 'numAuthSteps' (number of frames exchanged during authentication) must be 2 or 4, not %d", numAuthSteps);
        channelNumber = -1; // value will arrive from physical layer in receiveChangeNotification()
        WATCH(ssid);
        WATCH(channelNumber);
        WATCH(beaconInterval);
        WATCH(associationResponseTimeout);
        WATCH(numAuthSteps);
        WATCH(staList);

        // TODO fill in supportedRates

        // subscribe for notifications
        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(Ieee80211Radio::radioChannelChangedSignal, this);
        getContainingNicModule(this)->subscribe(IFrameSequenceHandler::frameSequenceFinishedSignal, this);

        // start beacon timer (randomize startup time)
        beaconTimer = new cMessage("beaconTimer");
        associationResponseTimeoutTimer = new cMessage("associationResponseTimeoutTimer");
    }
}

void Ieee80211MgmtAp::handleTimer(cMessage *msg)
{
    if (msg == beaconTimer) {
        sendBeacon();
        scheduleAfter(beaconInterval, beaconTimer);
    }
    else if (msg == associationResponseTimeoutTimer) {
        auto sta = staList.find(scheduledAssociationResponseTimeoutAddress);
        if (sta != staList.end() && isAssociationResponseTimeoutDue(sta->second,
                scheduledAssociationResponseTimeoutTransactionId, scheduledAssociationResponseTimeoutDeadline, simTime()))
            clearPendingAssociation(&sta->second);
        else
            scheduleAssociationResponseTimeout();
    }
    else {
        throw cRuntimeError("internal error: unrecognized timer '%s'", msg->getName());
    }
}

void Ieee80211MgmtAp::handleCommand(int msgkind, cObject *ctrl)
{
    throw cRuntimeError("handleCommand(): no commands supported");
}

void Ieee80211MgmtAp::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == Ieee80211Radio::radioChannelChangedSignal) {
        EV << "updating channel number\n";
        channelNumber = value;
        mib->htOperation.primaryChannel = channelNumber;
    }
}

void Ieee80211MgmtAp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == IFrameSequenceHandler::frameSequenceFinishedSignal) {
        auto context = check_and_cast<FrameSequenceContext *>(obj);
        if (context->getNumSteps() >= 2) {
            auto transmitStep = dynamic_cast<ITransmitStep *>(context->getStepBeforeLast());
            auto receiveStep = dynamic_cast<IReceiveStep *>(context->getLastStep());
            if (transmitStep && receiveStep) {
                const Packet *responseFrame = getAssociationResponseFrame(transmitStep);
                auto responseHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(responseFrame->peekAtFront<Ieee80211MacHeader>());
                if (responseHeader != nullptr && (responseHeader->getType() == ST_ASSOCIATIONRESPONSE || responseHeader->getType() == ST_REASSOCIATIONRESPONSE)) {
                    const auto& address = responseHeader->getReceiverAddress();
                    auto sta = staList.find(address);
                    bool exchangeSucceeded = transmitStep->getCompletion() == IFrameSequenceStep::Completion::ACCEPTED &&
                            receiveStep->getCompletion() == IFrameSequenceStep::Completion::ACCEPTED &&
                            receiveStep->getReceivedFrame()->peekAtFront<Ieee80211MacHeader>()->getType() == ST_ACK;
                    bool retryPending = false;
                    if (!exchangeSucceeded) {
                        auto inProgressFrames = context->getInProgressFrames();
                        for (int i = 0; i < inProgressFrames->getLength(); i++)
                            retryPending |= inProgressFrames->getFrames(i) == responseFrame;
                    }
                    uint64_t pendingTransactionId = sta == staList.end() ? 0 : sta->second.pendingAssociationTransactionId;
                    auto disposition = getAssociationResponseDisposition(responseFrame, pendingTransactionId, exchangeSucceeded, retryPending);
                    if (disposition == AssociationResponseDisposition::COMPLETE) {
                        if (exchangeSucceeded && sta->second.pendingAssociationSuccessful) {
                            bool wasAssociated = mib->bssAccessPointData.stations[address] == Ieee80211Mib::ASSOCIATED;
                            mib->commitAssociationId(address);
                            mib->bssAccessPointData.stations[address] = Ieee80211Mib::ASSOCIATED;
                            if (sta->second.pendingHtStateAvailable) {
                                // IEEE Std 802.11-2024, 11.3.5.3: association state becomes effective only after the successful response exchange.
                                if (sta->second.pendingHtCapabilitiesValid)
                                    mib->setPeerHtCapabilities(address, sta->second.pendingHtCapabilities, mib->htOperation);
                                else
                                    mib->removePeerHtCapabilities(address);
                            }
                            // Signal delivery is synchronous; observers must see committed station and peer state.
                            if (responseHeader->getType() == ST_ASSOCIATIONRESPONSE && !wasAssociated)
                                sendAssocNotification(address);
                        }
                        else if (exchangeSucceeded && mib->bssAccessPointData.stations[address] == Ieee80211Mib::ASSOCIATED) {
                            // This model does not implement negotiated management-frame protection.
                            // IEEE Std 802.11-2024, 11.3.5.3(p): a refused (re)association
                            // therefore moves the STA from State 4 back to State 3.
                            mib->releaseAssociationId(address);
                            mib->bssAccessPointData.stations[address] = Ieee80211Mib::AUTHENTICATED;
                            // Signal delivery is synchronous; observers must see the downgraded state.
                            sendDisAssocNotification(address);
                        }
                        clearPendingAssociation(&sta->second);
                    }
                }
            }
        }
    }
    else
        Ieee80211MgmtApBase::receiveSignal(source, signalID, obj, details);
}

Ieee80211MgmtAp::StaInfo *Ieee80211MgmtAp::lookupSenderSTA(const Ptr<const Ieee80211MgmtHeader>& header)
{
    auto it = staList.find(header->getTransmitterAddress());
    return it == staList.end() ? nullptr : &(it->second);
}

void Ieee80211MgmtAp::sendManagementFrame(const char *name, const Ptr<Ieee80211MgmtFrame>& body, int subtype, const MacAddress& destAddr, uint64_t transactionId)
{
    auto packet = new Packet(name);
    packet->addTag<MacAddressReq>()->setDestAddress(destAddr);
    packet->addTag<Ieee80211SubtypeReq>()->setSubtype(subtype);
    packet->insertAtBack(body);
    if (transactionId != 0)
        packet->addTag<Ieee80211MgmtTransactionTag>()->setTransactionId(transactionId);
    sendDown(packet);
}

uint64_t Ieee80211MgmtAp::createAssociationTransactionId()
{
    if (++nextAssociationTransactionId == 0)
        ++nextAssociationTransactionId;
    return nextAssociationTransactionId;
}

bool Ieee80211MgmtAp::isAssociationResponseTimeoutDue(const StaInfo& sta, uint64_t transactionId, simtime_t deadline, simtime_t currentTime)
{
    return transactionId != 0 && sta.pendingAssociationTransactionId == transactionId &&
            sta.pendingAssociationDeadline == deadline && deadline <= currentTime;
}

void Ieee80211MgmtAp::clearPendingAssociation(StaInfo *sta)
{
    mib->cancelAssociationIdReservation(sta->address);
    sta->pendingAssociationSuccessful = false;
    sta->pendingAssociationTransactionId = 0;
    sta->pendingAssociationDeadline = SIMTIME_MAX;
    sta->pendingHtStateAvailable = false;
    sta->pendingHtCapabilitiesValid = false;
    sta->pendingHtCapabilities = Ieee80211HtCapabilities();
    scheduleAssociationResponseTimeout();
}

void Ieee80211MgmtAp::scheduleAssociationResponseTimeout()
{
    cancelEvent(associationResponseTimeoutTimer);
    scheduledAssociationResponseTimeoutTransactionId = 0;
    scheduledAssociationResponseTimeoutDeadline = SIMTIME_MAX;
    for (const auto& entry : staList) {
        const auto& sta = entry.second;
        if (sta.pendingAssociationTransactionId != 0 && sta.pendingAssociationDeadline < scheduledAssociationResponseTimeoutDeadline) {
            scheduledAssociationResponseTimeoutAddress = entry.first;
            scheduledAssociationResponseTimeoutTransactionId = sta.pendingAssociationTransactionId;
            scheduledAssociationResponseTimeoutDeadline = sta.pendingAssociationDeadline;
        }
    }
    if (scheduledAssociationResponseTimeoutTransactionId != 0)
        scheduleAt(scheduledAssociationResponseTimeoutDeadline, associationResponseTimeoutTimer);
}

void Ieee80211MgmtAp::startAssociationResponseTimeout(StaInfo *sta)
{
    ASSERT(sta->pendingAssociationTransactionId != 0);
    sta->pendingAssociationDeadline = simTime() + associationResponseTimeout;
    scheduleAssociationResponseTimeout();
}

void Ieee80211MgmtAp::sendBeacon()
{
    EV << "Sending beacon\n";
    const auto& body = makeShared<Ieee80211BeaconFrame>();
    body->setSSID(ssid.c_str());
    body->setSupportedRates(supportedRates);
    body->setBeaconInterval(beaconInterval);
    body->setChannelNumber(channelNumber);
    addHtCapabilities(body);
    addHtOperation(body);
    body->setChunkLength(B(8 + 2 + 2 + (2 + ssid.length()) + (2 + supportedRates.numRates)) + getHtMgmtElementsLength(body));
    sendManagementFrame("Beacon", body, ST_BEACON, MacAddress::BROADCAST_ADDRESS);
}

void Ieee80211MgmtAp::handleAuthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    const auto& requestBody = packet->peekData<Ieee80211AuthenticationFrame>();
    int frameAuthSeq = requestBody->getSequenceNumber();
    EV << "Processing Authentication frame, seqNum=" << frameAuthSeq << "\n";

    // create STA entry if needed
    StaInfo *sta = lookupSenderSTA(header);
    if (!sta) {
        MacAddress staAddress = header->getTransmitterAddress();
        sta = &staList[staAddress]; // this implicitly creates a new entry
        sta->address = staAddress;
        mib->bssAccessPointData.stations[staAddress] = Ieee80211Mib::NOT_AUTHENTICATED;
        sta->authSeqExpected = 1;
    }
    clearPendingAssociation(sta);

    // reset authentication status, when starting a new auth sequence
    // The statements below are added because the L2 handover time was greater than before when
    // a STA wants to re-connect to an AP with which it was associated before. When the STA wants to
    // associate again with the previous AP, then since the AP is already having an entry of the STA
    // because of old association, and thus it is expecting an authentication frame number 3 but it
    // receives authentication frame number 1 from STA, which will cause the AP to return an Auth-Error
    // making the MN STA to start the handover process all over again.
    if (frameAuthSeq == 1) {
        if (mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::ASSOCIATED) {
            sendDisAssocNotification(sta->address);
            mib->releaseAssociationId(sta->address);
        }
        mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::NOT_AUTHENTICATED;
        mib->removePeerHtCapabilities(sta->address);
        sta->authSeqExpected = 1;
    }

    // check authentication sequence number is OK
    if (frameAuthSeq != sta->authSeqExpected) {
        // wrong sequence number: send error and return
        EV << "Wrong sequence number, " << sta->authSeqExpected << " expected\n";
        const auto& body = makeShared<Ieee80211AuthenticationFrame>();
        body->setStatusCode(SC_AUTH_OUT_OF_SEQ);
        sendManagementFrame("Auth-ERROR", body, ST_AUTHENTICATION, header->getTransmitterAddress());
        delete packet;
        sta->authSeqExpected = 1; // go back to start square
        return;
    }

    // station is authenticated if it made it through the required number of steps
    bool isLast = (frameAuthSeq + 1 == numAuthSteps);

    // send OK response (we don't model the cryptography part, just assume
    // successful authentication every time)
    EV << "Sending Authentication frame, seqNum=" << (frameAuthSeq + 1) << "\n";
    const auto& body = makeShared<Ieee80211AuthenticationFrame>();
    body->setSequenceNumber(frameAuthSeq + 1);
    body->setStatusCode(SC_SUCCESSFUL);
    body->setIsLast(isLast);
    // TODO frame length could be increased to account for challenge text length etc.
    sendManagementFrame(isLast ? "Auth-OK" : "Auth", body, ST_AUTHENTICATION, header->getTransmitterAddress());

    delete packet;

    // update status
    if (isLast) {
        if (mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::ASSOCIATED) {
            sendDisAssocNotification(sta->address);
            mib->releaseAssociationId(sta->address);
        }
        mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::AUTHENTICATED; // TODO only when ACK of this frame arrives
        EV << "STA authenticated\n";
    }
    else {
        sta->authSeqExpected += 2;
        EV << "Expecting Authentication frame " << sta->authSeqExpected << "\n";
    }
}

void Ieee80211MgmtAp::handleDeauthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Processing Deauthentication frame\n";

    StaInfo *sta = lookupSenderSTA(header);
    delete packet;

    if (sta) {
        // mark STA as not authenticated; alternatively, it could also be removed from staList
        if (mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::ASSOCIATED) {
            sendDisAssocNotification(sta->address);
            mib->releaseAssociationId(sta->address);
        }
        mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::NOT_AUTHENTICATED;
        sta->authSeqExpected = 1;
        clearPendingAssociation(sta);
        mib->removePeerHtCapabilities(sta->address);
    }
}

void Ieee80211MgmtAp::handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Processing AssociationRequest frame\n";

    // "11.3.2 AP association procedures"
    StaInfo *sta = lookupSenderSTA(header);
    if (sta != nullptr)
        clearPendingAssociation(sta);
    if (!sta || mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::NOT_AUTHENTICATED) {
        // STA not authenticated: send error and return
        const auto& body = makeShared<Ieee80211DeauthenticationFrame>();
        body->setReasonCode(RC_NONAUTH_ASS_REQUEST);
        sendManagementFrame("Deauth", body, ST_DEAUTHENTICATION, header->getTransmitterAddress());
        delete packet;
        return;
    }

    const auto& requestBody = packet->peekData<Ieee80211AssociationRequestFrame>();
    sta->pendingAssociationSuccessful = false;
    sta->pendingHtStateAvailable = true;
    sta->pendingHtCapabilitiesValid = mib->isHtOperationSupported() && requestBody->getHtCapabilitiesPresent();
    if (sta->pendingHtCapabilitiesValid)
        sta->pendingHtCapabilities = makeHtCapabilities(requestBody->getHtCapabilities());
    bool basicHtMcsSupported = !sta->pendingHtCapabilitiesValid ||
            supportsBasicHtMcsSet(sta->pendingHtCapabilities, mib->htOperation);
    delete packet;

    // IEEE Std 802.11-2024, 11.3.5.3 g): an HT STA must support every Basic HT-MCS.
    const auto& body = makeShared<Ieee80211AssociationResponseFrame>();
    body->setStatusCode(basicHtMcsSupported ? SC_SUCCESSFUL : SC_DATARATE_UNSUP);
    short associationId = basicHtMcsSupported ? mib->reserveAssociationId(sta->address) : 0;
    body->setAid(associationId);
    sta->pendingAssociationSuccessful = basicHtMcsSupported;
    sta->pendingAssociationTransactionId = createAssociationTransactionId();
    body->setSupportedRates(supportedRates);
    addHtCapabilities(body);
    addHtOperation(body);
    body->setChunkLength(B(2 + 2 + 2 + body->getSupportedRates().numRates + 2) + getHtMgmtElementsLength(body));
    startAssociationResponseTimeout(sta);
    sendManagementFrame(basicHtMcsSupported ? "AssocResp-OK" : "AssocResp-UnsupportedHtMcs", body, ST_ASSOCIATIONRESPONSE, sta->address, sta->pendingAssociationTransactionId);
}

void Ieee80211MgmtAp::handleAssociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAp::handleReassociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Processing ReassociationRequest frame\n";

    // "11.3.4 AP reassociation procedures" -- almost the same as AssociationRequest processing
    StaInfo *sta = lookupSenderSTA(header);
    if (sta != nullptr)
        clearPendingAssociation(sta);
    if (!sta || mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::NOT_AUTHENTICATED) {
        // STA not authenticated: send error and return
        const auto& body = makeShared<Ieee80211DeauthenticationFrame>();
        body->setReasonCode(RC_NONAUTH_ASS_REQUEST);
        sendManagementFrame("Deauth", body, ST_DEAUTHENTICATION, header->getTransmitterAddress());
        delete packet;
        return;
    }

    const auto& requestBody = packet->peekData<Ieee80211ReassociationRequestFrame>();
    sta->pendingAssociationSuccessful = false;
    sta->pendingHtStateAvailable = true;
    sta->pendingHtCapabilitiesValid = mib->isHtOperationSupported() && requestBody->getHtCapabilitiesPresent();
    if (sta->pendingHtCapabilitiesValid)
        sta->pendingHtCapabilities = makeHtCapabilities(requestBody->getHtCapabilities());
    bool basicHtMcsSupported = !sta->pendingHtCapabilitiesValid ||
            supportsBasicHtMcsSet(sta->pendingHtCapabilities, mib->htOperation);
    delete packet;

    // send OK response
    const auto& body = makeShared<Ieee80211ReassociationResponseFrame>();
    body->setStatusCode(basicHtMcsSupported ? SC_SUCCESSFUL : SC_DATARATE_UNSUP);
    short associationId = basicHtMcsSupported ? mib->reserveAssociationId(sta->address) : 0;
    body->setAid(associationId);
    sta->pendingAssociationSuccessful = basicHtMcsSupported;
    sta->pendingAssociationTransactionId = createAssociationTransactionId();
    body->setSupportedRates(supportedRates);
    addHtCapabilities(body);
    addHtOperation(body);
    body->setChunkLength(B(2 + 2 + 2 + (2 + supportedRates.numRates)) + getHtMgmtElementsLength(body));
    startAssociationResponseTimeout(sta);
    sendManagementFrame(basicHtMcsSupported ? "ReassocResp-OK" : "ReassocResp-UnsupportedHtMcs", body, ST_REASSOCIATIONRESPONSE, sta->address, sta->pendingAssociationTransactionId);
}

void Ieee80211MgmtAp::handleReassociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAp::handleDisassociationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    StaInfo *sta = lookupSenderSTA(header);
    delete packet;

    if (sta) {
        if (mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::ASSOCIATED) {
            sendDisAssocNotification(sta->address);
            mib->releaseAssociationId(sta->address);
        }
        mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::AUTHENTICATED;
        clearPendingAssociation(sta);
        mib->removePeerHtCapabilities(sta->address);
    }
}

void Ieee80211MgmtAp::handleBeaconFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAp::handleProbeRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Processing ProbeRequest frame\n";

    const auto& requestBody = packet->peekData<Ieee80211ProbeRequestFrame>();
    if (strcmp(requestBody->getSSID(), "") != 0 && strcmp(requestBody->getSSID(), ssid.c_str()) != 0) {
        EV << "SSID `" << requestBody->getSSID() << "' does not match, ignoring frame\n";
        dropManagementFrame(packet);
        return;
    }

    MacAddress staAddress = header->getTransmitterAddress();
    delete packet;

    EV << "Sending ProbeResponse frame\n";
    const auto& body = makeShared<Ieee80211ProbeResponseFrame>();
    body->setSSID(ssid.c_str());
    body->setSupportedRates(supportedRates);
    body->setBeaconInterval(beaconInterval);
    body->setChannelNumber(channelNumber);
    addHtCapabilities(body);
    addHtOperation(body);
    body->setChunkLength(B(8 + 2 + 2 + (2 + ssid.length()) + (2 + supportedRates.numRates)) + getHtMgmtElementsLength(body));
    sendManagementFrame("ProbeResp", body, ST_PROBERESPONSE, staAddress);
}

void Ieee80211MgmtAp::handleProbeResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAp::sendAssocNotification(const MacAddress& addr)
{
    NotificationInfoSta notif;
    notif.setApAddress(mib->address);
    notif.setStaAddress(addr);
    emit(l2ApAssociatedSignal, &notif);
}

void Ieee80211MgmtAp::sendDisAssocNotification(const MacAddress& addr)
{
    NotificationInfoSta notif;
    notif.setApAddress(mib->address);
    notif.setStaAddress(addr);
    emit(l2ApDisassociatedSignal, &notif);
}

void Ieee80211MgmtAp::start()
{
    Ieee80211MgmtApBase::start();
    scheduleAfter(uniform(0, beaconInterval), beaconTimer);
}

void Ieee80211MgmtAp::stop()
{
    cancelEvent(beaconTimer);
    cancelEvent(associationResponseTimeoutTimer);
    staList.clear();
    mib->clearAssociationIds();
    Ieee80211MgmtApBase::stop();
}

} // namespace ieee80211

} // namespace inet
