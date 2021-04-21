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

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211SubtypeTag_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"

namespace inet {
namespace ieee80211 {

using namespace physicallayer;

//TBD supportedRates!
//TBD use command msg kinds?
//TBD implement bitrate switching (Radio already supports it)
//TBD where to put LCC header (SNAP)..?
//TBD mac should be able to signal when msg got transmitted

Define_Module(Ieee80211MgmtSta);

// message kind values for timers
#define MK_AUTH_TIMEOUT           1
#define MK_ASSOC_TIMEOUT          2
#define MK_SCAN_SENDPROBE         3
#define MK_SCAN_MINCHANNELTIME    4
#define MK_SCAN_MAXCHANNELTIME    5
#define MK_BEACON_TIMEOUT         6

#define MAX_BEACONS_MISSED        3.5  // beacon lost timeout, in beacon intervals (doesn't need to be integer)

std::ostream& operator<<(std::ostream& os, const Ieee80211MgmtSta::ScanningInfo& scanning)
{
    os << "activeScan=" << scanning.activeScan
       << " probeDelay=" << scanning.probeDelay
       << " curChan=";
    if (scanning.channelList.empty())
        os << "<none>";
    else
        os << scanning.channelList[scanning.currentChannelIndex];
    os << " minChanTime=" << scanning.minChannelTime
       << " maxChanTime=" << scanning.maxChannelTime;
    os << " chanList={";
    for (size_t i = 0; i < scanning.channelList.size(); i++)
        os << (i == 0 ? "" : " ") << scanning.channelList[i];
    os << "}";

    return os;
}

std::ostream& operator<<(std::ostream& os, const Ieee80211MgmtSta::ApInfo& ap)
{
    os << "AP addr=" << ap.address
       << " chan=" << ap.channel
       << " ssid=" << ap.ssid
        //TBD supportedRates
       << " beaconIntvl=" << ap.beaconInterval
       << " rxPower=" << ap.rxPower
       << " authSeqExpected=" << ap.authSeqExpected
       << " isAuthenticated=" << ap.isAuthenticated;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Ieee80211MgmtSta::AssociatedApInfo& assocAP)
{
    os << "AP addr=" << assocAP.address
       << " chan=" << assocAP.channel
       << " ssid=" << assocAP.ssid
       << " beaconIntvl=" << assocAP.beaconInterval
       << " receiveSeq=" << assocAP.receiveSequence
       << " rxPower=" << assocAP.rxPower;
    return os;
}

void Ieee80211MgmtSta::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        mib->mode = Ieee80211Mib::INFRASTRUCTURE;
        mib->bssStationData.stationType = Ieee80211Mib::STATION;
        mib->bssStationData.isAssociated = false;

        isScanning = false;
        assocTimeoutMsg = nullptr;
        numChannels = par("numChannels");

        host = getContainingNode(this);

        WATCH(isScanning);

        WATCH(scanning);
        WATCH(assocAP);
        WATCH_LIST(apList);
    }
}

