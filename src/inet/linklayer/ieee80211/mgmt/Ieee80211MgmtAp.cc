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
        channelNumber = -1; // value will arrive from physical layer in receiveChangeNotification()
        WATCH(ssid);
        WATCH(channelNumber);
        WATCH(beaconInterval);
        WATCH(numAuthSteps);
        WATCH_MAP(staList);

        // TODO fill in supportedRates

        // subscribe for notifications
        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(Ieee80211Radio::radioChannelChangedSignal, this);

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

void Ieee80211MgmtAp::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    if (signalID == Ieee80211Radio::radioChannelChangedSignal) {
        EV << "updating channel number\n";
        channelNumber = value;
    }
}

Ieee80211MgmtAp::StaInfo *Ieee80211MgmtAp::lookupSenderSTA(const Ptr<const Ieee80211MgmtHeader>& header)
{
    auto it = staList.find(header->getTransmitterAddress());
    return it == staList.end() ? nullptr : &(it->second);
}

void Ieee80211MgmtAp::sendManagementFrame(const char *name, const Ptr<Ieee80211MgmtFrame>& body, int subtype, const MacAddress& destAddr)
{
    auto packet = new Packet(name);
    packet->addTag<MacAddressReq>()->setDestAddress(destAddr);
    packet->addTag<Ieee80211SubtypeReq>()->setSubtype(subtype);
    packet->insertAtBack(body);
    sendDown(packet);
}

void Ieee80211MgmtAp::sendBeacon()
{
    EV << "Sending beacon\n";
    const auto& body = makeShared<Ieee80211BeaconFrame>();
    body->setSSID(ssid.c_str());
    body->setSupportedRates(supportedRates);
    body->setBeaconInterval(beaconInterval);
    body->setChannelNumber(channelNumber);
    body->setChunkLength(B(8 + 2 + 2 + (2 + ssid.length()) + (2 + supportedRates.numRates)));
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
        if (mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::ASSOCIATED)
            sendDisAssocNotification(sta->address);
        mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::NOT_AUTHENTICATED;
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
        if (mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::ASSOCIATED)
            sendDisAssocNotification(sta->address);
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
        if (mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::ASSOCIATED)
            sendDisAssocNotification(sta->address);
        mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::NOT_AUTHENTICATED;
        sta->authSeqExpected = 1;
    }
}

void Ieee80211MgmtAp::handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Processing AssociationRequest frame\n";

    // "11.3.2 AP association procedures"
    StaInfo *sta = lookupSenderSTA(header);
    if (!sta || mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::NOT_AUTHENTICATED) {
        // STA not authenticated: send error and return
        const auto& body = makeShared<Ieee80211DeauthenticationFrame>();
        body->setReasonCode(RC_NONAUTH_ASS_REQUEST);
        sendManagementFrame("Deauth", body, ST_DEAUTHENTICATION, header->getTransmitterAddress());
        delete packet;
        return;
    }

    delete packet;

    // mark STA as associated
    if (mib->bssAccessPointData.stations[sta->address] != Ieee80211Mib::ASSOCIATED)
        sendAssocNotification(sta->address);
    mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::ASSOCIATED; // TODO this should only take place when MAC receives the ACK for the response

    // send OK response
    const auto& body = makeShared<Ieee80211AssociationResponseFrame>();
    body->setStatusCode(SC_SUCCESSFUL);
    body->setAid(0); // TODO
    body->setSupportedRates(supportedRates);
    body->setChunkLength(B(2 + 2 + 2 + body->getSupportedRates().numRates + 2));
    sendManagementFrame("AssocResp-OK", body, ST_ASSOCIATIONRESPONSE, sta->address);
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
    if (!sta || mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::NOT_AUTHENTICATED) {
        // STA not authenticated: send error and return
        const auto& body = makeShared<Ieee80211DeauthenticationFrame>();
        body->setReasonCode(RC_NONAUTH_ASS_REQUEST);
        sendManagementFrame("Deauth", body, ST_DEAUTHENTICATION, header->getTransmitterAddress());
        delete packet;
        return;
    }

    delete packet;

    // mark STA as associated
    mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::ASSOCIATED; // TODO this should only take place when MAC receives the ACK for the response

    // send OK response
    const auto& body = makeShared<Ieee80211ReassociationResponseFrame>();
    body->setStatusCode(SC_SUCCESSFUL);
    body->setAid(0); // TODO
    body->setSupportedRates(supportedRates);
    body->setChunkLength(B(2 + (2 + ssid.length()) + (2 + supportedRates.numRates) + 6));
    sendManagementFrame("ReassocResp-OK", body, ST_REASSOCIATIONRESPONSE, sta->address);
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
        if (mib->bssAccessPointData.stations[sta->address] == Ieee80211Mib::ASSOCIATED)
            sendDisAssocNotification(sta->address);
        mib->bssAccessPointData.stations[sta->address] = Ieee80211Mib::AUTHENTICATED;
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
    body->setChunkLength(B(8 + 2 + 2 + (2 + ssid.length()) + (2 + supportedRates.numRates)));
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
    Ieee80211MgmtApBase::stop();
}

} // namespace ieee80211

} // namespace inet

