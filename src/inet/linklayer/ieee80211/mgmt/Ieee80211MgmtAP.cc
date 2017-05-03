//
// Copyright (C) 2006 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAP.h"

#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#endif // ifdef WITH_ETHERNET

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Radio.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"

namespace inet {

namespace ieee80211 {

using namespace physicallayer;

Define_Module(Ieee80211MgmtAP);
Register_Class(Ieee80211MgmtAP::NotificationInfoSta);

static std::ostream& operator<<(std::ostream& os, const Ieee80211MgmtAP::STAInfo& sta)
{
    os << "state:" << sta.status;
    return os;
}

Ieee80211MgmtAP::~Ieee80211MgmtAP()
{
    cancelAndDelete(beaconTimer);
}

void Ieee80211MgmtAP::initialize(int stage)
{
    Ieee80211MgmtAPBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read params and init vars
        ssid = par("ssid").stringValue();
        beaconInterval = par("beaconInterval");
        numAuthSteps = par("numAuthSteps");
        if (numAuthSteps != 2 && numAuthSteps != 4)
            throw cRuntimeError("parameter 'numAuthSteps' (number of frames exchanged during authentication) must be 2 or 4, not %d", numAuthSteps);
        channelNumber = -1;    // value will arrive from physical layer in receiveChangeNotification()
        WATCH(ssid);
        WATCH(channelNumber);
        WATCH(beaconInterval);
        WATCH(numAuthSteps);
        WATCH_MAP(staList);

        //TBD fill in supportedRates

        // subscribe for notifications
        cModule *radioModule = getModuleFromPar<cModule>(par("radioModule"), this);
        radioModule->subscribe(Ieee80211Radio::radioChannelChangedSignal, this);

        // start beacon timer (randomize startup time)
        beaconTimer = new cMessage("beaconTimer");
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        if (isOperational)
            scheduleAt(simTime() + uniform(0, beaconInterval), beaconTimer);
    }
}

void Ieee80211MgmtAP::handleTimer(cMessage *msg)
{
    if (msg == beaconTimer) {
        sendBeacon();
        scheduleAt(simTime() + beaconInterval, beaconTimer);
    }
    else {
        throw cRuntimeError("internal error: unrecognized timer '%s'", msg->getName());
    }
}

void Ieee80211MgmtAP::handleUpperMessage(cPacket *msg)
{
    auto packet = check_and_cast<Packet *>(msg);
    encapsulate(packet);
    const auto& frame = packet->peekHeader<Ieee80211DataFrame>();
    MACAddress macAddr = frame->getReceiverAddress();
    if (!macAddr.isMulticast()) {
        auto it = staList.find(macAddr);
        if (it == staList.end() || it->second.status != ASSOCIATED) {
            EV << "STA with MAC address " << macAddr << " not associated with this AP, dropping frame\n";
            delete packet;    // XXX count drops?
            return;
        }
    }

    sendDown(packet);
}

void Ieee80211MgmtAP::handleCommand(int msgkind, cObject *ctrl)
{
    throw cRuntimeError("handleCommand(): no commands supported");
}

void Ieee80211MgmtAP::receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == Ieee80211Radio::radioChannelChangedSignal) {
        EV << "updating channel number\n";
        channelNumber = value;
    }
}

Ieee80211MgmtAP::STAInfo *Ieee80211MgmtAP::lookupSenderSTA(const Ptr<Ieee80211ManagementHeader>& frame)
{
    auto it = staList.find(frame->getTransmitterAddress());
    return it == staList.end() ? nullptr : &(it->second);
}

void Ieee80211MgmtAP::sendManagementFrame(const char *name, const Ptr<Ieee80211ManagementHeader>& frame, const Ptr<Ieee80211ManagementFrame>& body, const MACAddress& destAddr)
{
    frame->setReceiverAddress(destAddr);
    frame->setAddress3(myAddress);
    frame->markImmutable();
    auto packet = new Packet(name);
    body->markImmutable();
    packet->append(body);
    packet->insertHeader(frame);
    sendDown(packet);
}