void Ieee80211MgmtSta::handleTimer(cMessage *msg)
{
    if (msg->getKind() == MK_AUTH_TIMEOUT) {
        // authentication timed out
        ApInfo *ap = (ApInfo *)msg->getContextPointer();
        EV << "Authentication timed out, AP address = " << ap->address << "\n";

        // send back failure report to agent
        sendAuthenticationConfirm(ap, PRC_TIMEOUT);
    }
    else if (msg->getKind() == MK_ASSOC_TIMEOUT) {
        // association timed out
        ApInfo *ap = (ApInfo *)msg->getContextPointer();
        EV << "Association timed out, AP address = " << ap->address << "\n";

        // send back failure report to agent
        sendAssociationConfirm(ap, PRC_TIMEOUT);
    }
    else if (msg->getKind() == MK_SCAN_MAXCHANNELTIME) {
        // go to next channel during scanning
        bool done = scanNextChannel();
        if (done)
            sendScanConfirm(); // send back response to agents' "scan" command
        delete msg;
    }
    else if (msg->getKind() == MK_SCAN_SENDPROBE) {
        // Active Scan: send a probe request, then wait for minChannelTime (11.1.3.2.2)
        delete msg;
        sendProbeRequest();
        cMessage *timerMsg = new cMessage("minChannelTime", MK_SCAN_MINCHANNELTIME);
        scheduleAt(simTime() + scanning.minChannelTime, timerMsg);    //XXX actually, we should start waiting after ProbeReq actually got transmitted
    }
    else if (msg->getKind() == MK_SCAN_MINCHANNELTIME) {
        // Active Scan: after minChannelTime, possibly listen for the remaining time until maxChannelTime
        delete msg;
        if (scanning.busyChannelDetected) {
            EV << "Busy channel detected during minChannelTime, continuing listening until maxChannelTime elapses\n";
            cMessage *timerMsg = new cMessage("maxChannelTime", MK_SCAN_MAXCHANNELTIME);
            scheduleAt(simTime() + scanning.maxChannelTime - scanning.minChannelTime, timerMsg);
        }
        else {
            EV << "Channel was empty during minChannelTime, going to next channel\n";
            bool done = scanNextChannel();
            if (done)
                sendScanConfirm(); // send back response to agents' "scan" command
        }
    }
    else if (msg->getKind() == MK_BEACON_TIMEOUT) {
        // missed a few consecutive beacons
        beaconLost();
    }
    else {
        throw cRuntimeError("internal error: unrecognized timer '%s'", msg->getName());
    }
}

void Ieee80211MgmtSta::handleCommand(int msgkind, cObject *ctrl)
{
    if (auto cmd = dynamic_cast<Ieee80211Prim_ScanRequest *>(ctrl))
        processScanCommand(cmd);
    else if (auto cmd = dynamic_cast<Ieee80211Prim_AuthenticateRequest *>(ctrl))
        processAuthenticateCommand(cmd);
    else if (auto cmd = dynamic_cast<Ieee80211Prim_DeauthenticateRequest *>(ctrl))
        processDeauthenticateCommand(cmd);
    else if (auto cmd = dynamic_cast<Ieee80211Prim_AssociateRequest *>(ctrl))
        processAssociateCommand(cmd);
    else if (auto cmd = dynamic_cast<Ieee80211Prim_ReassociateRequest *>(ctrl))
        processReassociateCommand(cmd);
    else if (auto cmd = dynamic_cast<Ieee80211Prim_DisassociateRequest *>(ctrl))
        processDisassociateCommand(cmd);
    else if (ctrl)
        throw cRuntimeError("handleCommand(): unrecognized control info class `%s'", ctrl->getClassName());
    else
        throw cRuntimeError("handleCommand(): control info is nullptr");
    delete ctrl;
}

Ieee80211MgmtSta::ApInfo *Ieee80211MgmtSta::lookupAP(const MacAddress& address)
{
    for (auto & elem : apList)
        if (elem.address == address)
            return &(elem);

    return nullptr;
}

void Ieee80211MgmtSta::clearAPList()
{
    for (auto & elem : apList)
        if (elem.authTimeoutMsg)
            delete cancelEvent(elem.authTimeoutMsg);

    apList.clear();
}

void Ieee80211MgmtSta::changeChannel(int channelNum)
{
    EV << "Tuning to channel #" << channelNum << "\n";

    Ieee80211ConfigureRadioCommand *configureCommand = new Ieee80211ConfigureRadioCommand();
    configureCommand->setChannelNumber(channelNum);
    auto request = new Request("changeChannel", RADIO_C_CONFIGURE);
    request->setControlInfo(configureCommand);
    send(request, "macOut");
}

void Ieee80211MgmtSta::beaconLost()
{
    EV << "Missed a few consecutive beacons -- AP is considered lost\n";
    emit(l2BeaconLostSignal, myIface);
}

void Ieee80211MgmtSta::sendManagementFrame(const char *name, const Ptr<Ieee80211MgmtFrame>& body, int subtype, const MacAddress& address)
{
    auto packet = new Packet(name);
    packet->addTag<MacAddressReq>()->setDestAddress(address);
    packet->addTag<Ieee80211SubtypeReq>()->setSubtype(subtype);
    packet->insertAtBack(body);
    sendDown(packet);
}

