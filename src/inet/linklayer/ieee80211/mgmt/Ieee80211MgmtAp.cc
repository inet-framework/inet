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

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211SubtypeTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAp.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211HtMgmtElements.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtTransactionTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

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

Ieee80211MgmtAp::~Ieee80211MgmtAp()
{
    cancelAndDelete(beaconTimer);
}

void Ieee80211MgmtAp::initialize(int stage)
{
    Ieee80211MgmtApBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read params and init vars
        ssid = par("ssid").stdstringValue();
        beaconInterval = par("beaconInterval");
        numAuthSteps = par("numAuthSteps");
        if (numAuthSteps != 2 && numAuthSteps != 4)
            throw cRuntimeError("parameter 'numAuthSteps' (number of frames exchanged during authentication) must be 2 or 4, not %d", numAuthSteps);
        WATCH(ssid);
        WATCH(beaconInterval);
        WATCH(numAuthSteps);
        WATCH(staList);

        // TODO fill in supportedRates

        // start beacon timer (randomize startup time)
        beaconTimer = new cMessage("beaconTimer");
    }
}

void Ieee80211MgmtAp::handleTimer(cMessage *msg)
{
    if (msg == beaconTimer) {
        sendBeacon();
        scheduleAfter(beaconInterval, beaconTimer);
    }
    else {
        throw cRuntimeError("internal error: unrecognized timer '%s'", msg->getName());
    }
}

void Ieee80211MgmtAp::handleCommand(int msgkind, cObject *ctrl)
{
    throw cRuntimeError("handleCommand(): no commands supported");
}

Ieee80211MgmtAp::AssociationResponseDisposition Ieee80211MgmtAp::getAssociationResponseDisposition(const Packet *responseFrame,
        uint64_t pendingTransactionId, IFrameTransmissionCallback::Status status)
{
    if (responseFrame == nullptr || pendingTransactionId == 0)
        return AssociationResponseDisposition::IGNORE;
    const auto& transactionTag = responseFrame->findTag<Ieee80211MgmtTransactionTag>();
    if (transactionTag == nullptr || transactionTag->getTransactionId() != pendingTransactionId)
        return AssociationResponseDisposition::IGNORE;
    const auto& frontChunk = responseFrame->peekAtFront(b(-1), Chunk::PF_ALLOW_NULLPTR);
    const auto& responseHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(frontChunk);
    if (responseHeader == nullptr || (responseHeader->getType() != ST_ASSOCIATIONRESPONSE && responseHeader->getType() != ST_REASSOCIATIONRESPONSE))
        return AssociationResponseDisposition::IGNORE;
    if (status == IFrameTransmissionCallback::Status::ACKNOWLEDGED && responseHeader->getMoreFragments())
        return AssociationResponseDisposition::RETAIN;
    return AssociationResponseDisposition::COMPLETE;
}