void Ieee80211MgmtAP::sendBeacon()
{
    EV << "Sending beacon\n";
    const auto& frame = std::make_shared<Ieee80211ManagementHeader>();
    frame->setType(ST_BEACON);
    const auto& body = std::make_shared<Ieee80211BeaconFrame>();
    body->setSSID(ssid.c_str());
    body->setSupportedRates(supportedRates);
    body->setBeaconInterval(beaconInterval);
    body->setChannelNumber(channelNumber);
    body->setChunkLength(byte(8 + 2 + 2 + (2 + ssid.length()) + (2 + supportedRates.numRates)));
    frame->setChunkLength(byte(24));
    sendManagementFrame("Beacon", frame, body, MACAddress::BROADCAST_ADDRESS);
}

void Ieee80211MgmtAP::handleDataFrame(Packet *packet, const Ptr<Ieee80211DataFrame>& frame)
{
    // check toDS bit
    if (!frame->getToDS()) {
        // looks like this is not for us - discard
        EV << "Frame is not for us (toDS=false) -- discarding\n";
        delete packet;
        return;
    }

    // handle broadcast/multicast frames
    if (frame->getAddress3().isMulticast()) {
        EV << "Handling multicast frame\n";

        if (isConnectedToHL)
            sendToUpperLayer(packet->dup());

        distributeReceivedDataFrame(packet);
        return;
    }

    // look up destination address in our STA list
    auto it = staList.find(frame->getAddress3());
    if (it == staList.end()) {
        // not our STA -- pass up frame to relayUnit for LAN bridging if we have one
        if (isConnectedToHL) {
            sendToUpperLayer(packet);
        }
        else {
            EV << "Frame's destination address is not in our STA list -- dropping frame\n";
            delete packet;
        }
    }
    else {
        // dest address is our STA, but is it already associated?
        if (it->second.status == ASSOCIATED)
            distributeReceivedDataFrame(packet); // send it out to the destination STA
        else {
            EV << "Frame's destination STA is not in associated state -- dropping frame\n";
            delete packet;
        }
    }
}

void Ieee80211MgmtAP::handleAuthenticationFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    const auto& requestBody = packet->peekDataAt<Ieee80211AuthenticationFrame>(frame->getChunkLength());
    int frameAuthSeq = requestBody->getSequenceNumber();
    EV << "Processing Authentication frame, seqNum=" << frameAuthSeq << "\n";

    // create STA entry if needed
    STAInfo *sta = lookupSenderSTA(frame);
    if (!sta) {
        MACAddress staAddress = frame->getTransmitterAddress();
        sta = &staList[staAddress];    // this implicitly creates a new entry
        sta->address = staAddress;
        sta->status = NOT_AUTHENTICATED;
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
        if (sta->status == ASSOCIATED)
            sendDisAssocNotification(sta->address);
        sta->status = NOT_AUTHENTICATED;
        sta->authSeqExpected = 1;
    }

    // check authentication sequence number is OK
    if (frameAuthSeq != sta->authSeqExpected) {
        // wrong sequence number: send error and return
        EV << "Wrong sequence number, " << sta->authSeqExpected << " expected\n";
        const auto& resp = std::make_shared<Ieee80211ManagementHeader>();
        resp->setType(ST_AUTHENTICATION);
        const auto& body = std::make_shared<Ieee80211AuthenticationFrame>();
        body->setStatusCode(SC_AUTH_OUT_OF_SEQ);
        resp->setChunkLength(byte(24));
        sendManagementFrame("Auth-ERROR", resp, body, frame->getTransmitterAddress());
        delete packet;
        sta->authSeqExpected = 1;    // go back to start square
        return;
    }

    // station is authenticated if it made it through the required number of steps
    bool isLast = (frameAuthSeq + 1 == numAuthSteps);

    // send OK response (we don't model the cryptography part, just assume
    // successful authentication every time)
    EV << "Sending Authentication frame, seqNum=" << (frameAuthSeq + 1) << "\n";
    const auto& resp = std::make_shared<Ieee80211ManagementHeader>();
    resp->setType(ST_AUTHENTICATION);
    const auto& body = std::make_shared<Ieee80211AuthenticationFrame>();
    body->setSequenceNumber(frameAuthSeq + 1);
    body->setStatusCode(SC_SUCCESSFUL);
    body->setIsLast(isLast);
    // XXX frame length could be increased to account for challenge text length etc.
    resp->setChunkLength(byte(24));
    sendManagementFrame(isLast ? "Auth-OK" : "Auth", resp, body, frame->getTransmitterAddress());

    delete packet;

    // update status
    if (isLast) {
        if (sta->status == ASSOCIATED)
            sendDisAssocNotification(sta->address);
        sta->status = AUTHENTICATED;    // XXX only when ACK of this frame arrives
        EV << "STA authenticated\n";
    }
    else {
        sta->authSeqExpected += 2;
        EV << "Expecting Authentication frame " << sta->authSeqExpected << "\n";
    }
}