void Ieee80211MgmtSta::startAuthentication(ApInfo *ap, simtime_t timeout)
{
    if (ap->authTimeoutMsg)
        throw cRuntimeError("startAuthentication: authentication currently in progress with AP address='%s'", ap->address.str().c_str());
    if (ap->isAuthenticated)
        throw cRuntimeError("startAuthentication: already authenticated with AP address='%s'", ap->address.str().c_str());

    changeChannel(ap->channel);

    EV << "Sending initial Authentication frame with seqNum=1\n";

    // create and send first authentication frame
    const auto& body = makeShared<Ieee80211AuthenticationFrame>();
    body->setSequenceNumber(1);
    //XXX frame length could be increased to account for challenge text length etc.
    sendManagementFrame("Auth", body, ST_AUTHENTICATION, ap->address);

    ap->authSeqExpected = 2;

    // schedule timeout
    ASSERT(ap->authTimeoutMsg == nullptr);
    ap->authTimeoutMsg = new cMessage("authTimeout", MK_AUTH_TIMEOUT);
    ap->authTimeoutMsg->setContextPointer(ap);
    scheduleAt(simTime() + timeout, ap->authTimeoutMsg);
}

void Ieee80211MgmtSta::startAssociation(ApInfo *ap, simtime_t timeout)
{
    if (mib->bssStationData.isAssociated || assocTimeoutMsg)
        throw cRuntimeError("startAssociation: already associated or association currently in progress");
    if (!ap->isAuthenticated)
        throw cRuntimeError("startAssociation: not yet authenticated with AP address='%s'", ap->address.str().c_str());

    // switch to that channel
    changeChannel(ap->channel);

    // create and send association request
    const auto& body = makeShared<Ieee80211AssociationRequestFrame>();

    //XXX set the following too?
    // string SSID
    // Ieee80211SupportedRatesElement supportedRates;

    body->setChunkLength(B(2 + 2 + strlen(body->getSSID()) + 2 + body->getSupportedRates().numRates + 2));
    sendManagementFrame("Assoc", body, ST_ASSOCIATIONREQUEST, ap->address);

    // schedule timeout
    ASSERT(assocTimeoutMsg == nullptr);
    assocTimeoutMsg = new cMessage("assocTimeout", MK_ASSOC_TIMEOUT);
    assocTimeoutMsg->setContextPointer(ap);
    scheduleAt(simTime() + timeout, assocTimeoutMsg);
}

void Ieee80211MgmtSta::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method_Silent();
    // Note that we are only subscribed during scanning!
    if (signalID == IRadio::receptionStateChangedSignal) {
        IRadio::ReceptionState newReceptionState = static_cast<IRadio::ReceptionState>(value);
        if (newReceptionState != IRadio::RECEPTION_STATE_UNDEFINED && newReceptionState != IRadio::RECEPTION_STATE_IDLE) {
            EV << "busy radio channel detected during scanning\n";
            scanning.busyChannelDetected = true;
        }
    }
}

void Ieee80211MgmtSta::processScanCommand(Ieee80211Prim_ScanRequest *ctrl)
{
    EV << "Received Scan Request from agent, clearing AP list and starting scanning...\n";

    if (isScanning)
        throw cRuntimeError("processScanCommand: scanning already in progress");
    if (mib->bssStationData.isAssociated) {
        disassociate();
    }
    else if (assocTimeoutMsg) {
        EV << "Cancelling ongoing association process\n";
        delete cancelEvent(assocTimeoutMsg);
        assocTimeoutMsg = nullptr;
    }

    // clear existing AP list (and cancel any pending authentications) -- we want to start with a clean page
    clearAPList();

    // fill in scanning state
    ASSERT(ctrl->getBSSType() == BSSTYPE_INFRASTRUCTURE);
    scanning.bssid = ctrl->getBSSID().isUnspecified() ? MacAddress::BROADCAST_ADDRESS : ctrl->getBSSID();
    scanning.ssid = ctrl->getSSID();
    scanning.activeScan = ctrl->getActiveScan();
    scanning.probeDelay = ctrl->getProbeDelay();
    scanning.channelList.clear();
    scanning.minChannelTime = ctrl->getMinChannelTime();
    scanning.maxChannelTime = ctrl->getMaxChannelTime();
    ASSERT(scanning.minChannelTime <= scanning.maxChannelTime);

    // channel list to scan (default: all channels)
    for (size_t i = 0; i < ctrl->getChannelListArraySize(); i++)
        scanning.channelList.push_back(ctrl->getChannelList(i));
    if (scanning.channelList.empty())
        for (int i = 0; i < numChannels; i++)
            scanning.channelList.push_back(i);


    // start scanning
    if (scanning.activeScan)
        host->subscribe(IRadio::receptionStateChangedSignal, this);
    scanning.currentChannelIndex = -1;    // so we'll start with index==0
    isScanning = true;
    scanNextChannel();
}