void Ieee80211MgmtAp::frameTransmissionFinished(const IFrameTransmissionCallback::Result& result)
{
    Enter_Method("frameTransmissionFinished");
    const Packet *responseFrame = result.getFrame();
    if (responseFrame == nullptr)
        return;
    const auto& responseHeader = dynamicPtrCast<const Ieee80211MgmtHeader>(responseFrame->peekAtFront(b(-1), Chunk::PF_ALLOW_NULLPTR));
    if (responseHeader == nullptr || (responseHeader->getType() != ST_ASSOCIATIONRESPONSE && responseHeader->getType() != ST_REASSOCIATIONRESPONSE))
        return;
    auto address = responseHeader->getReceiverAddress();
    auto sta = staList.find(address);
    uint64_t pendingTransactionId = sta == staList.end() ? 0 : sta->second.pendingAssociationTransactionId;
    auto disposition = getAssociationResponseDisposition(responseFrame, pendingTransactionId, result.getStatus());
    if (disposition == AssociationResponseDisposition::IGNORE)
        return;
    ASSERT(sta != staList.end());
    if (disposition == AssociationResponseDisposition::RETAIN)
        return;

    if (result.getStatus() == IFrameTransmissionCallback::Status::ACKNOWLEDGED) {
        if (sta->second.pendingAssociationSuccessful) {
            bool wasAssociated = mib->bssAccessPointData.stations[address] == Ieee80211Mib::ASSOCIATED;
            mib->commitAssociationId(address);
            mib->bssAccessPointData.stations[address] = Ieee80211Mib::ASSOCIATED;
            if (sta->second.pendingHtStateAvailable) {
                // IEEE Std 802.11-2024, 11.3.5.3: association state becomes effective only after the successful response exchange.
                if (sta->second.pendingHtCapabilitiesValid) {
                    ASSERT(sta->second.pendingHtOperationValid);
                    mib->setPeerHtCapabilities(address, sta->second.pendingHtCapabilities, sta->second.pendingHtOperation);
                }
                else
                    mib->removePeerHtCapabilities(address);
            }
            // Signal delivery is synchronous; observers must see committed station and peer state.
            if (!wasAssociated)
                sendAssocNotification(address);
        }
        else if (mib->bssAccessPointData.stations[address] == Ieee80211Mib::ASSOCIATED) {
            // This model does not implement negotiated management-frame protection.
            // IEEE Std 802.11-2024, 11.3.5.3(p) for association and 11.3.5.5(n)
            // for same-AP reassociation therefore require the existing association
            // state to be cleared after this acknowledged refusal.
            mib->releaseAssociationId(address);
            mib->bssAccessPointData.stations[address] = Ieee80211Mib::AUTHENTICATED;
            // Signal delivery is synchronous; observers must see the downgraded state.
            sendDisAssocNotification(address);
        }
        clearPendingAssociation(&sta->second);
    }
    else
        clearPendingAssociation(&sta->second);
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

void Ieee80211MgmtAp::clearPendingAssociation(StaInfo *sta)
{
    mib->cancelAssociationIdReservation(sta->address);
    sta->pendingAssociationSuccessful = false;
    sta->pendingAssociationTransactionId = 0;
    sta->pendingHtStateAvailable = false;
    sta->pendingHtCapabilitiesValid = false;
    sta->pendingHtCapabilities = Ieee80211HtCapabilities();
    sta->pendingHtOperationValid = false;
    sta->pendingHtOperation = Ieee80211HtOperation();
}

void Ieee80211MgmtAp::sendBeacon()
{
    EV << "Sending beacon\n";
    int primaryChannel = mib->requirePrimaryChannel();
    const auto htOperation = mib->getHtOperation();
    const auto& body = makeShared<Ieee80211BeaconFrame>();
    body->setSSID(ssid.c_str());
    setSupportedRateElements(body);
    body->setBeaconInterval(beaconInterval);
    body->setChannelNumber(primaryChannel);
    addHtCapabilities(body);
    if (mib->isHtOperationSupported())
        setHtOperation(body, htOperation);
    body->setChunkLength(B(8 + 2 + 2 + (2 + ssid.length())) + getSupportedRateElementsLength(body) + getHtMgmtElementsLength(body));
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
    // reset authentication status, when starting a new auth sequence
    // The statements below are added because the L2 handover time was greater than before when
    // a STA wants to re-connect to an AP with which it was associated before. When the STA wants to
    // associate again with the previous AP, then since the AP is already having an entry of the STA
    // because of old association, and thus it is expecting an authentication frame number 3 but it
    // receives authentication frame number 1 from STA, which will cause the AP to return an Auth-Error
    // making the MN STA to start the handover process all over again.
    if (frameAuthSeq == 1) {
        // INET policy: an accepted new authentication sequence supersedes any
        // association response that is still owned by the MAC. IEEE 802.11-2024
        // 11.3.4 does not require an associated peer to downgrade on frame 1;
        // keep this cancellation scoped to this existing model transition.
        clearPendingAssociation(sta);
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
        clearPendingAssociation(sta);
        // mark STA as not authenticated; alternatively, it could also be removed from staList
        if (mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::ASSOCIATED) {
            sendDisAssocNotification(sta->address);
            mib->releaseAssociationId(sta->address);
        }
        mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::NOT_AUTHENTICATED;
        sta->authSeqExpected = 1;
        mib->removePeerHtCapabilities(sta->address);
    }
}

void Ieee80211MgmtAp::handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Processing AssociationRequest frame\n";

    // "11.3.2 AP association procedures"
    StaInfo *sta = lookupSenderSTA(header);
    if (sta != nullptr && sta->pendingAssociationTransactionId != 0) {
        // A response transaction is MAC-owned until its terminal completion.
        // Coalesce a retransmitted request instead of replacing its response
        // or allocating another association ID.
        delete packet;
        return;
    }
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
    bool pendingHtOperationValid = mib->isHtOperationSupported();
    Ieee80211HtOperation pendingHtOperation;
    if (pendingHtOperationValid)
        pendingHtOperation = mib->getHtOperation();
    bool pendingHtCapabilitiesValid = pendingHtOperationValid && requestBody->getHtCapabilitiesPresent();
    Ieee80211HtCapabilities pendingHtCapabilities;
    if (pendingHtCapabilitiesValid)
        pendingHtCapabilities = makeHtCapabilities(requestBody->getHtCapabilities());
    bool basicHtMcsSupported = !pendingHtCapabilitiesValid ||
            supportsBasicHtMcsSet(pendingHtCapabilities, pendingHtOperation);
    delete packet;

    // IEEE Std 802.11-2024, 11.3.5.3 g): an HT STA must support every Basic HT-MCS.
    const auto& body = makeShared<Ieee80211AssociationResponseFrame>();
    // Constructing an HT response requires an authoritative primary channel.
    // Do this before reserving an AID or publishing pending transaction state,
    // so an unavailable channel cannot leave a half-created association.
    if (pendingHtOperationValid)
        setHtOperation(body, pendingHtOperation);
    body->setStatusCode(basicHtMcsSupported ? SC_SUCCESSFUL : SC_DATARATE_UNSUP);
    short associationId = basicHtMcsSupported ? mib->reserveAssociationId(sta->address) : 0;
    body->setAid(associationId);
    sta->pendingAssociationSuccessful = basicHtMcsSupported;
    sta->pendingHtStateAvailable = true;
    sta->pendingHtCapabilitiesValid = pendingHtCapabilitiesValid;
    sta->pendingHtCapabilities = pendingHtCapabilities;
    sta->pendingHtOperationValid = pendingHtOperationValid;
    sta->pendingHtOperation = pendingHtOperation;
    sta->pendingAssociationTransactionId = createAssociationTransactionId();
    setSupportedRateElements(body);
    addHtCapabilities(body);
    body->setChunkLength(B(2 + 2 + 2) + getSupportedRateElementsLength(body) + getHtMgmtElementsLength(body));
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
    if (sta != nullptr && sta->pendingAssociationTransactionId != 0) {
        delete packet;
        return;
    }
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
    bool pendingHtOperationValid = mib->isHtOperationSupported();
    Ieee80211HtOperation pendingHtOperation;
    if (pendingHtOperationValid)
        pendingHtOperation = mib->getHtOperation();
    bool pendingHtCapabilitiesValid = pendingHtOperationValid && requestBody->getHtCapabilitiesPresent();
    Ieee80211HtCapabilities pendingHtCapabilities;
    if (pendingHtCapabilitiesValid)
        pendingHtCapabilities = makeHtCapabilities(requestBody->getHtCapabilities());
    bool basicHtMcsSupported = !pendingHtCapabilitiesValid ||
            supportsBasicHtMcsSet(pendingHtCapabilities, pendingHtOperation);
    delete packet;

    // send OK response
    const auto& body = makeShared<Ieee80211ReassociationResponseFrame>();
    // See the association response path above: fail while constructing the
    // response, before mutating association bookkeeping.
    if (pendingHtOperationValid)
        setHtOperation(body, pendingHtOperation);
    body->setStatusCode(basicHtMcsSupported ? SC_SUCCESSFUL : SC_DATARATE_UNSUP);
    short associationId = basicHtMcsSupported ? mib->reserveAssociationId(sta->address) : 0;
    body->setAid(associationId);
    sta->pendingAssociationSuccessful = basicHtMcsSupported;
    sta->pendingHtStateAvailable = true;
    sta->pendingHtCapabilitiesValid = pendingHtCapabilitiesValid;
    sta->pendingHtCapabilities = pendingHtCapabilities;
    sta->pendingHtOperationValid = pendingHtOperationValid;
    sta->pendingHtOperation = pendingHtOperation;
    sta->pendingAssociationTransactionId = createAssociationTransactionId();
    setSupportedRateElements(body);
    addHtCapabilities(body);
    body->setChunkLength(B(2 + 2 + 2) + getSupportedRateElementsLength(body) + getHtMgmtElementsLength(body));
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
        clearPendingAssociation(sta);
        if (mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::ASSOCIATED) {
            sendDisAssocNotification(sta->address);
            mib->releaseAssociationId(sta->address);
        }
        mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::AUTHENTICATED;
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
    int primaryChannel = mib->requirePrimaryChannel();
    const auto htOperation = mib->getHtOperation();
    const auto& body = makeShared<Ieee80211ProbeResponseFrame>();
    body->setSSID(ssid.c_str());
    setSupportedRateElements(body);
    body->setBeaconInterval(beaconInterval);
    body->setChannelNumber(primaryChannel);
    addHtCapabilities(body);
    if (mib->isHtOperationSupported())
        setHtOperation(body, htOperation);
    body->setChunkLength(B(8 + 2 + 2 + (2 + ssid.length())) + getSupportedRateElementsLength(body) + getHtMgmtElementsLength(body));
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
    staList.clear();
    mib->clearAssociationIds();
    Ieee80211MgmtApBase::stop();
}

} // namespace ieee80211

} // namespace inet