void Ieee80211MgmtAP::handleDeauthenticationFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    EV << "Processing Deauthentication frame\n";

    STAInfo *sta = lookupSenderSTA(frame);
    delete packet;

    if (sta) {
        // mark STA as not authenticated; alternatively, it could also be removed from staList
        if (sta->status == ASSOCIATED)
            sendDisAssocNotification(sta->address);
        sta->status = NOT_AUTHENTICATED;
        sta->authSeqExpected = 1;
    }
}

void Ieee80211MgmtAP::handleAssociationRequestFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    EV << "Processing AssociationRequest frame\n";

    // "11.3.2 AP association procedures"
    STAInfo *sta = lookupSenderSTA(frame);
    if (!sta || sta->status == NOT_AUTHENTICATED) {
        // STA not authenticated: send error and return
        const auto& resp = std::make_shared<Ieee80211ManagementHeader>();
        resp->setType(ST_DEAUTHENTICATION);
        const auto& body = std::make_shared<Ieee80211DeauthenticationFrame>();
        body->setReasonCode(RC_NONAUTH_ASS_REQUEST);
        resp->setChunkLength(byte(24));
        sendManagementFrame("Deauth", resp, body, frame->getTransmitterAddress());
        delete packet;
        return;
    }

    delete packet;

    // mark STA as associated
    if (sta->status != ASSOCIATED)
        sendAssocNotification(sta->address);
    sta->status = ASSOCIATED;    // XXX this should only take place when MAC receives the ACK for the response

    // send OK response
    const auto& resp = std::make_shared<Ieee80211ManagementHeader>();
    resp->setType(ST_ASSOCIATIONRESPONSE);
    const auto& body = std::make_shared<Ieee80211AssociationResponseFrame>();
    body->setStatusCode(SC_SUCCESSFUL);
    body->setAid(0);    //XXX
    body->setSupportedRates(supportedRates);
    body->setChunkLength(byte(2 + 2 + 2 + body->getSupportedRates().numRates + 2));
    resp->setChunkLength(byte(24));
    sendManagementFrame("AssocResp-OK", resp, body, sta->address);
}