bool Ieee80211MgmtSta::scanNextChannel()
{
    // if we're already at the last channel, we're through
    if (scanning.currentChannelIndex == (int)scanning.channelList.size() - 1) {
        EV << "Finished scanning last channel\n";
        if (scanning.activeScan)
            host->unsubscribe(IRadio::receptionStateChangedSignal, this);
        isScanning = false;
        return true;    // we're done
    }

    // tune to next channel
    int newChannel = scanning.channelList[++scanning.currentChannelIndex];
    changeChannel(newChannel);
    scanning.busyChannelDetected = false;

    if (scanning.activeScan) {
        // Active Scan: first wait probeDelay, then send a probe. Listening
        // for minChannelTime or maxChannelTime takes place after that. (11.1.3.2)
        scheduleAt(simTime() + scanning.probeDelay, new cMessage("sendProbe", MK_SCAN_SENDPROBE));
    }
    else {
        // Passive Scan: spend maxChannelTime on the channel (11.1.3.1)
        cMessage *timerMsg = new cMessage("maxChannelTime", MK_SCAN_MAXCHANNELTIME);
        scheduleAt(simTime() + scanning.maxChannelTime, timerMsg);
    }

    return false;
}

void Ieee80211MgmtSta::sendProbeRequest()
{
    EV << "Sending Probe Request, BSSID=" << scanning.bssid << ", SSID=\"" << scanning.ssid << "\"\n";
    const auto& body = makeShared<Ieee80211ProbeRequestFrame>();
    body->setSSID(scanning.ssid.c_str());
    body->setChunkLength(B((2 + scanning.ssid.length()) + (2 + body->getSupportedRates().numRates)));
    sendManagementFrame("ProbeReq", body, ST_PROBEREQUEST, scanning.bssid);
}

void Ieee80211MgmtSta::sendScanConfirm()
{
    EV << "Scanning complete, found " << apList.size() << " APs, sending confirmation to agent\n";

    // copy apList contents into a ScanConfirm primitive and send it back
    int n = apList.size();
    Ieee80211Prim_ScanConfirm *confirm = new Ieee80211Prim_ScanConfirm();
    confirm->setBssListArraySize(n);
    auto it = apList.begin();
    //XXX filter for req'd bssid and ssid
    for (int i = 0; i < n; i++, it++) {
        ApInfo *ap = &(*it);
        Ieee80211Prim_BssDescription& bss = confirm->getBssListForUpdate(i);
        bss.setChannelNumber(ap->channel);
        bss.setBSSID(ap->address);
        bss.setSSID(ap->ssid.c_str());
        bss.setSupportedRates(ap->supportedRates);
        bss.setBeaconInterval(ap->beaconInterval);
        bss.setRxPower(ap->rxPower);
    }
    sendConfirm(confirm, PRC_SUCCESS);
}

void Ieee80211MgmtSta::processAuthenticateCommand(Ieee80211Prim_AuthenticateRequest *ctrl)
{
    const MacAddress& address = ctrl->getAddress();
    ApInfo *ap = lookupAP(address);
    if (!ap)
        throw cRuntimeError("processAuthenticateCommand: AP not known: address = %s", address.str().c_str());
    startAuthentication(ap, ctrl->getTimeout());
}

void Ieee80211MgmtSta::processDeauthenticateCommand(Ieee80211Prim_DeauthenticateRequest *ctrl)
{
    const MacAddress& address = ctrl->getAddress();
    ApInfo *ap = lookupAP(address);
    if (!ap)
        throw cRuntimeError("processDeauthenticateCommand: AP not known: address = %s", address.str().c_str());

    if (mib->bssStationData.isAssociated && assocAP.address == address)
        disassociate();

    if (ap->isAuthenticated)
        ap->isAuthenticated = false;

    // cancel possible pending authentication timer
    if (ap->authTimeoutMsg) {
        delete cancelEvent(ap->authTimeoutMsg);
        ap->authTimeoutMsg = nullptr;
    }

    // create and send deauthentication request
    const auto& body = makeShared<Ieee80211DeauthenticationFrame>();
    body->setReasonCode(ctrl->getReasonCode());
    sendManagementFrame("Deauth", body, ST_DEAUTHENTICATION, address);
}

void Ieee80211MgmtSta::processAssociateCommand(Ieee80211Prim_AssociateRequest *ctrl)
{
    const MacAddress& address = ctrl->getAddress();
    ApInfo *ap = lookupAP(address);
    if (!ap)
        throw cRuntimeError("processAssociateCommand: AP not known: address = %s", address.str().c_str());
    startAssociation(ap, ctrl->getTimeout());
}

void Ieee80211MgmtSta::processReassociateCommand(Ieee80211Prim_ReassociateRequest *ctrl)
{
    // treat the same way as association
    //XXX refine
    processAssociateCommand(ctrl);
}

void Ieee80211MgmtSta::processDisassociateCommand(Ieee80211Prim_DisassociateRequest *ctrl)
{
    const MacAddress& address = ctrl->getAddress();

    if (mib->bssStationData.isAssociated && address == assocAP.address) {
        disassociate();
    }
    else if (assocTimeoutMsg) {
        // pending association
        delete cancelEvent(assocTimeoutMsg);
        assocTimeoutMsg = nullptr;
    }

    // create and send disassociation request
    const auto& body = makeShared<Ieee80211DisassociationFrame>();
    body->setReasonCode(ctrl->getReasonCode());
    sendManagementFrame("Disass", body, ST_DISASSOCIATION, address);
}

void Ieee80211MgmtSta::disassociate()
{
    EV << "Disassociating from AP address=" << assocAP.address << "\n";
    ASSERT(mib->bssStationData.isAssociated);
    mib->bssStationData.isAssociated = false;
    delete cancelEvent(assocAP.beaconTimeoutMsg);
    assocAP.beaconTimeoutMsg = nullptr;
    assocAP = AssociatedApInfo();    // clear it
}

void Ieee80211MgmtSta::sendAuthenticationConfirm(ApInfo *ap, Ieee80211PrimResultCode resultCode)
{
    Ieee80211Prim_AuthenticateConfirm *confirm = new Ieee80211Prim_AuthenticateConfirm();
    confirm->setAddress(ap->address);
    sendConfirm(confirm, resultCode);
}

void Ieee80211MgmtSta::sendAssociationConfirm(ApInfo *ap, Ieee80211PrimResultCode resultCode)
{
    sendConfirm(new Ieee80211Prim_AssociateConfirm(), resultCode);
}

void Ieee80211MgmtSta::sendConfirm(Ieee80211PrimConfirm *confirm, Ieee80211PrimResultCode resultCode)
{
    confirm->setResultCode(resultCode);
    cMessage *msg = new cMessage(confirm->getClassName());
    msg->setControlInfo(confirm);
    send(msg, "agentOut");
}

Ieee80211PrimResultCode Ieee80211MgmtSta::statusCodeToPrimResultCode(int statusCode)
{
    return statusCode == SC_SUCCESSFUL ? PRC_SUCCESS : PRC_REFUSED;
}