void Ieee80211MgmtAP::handleAssociationResponseFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAP::handleReassociationRequestFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    EV << "Processing ReassociationRequest frame\n";

    // "11.3.4 AP reassociation procedures" -- almost the same as AssociationRequest processing
    STAInfo *sta = lookupSenderSTA(frame);
    if (!sta || sta->status == NOT_AUTHENTICATED) {
        // STA not authenticated: send error and return
        const auto& resp = std::make_shared<Ieee80211ManagementHeader>();
        resp->setType(ST_DEAUTHENTICATION);
        const auto& body = std::make_shared<Ieee80211DeauthenticationFrame>();
        body->setReasonCode(RC_NONAUTH_ASS_REQUEST);
        resp->setChunkLength(byte(24));
        sendManagementFrame("Deauth", resp, body, frame->getTransmitterAddress());
        delete packet;
        return;
    }

    delete packet;

    // mark STA as associated
    sta->status = ASSOCIATED;    // XXX this should only take place when MAC receives the ACK for the response

    // send OK response
    const auto& resp = std::make_shared<Ieee80211ManagementHeader>();
    resp->setType(ST_REASSOCIATIONRESPONSE);
    const auto& body = std::make_shared<Ieee80211ReassociationResponseFrame>();
    body->setStatusCode(SC_SUCCESSFUL);
    body->setAid(0);    //XXX
    body->setSupportedRates(supportedRates);
    body->setChunkLength(byte(2 + (2 + ssid.length()) + (2 + supportedRates.numRates) + 6));
    resp->setChunkLength(byte(24));
    sendManagementFrame("ReassocResp-OK", resp, body, sta->address);
}

void Ieee80211MgmtAP::handleReassociationResponseFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAP::handleDisassociationFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    STAInfo *sta = lookupSenderSTA(frame);
    delete packet;

    if (sta) {
        if (sta->status == ASSOCIATED)
            sendDisAssocNotification(sta->address);
        sta->status = AUTHENTICATED;
    }
}

void Ieee80211MgmtAP::handleBeaconFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAP::handleProbeRequestFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    EV << "Processing ProbeRequest frame\n";

    const auto& requestBody = packet->peekDataAt<Ieee80211ProbeRequestFrame>(frame->getChunkLength());
    if (strcmp(requestBody->getSSID(), "") != 0 && strcmp(requestBody->getSSID(), ssid.c_str()) != 0) {
        EV << "SSID `" << requestBody->getSSID() << "' does not match, ignoring frame\n";
        dropManagementFrame(packet);
        return;
    }

    MACAddress staAddress = frame->getTransmitterAddress();
    delete packet;

    EV << "Sending ProbeResponse frame\n";
    const auto& resp = std::make_shared<Ieee80211ManagementHeader>();
    resp->setType(ST_PROBERESPONSE);
    const auto& body = std::make_shared<Ieee80211ProbeResponseFrame>();
    body->setSSID(ssid.c_str());
    body->setSupportedRates(supportedRates);
    body->setBeaconInterval(beaconInterval);
    body->setChannelNumber(channelNumber);
    body->setChunkLength(byte(8 + 2 + 2 + (2 + ssid.length()) + (2 + supportedRates.numRates)));
    resp->setChunkLength(byte(24));
    sendManagementFrame("ProbeResp", resp, body, staAddress);
}

void Ieee80211MgmtAP::handleProbeResponseFrame(Packet *packet, const Ptr<Ieee80211ManagementHeader>& frame)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtAP::sendAssocNotification(const MACAddress& addr)
{
    NotificationInfoSta notif;
    notif.setApAddress(myAddress);
    notif.setStaAddress(addr);
    emit(NF_L2_AP_ASSOCIATED, &notif);
}

void Ieee80211MgmtAP::sendDisAssocNotification(const MACAddress& addr)
{
    NotificationInfoSta notif;
    notif.setApAddress(myAddress);
    notif.setStaAddress(addr);
    emit(NF_L2_AP_DISASSOCIATED, &notif);
}

void Ieee80211MgmtAP::start()
{
    Ieee80211MgmtAPBase::start();
    scheduleAt(simTime() + uniform(0, beaconInterval), beaconTimer);
}

void Ieee80211MgmtAP::stop()
{
    cancelEvent(beaconTimer);
    staList.clear();
    Ieee80211MgmtAPBase::stop();
}

} // namespace ieee80211

} // namespace inet