void Ieee80211MgmtSta::handleAuthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    const auto& requestBody = packet->peekData<Ieee80211AuthenticationFrame>();
    MacAddress address = header->getTransmitterAddress();
    int frameAuthSeq = requestBody->getSequenceNumber();
    EV << "Received Authentication frame from address=" << address << ", seqNum=" << frameAuthSeq << "\n";

    ApInfo *ap = lookupAP(address);
    if (!ap) {
        EV << "AP not known, discarding authentication frame\n";
        delete packet;
        return;
    }

    // what if already authenticated with AP
    if (ap->isAuthenticated) {
        EV << "AP already authenticated, ignoring frame\n";
        delete packet;
        return;
    }

    // is authentication is in progress with this AP?
    if (!ap->authTimeoutMsg) {
        EV << "No authentication in progress with AP, ignoring frame\n";
        delete packet;
        return;
    }

    // check authentication sequence number is OK
    if (frameAuthSeq != ap->authSeqExpected) {
        // wrong sequence number: send error and return
        EV << "Wrong sequence number, " << ap->authSeqExpected << " expected\n";
        const auto& body = makeShared<Ieee80211AuthenticationFrame>();
        body->setStatusCode(SC_AUTH_OUT_OF_SEQ);
        sendManagementFrame("Auth-ERROR", body, ST_AUTHENTICATION, header->getTransmitterAddress());
        delete packet;

        // cancel timeout, send error to agent
        delete cancelEvent(ap->authTimeoutMsg);
        ap->authTimeoutMsg = nullptr;
        sendAuthenticationConfirm(ap, PRC_REFUSED);    //XXX or what resultCode?
        return;
    }

    // check if more exchanges are needed for auth to be complete
    int statusCode = requestBody->getStatusCode();

    if (statusCode == SC_SUCCESSFUL && !requestBody->isLast()) {
        EV << "More steps required, sending another Authentication frame\n";

        // more steps required, send another Authentication frame
        const auto& body = makeShared<Ieee80211AuthenticationFrame>();
        body->setSequenceNumber(frameAuthSeq + 1);
        body->setStatusCode(SC_SUCCESSFUL);
        // XXX frame length could be increased to account for challenge text length etc.
        sendManagementFrame("Auth", body, ST_AUTHENTICATION, address);
        ap->authSeqExpected += 2;
    }
    else {
        if (statusCode == SC_SUCCESSFUL)
            EV << "Authentication successful\n";
        else
            EV << "Authentication failed\n";

        // authentication completed
        ap->isAuthenticated = (statusCode == SC_SUCCESSFUL);
        delete cancelEvent(ap->authTimeoutMsg);
        ap->authTimeoutMsg = nullptr;
        sendAuthenticationConfirm(ap, statusCodeToPrimResultCode(statusCode));
    }

    delete packet;
}

void Ieee80211MgmtSta::handleDeauthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Received Deauthentication frame\n";
    const MacAddress& address = header->getAddress3();    // source address
    ApInfo *ap = lookupAP(address);
    if (!ap || !ap->isAuthenticated) {
        EV << "Unknown AP, or not authenticated with that AP -- ignoring frame\n";
        delete packet;
        return;
    }
    if (ap->authTimeoutMsg) {
        delete cancelEvent(ap->authTimeoutMsg);
        ap->authTimeoutMsg = nullptr;
        EV << "Cancelling pending authentication\n";
        delete packet;
        return;
    }

    EV << "Setting isAuthenticated flag for that AP to false\n";
    ap->isAuthenticated = false;
    delete packet;
}

void Ieee80211MgmtSta::handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSta::handleAssociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Received Association Response frame\n";

    if (!assocTimeoutMsg) {
        EV << "No association in progress, ignoring frame\n";
        delete packet;
        return;
    }

    // extract frame contents
    const auto& responseBody = packet->peekData<Ieee80211AssociationResponseFrame>();
    MacAddress address = header->getTransmitterAddress();
    int statusCode = responseBody->getStatusCode();
    //XXX short aid;
    //XXX Ieee80211SupportedRatesElement supportedRates;
    delete packet;

    // look up AP data structure
    ApInfo *ap = lookupAP(address);
    if (!ap)
        throw cRuntimeError("handleAssociationResponseFrame: AP not known: address=%s", address.str().c_str());

    if (mib->bssStationData.isAssociated) {
        EV << "Breaking existing association with AP address=" << assocAP.address << "\n";
        mib->bssStationData.isAssociated = false;
        delete cancelEvent(assocAP.beaconTimeoutMsg);
        assocAP.beaconTimeoutMsg = nullptr;
        assocAP = AssociatedApInfo();
    }

    delete cancelEvent(assocTimeoutMsg);
    assocTimeoutMsg = nullptr;

    if (statusCode != SC_SUCCESSFUL) {
        EV << "Association failed with AP address=" << ap->address << "\n";
    }
    else {
        EV << "Association successful, AP address=" << ap->address << "\n";

        // change our state to "associated"
        mib->bssData.ssid = ap->ssid;
        mib->bssData.bssid = ap->address;
        mib->bssStationData.isAssociated = true;
        (ApInfo&)assocAP = (*ap);

        emit(l2AssociatedSignal, myIface, ap);

        assocAP.beaconTimeoutMsg = new cMessage("beaconTimeout", MK_BEACON_TIMEOUT);
        scheduleAt(simTime() + MAX_BEACONS_MISSED * assocAP.beaconInterval, assocAP.beaconTimeoutMsg);
    }

    // report back to agent
    sendAssociationConfirm(ap, statusCodeToPrimResultCode(statusCode));
}

void Ieee80211MgmtSta::handleReassociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSta::handleReassociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Received Reassociation Response frame\n";
    //TBD handle with the same code as Association Response?
}

void Ieee80211MgmtSta::handleDisassociationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Received Disassociation frame\n";
    const MacAddress& address = header->getAddress3();    // source address

    if (assocTimeoutMsg) {
        // pending association
        delete cancelEvent(assocTimeoutMsg);
        assocTimeoutMsg = nullptr;
    }
    if (!mib->bssStationData.isAssociated || address != assocAP.address) {
        EV << "Not associated with that AP -- ignoring frame\n";
        delete packet;
        return;
    }

    EV << "Setting isAssociated flag to false\n";
    mib->bssStationData.isAssociated = false;
    delete cancelEvent(assocAP.beaconTimeoutMsg);
    assocAP.beaconTimeoutMsg = nullptr;
}

void Ieee80211MgmtSta::handleBeaconFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Received Beacon frame\n";
    const auto& beaconBody = packet->peekData<Ieee80211BeaconFrame>();
    storeAPInfo(packet, header, beaconBody);

    // if it is out associate AP, restart beacon timeout
    if (mib->bssStationData.isAssociated && header->getTransmitterAddress() == assocAP.address) {
        EV << "Beacon is from associated AP, restarting beacon timeout timer\n";
        ASSERT(assocAP.beaconTimeoutMsg != nullptr);
        cancelEvent(assocAP.beaconTimeoutMsg);
        scheduleAt(simTime() + MAX_BEACONS_MISSED * assocAP.beaconInterval, assocAP.beaconTimeoutMsg);

        //ApInfo *ap = lookupAP(frame->getTransmitterAddress());
        //ASSERT(ap!=nullptr);
    }

    delete packet;
}

void Ieee80211MgmtSta::handleProbeRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSta::handleProbeResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Received Probe Response frame\n";
    const auto& probeResponseBody = packet->peekData<Ieee80211ProbeResponseFrame>();
    storeAPInfo(packet, header, probeResponseBody);
    delete packet;
}

void Ieee80211MgmtSta::storeAPInfo(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header, const Ptr<const Ieee80211BeaconFrame>& body)
{
    auto address = header->getTransmitterAddress();
    ApInfo *ap = lookupAP(address);
    if (ap) {
        EV << "AP address=" << address << ", SSID=" << body->getSSID() << " already in our AP list, refreshing the info\n";
    }
    else {
        EV << "Inserting AP address=" << address << ", SSID=" << body->getSSID() << " into our AP list\n";
        apList.push_back(ApInfo());
        ap = &apList.back();
    }

    ap->channel = body->getChannelNumber();
    ap->address = address;
    ap->ssid = body->getSSID();
    ap->supportedRates = body->getSupportedRates();
    ap->beaconInterval = body->getBeaconInterval();
    auto signalPowerInd = packet->getTag<SignalPowerInd>();
    if (signalPowerInd != nullptr) {
        ap->rxPower = signalPowerInd->getPower().get();
        if (ap->address == assocAP.address)
            assocAP.rxPower = ap->rxPower;
    }
}

} // namespace ieee80211
} // namespace inet

