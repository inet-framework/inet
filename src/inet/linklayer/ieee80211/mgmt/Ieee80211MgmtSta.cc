//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211HtMgmtElements.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/packet/Message.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211SubtypeTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"

namespace inet {
namespace ieee80211 {

using namespace physicallayer;

// TODO supportedRates!
// TODO use command msg kinds?
// TODO implement bitrate switching (Radio already supports it)
// TODO where to put LCC header (SNAP)..?
// TODO mac should be able to signal when msg got transmitted

Define_Module(Ieee80211MgmtSta);
Register_Class(Ieee80211MgmtSta::HtNegotiationFailure);

simsignal_t Ieee80211MgmtSta::htNegotiationFailedSignal = cComponent::registerSignal("htNegotiationFailed");

// message kind values for timers
#define MK_AUTH_TIMEOUT           1
#define MK_ASSOC_TIMEOUT          2
#define MK_SCAN_SENDPROBE         3
#define MK_SCAN_MINCHANNELTIME    4
#define MK_SCAN_MAXCHANNELTIME    5
#define MK_BEACON_TIMEOUT         6

#define MAX_BEACONS_MISSED        3.5  // beacon lost timeout, in beacon intervals (doesn't need to be integer)

Ieee80211MgmtSta::~Ieee80211MgmtSta()
{
    cancelAndDelete(scanTimer);
    cancelAndDelete(assocTimeoutMsg);
    for (auto& ap : apList)
        cancelAndDelete(ap.authTimeoutMsg);
    cancelAndDelete(assocAP.beaconTimeoutMsg);
}

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
        // TODO supportedRates
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
        WATCH(apList);
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
        ASSERT(msg == assocTimeoutMsg);
        ApInfo *ap = (ApInfo *)msg->getContextPointer();
        bool reassociation = reassociationInProgress;
        EV << "Association timed out, AP address = " << ap->address << "\n";

        assocTimeoutMsg = nullptr;
        reassociationInProgress = false;
        delete msg;

        // send back failure report to agent
        if (reassociation) {
            handleReassociationFailure(ap);
            sendReassociationConfirm(ap, PRC_TIMEOUT);
        }
        else
            sendAssociationConfirm(ap, PRC_TIMEOUT);
    }
    else if (msg->getKind() == MK_SCAN_MAXCHANNELTIME) {
        ASSERT(msg == scanTimer);
        scanTimer = nullptr;
        // go to next channel during scanning
        bool done = scanNextChannel();
        if (done)
            sendScanConfirm(); // send back response to agents' "scan" command
        delete msg;
    }
    else if (msg->getKind() == MK_SCAN_SENDPROBE) {
        ASSERT(msg == scanTimer);
        scanTimer = nullptr;
        // Active Scan: send a probe request, then wait for minChannelTime (11.1.3.2.2)
        delete msg;
        sendProbeRequest();
        ASSERT(scanTimer == nullptr);
        scanTimer = new cMessage("minChannelTime", MK_SCAN_MINCHANNELTIME);
        scheduleAfter(scanning.minChannelTime, scanTimer); // TODO actually, we should start waiting after ProbeReq actually got transmitted
    }
    else if (msg->getKind() == MK_SCAN_MINCHANNELTIME) {
        ASSERT(msg == scanTimer);
        scanTimer = nullptr;
        // Active Scan: after minChannelTime, possibly listen for the remaining time until maxChannelTime
        delete msg;
        if (scanning.busyChannelDetected) {
            EV << "Busy channel detected during minChannelTime, continuing listening until maxChannelTime elapses\n";
            ASSERT(scanTimer == nullptr);
            scanTimer = new cMessage("maxChannelTime", MK_SCAN_MAXCHANNELTIME);
            scheduleAfter(scanning.maxChannelTime - scanning.minChannelTime, scanTimer);
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
    else if (auto cmd = dynamic_cast<Ieee80211Prim_ReassociateRequest *>(ctrl))
        processReassociateCommand(cmd);
    else if (auto cmd = dynamic_cast<Ieee80211Prim_AssociateRequest *>(ctrl))
        processAssociateCommand(cmd);
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
    for (auto& elem : apList)
        if (elem.address == address)
            return &(elem);

    return nullptr;
}

void Ieee80211MgmtSta::clearAPList()
{
    cancelPendingAssociation();

    for (auto& elem : apList)
        if (elem.authTimeoutMsg)
            cancelAndDelete(elem.authTimeoutMsg);

    apList.clear();
}

void Ieee80211MgmtSta::cancelPendingAssociation()
{
    cancelAndDelete(assocTimeoutMsg);
    assocTimeoutMsg = nullptr;
    reassociationInProgress = false;
}

void Ieee80211MgmtSta::cancelScanTimer()
{
    cancelAndDelete(scanTimer);
    scanTimer = nullptr;
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
    // TODO frame length could be increased to account for challenge text length etc.
    sendManagementFrame("Auth", body, ST_AUTHENTICATION, ap->address);

    ap->authSeqExpected = 2;

    // schedule timeout
    ASSERT(ap->authTimeoutMsg == nullptr);
    ap->authTimeoutMsg = new cMessage("authTimeout", MK_AUTH_TIMEOUT);
    ap->authTimeoutMsg->setContextPointer(ap);
    scheduleAfter(timeout, ap->authTimeoutMsg);
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
    body->setSSID(ap->ssid.c_str());
    setSupportedRateElements(body);
    addHtCapabilities(body);
    body->setChunkLength(B(2 + 2 + (2 + strlen(body->getSSID()))) + getSupportedRateElementsLength(body) + getHtMgmtElementsLength(body));
    sendManagementFrame("Assoc", body, ST_ASSOCIATIONREQUEST, ap->address);
    reassociationInProgress = false;

    // schedule timeout
    ASSERT(assocTimeoutMsg == nullptr);
    assocTimeoutMsg = new cMessage("assocTimeout", MK_ASSOC_TIMEOUT);
    assocTimeoutMsg->setContextPointer(ap);
    scheduleAfter(timeout, assocTimeoutMsg);
}

void Ieee80211MgmtSta::startReassociation(ApInfo *ap, simtime_t timeout)
{
    if (!mib->bssStationData.isAssociated || assocTimeoutMsg)
        throw cRuntimeError("startReassociation: not associated or association currently in progress");
    if (!ap->isAuthenticated)
        throw cRuntimeError("startReassociation: not authenticated with AP address='%s'", ap->address.str().c_str());
    changeChannel(ap->channel);
    const auto& body = makeShared<Ieee80211ReassociationRequestFrame>();
    body->setCurrentAP(assocAP.address);
    body->setSSID(ap->ssid.c_str());
    setSupportedRateElements(body);
    addHtCapabilities(body);
    body->setChunkLength(B(2 + 2 + 6 + (2 + strlen(body->getSSID()))) + getSupportedRateElementsLength(body) + getHtMgmtElementsLength(body));
    sendManagementFrame("Reassoc", body, ST_REASSOCIATIONREQUEST, ap->address);
    reassociationInProgress = true;
    assocTimeoutMsg = new cMessage("assocTimeout", MK_ASSOC_TIMEOUT);
    assocTimeoutMsg->setContextPointer(ap);
    scheduleAfter(timeout, assocTimeoutMsg);
}

void Ieee80211MgmtSta::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

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
    if (mib->bssStationData.isAssociated)
        disassociate();

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
    scanning.currentChannelIndex = -1; // so we'll start with index==0
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
        return true; // we're done
    }

    // tune to next channel
    int newChannel = scanning.channelList[++scanning.currentChannelIndex];
    changeChannel(newChannel);
    scanning.busyChannelDetected = false;

    ASSERT(scanTimer == nullptr);
    if (scanning.activeScan) {
        // Active Scan: first wait probeDelay, then send a probe. Listening
        // for minChannelTime or maxChannelTime takes place after that. (11.1.3.2)
        scanTimer = new cMessage("sendProbe", MK_SCAN_SENDPROBE);
        scheduleAfter(scanning.probeDelay, scanTimer);
    }
    else {
        // Passive Scan: spend maxChannelTime on the channel (11.1.3.1)
        scanTimer = new cMessage("maxChannelTime", MK_SCAN_MAXCHANNELTIME);
        scheduleAfter(scanning.maxChannelTime, scanTimer);
    }

    return false;
}

void Ieee80211MgmtSta::sendProbeRequest()
{
    EV << "Sending Probe Request, BSSID=" << scanning.bssid << ", SSID=\"" << scanning.ssid << "\"\n";
    const auto& body = makeShared<Ieee80211ProbeRequestFrame>();
    body->setSSID(scanning.ssid.c_str());
    setSupportedRateElements(body);
    addHtCapabilities(body);
    body->setChunkLength(B(2 + scanning.ssid.length()) + getSupportedRateElementsLength(body) + getHtMgmtElementsLength(body));
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
    // TODO filter for req'd bssid and ssid
    for (int i = 0; i < n; i++, it++) {
        ApInfo *ap = &(*it);
        Ieee80211Prim_BssDescription& bss = confirm->getBssListForUpdate(i);
        bss.setChannelNumber(ap->channel);
        bss.setBSSID(ap->address);
        bss.setSSID(ap->ssid.c_str());
        bss.setSupportedRates(ap->supportedRates);
        bss.setExtendedSupportedRatesPresent(ap->extendedSupportedRatesPresent);
        bss.setExtendedSupportedRates(ap->extendedSupportedRates);
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
    else if (assocTimeoutMsg && assocTimeoutMsg->getContextPointer() == ap)
        cancelPendingAssociation();

    if (ap->isAuthenticated)
        ap->isAuthenticated = false;

    // cancel possible pending authentication timer
    if (ap->authTimeoutMsg) {
        cancelAndDelete(ap->authTimeoutMsg);
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
    const MacAddress& address = ctrl->getAddress();
    if (!mib->bssStationData.isAssociated) {
        auto confirm = new Ieee80211Prim_ReassociateConfirm();
        confirm->setAddress(address);
        sendConfirm(confirm, PRC_REFUSED);
        return;
    }
    ApInfo *ap = lookupAP(address);
    if (!ap)
        throw cRuntimeError("processReassociateCommand: AP not known: address = %s", address.str().c_str());
    startReassociation(ap, ctrl->getTimeout());
}

void Ieee80211MgmtSta::processDisassociateCommand(Ieee80211Prim_DisassociateRequest *ctrl)
{
    const MacAddress& address = ctrl->getAddress();

    if (mib->bssStationData.isAssociated && address == assocAP.address) {
        disassociate();
    }
    else if (assocTimeoutMsg) {
        // IEEE Std 802.11-2024, 6.5.9.1.2 scopes PeerSTAAddress to the peer
        // being disassociated; the model cancels only a matching pending
        // transaction.
        auto pendingAp = static_cast<ApInfo *>(assocTimeoutMsg->getContextPointer());
        if (pendingAp != nullptr && pendingAp->address == address)
            cancelPendingAssociation();
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
    cancelPendingAssociation();
    clearCurrentAssociation();
}

void Ieee80211MgmtSta::clearCurrentAssociation()
{
    ASSERT(mib->bssStationData.isAssociated);
    mib->bssStationData.isAssociated = false;
    mib->removePeerHtCapabilities(assocAP.address);
    cancelAndDelete(assocAP.beaconTimeoutMsg);
    assocAP.beaconTimeoutMsg = nullptr;
    assocAP = AssociatedApInfo(); // clear it
}

bool Ieee80211MgmtSta::terminateCurrentAssociationFromPeer(const MacAddress& address)
{
    if (!mib->bssStationData.isAssociated || address != assocAP.address)
        return false;

    // Keep a stable AP-list object for the primitive confirmation while the
    // association snapshot is cleared. A pending transaction for a different
    // AP remains valid after the current association is terminated.
    ApInfo *pendingAp = assocTimeoutMsg ? static_cast<ApInfo *>(assocTimeoutMsg->getContextPointer()) : nullptr;
    bool pendingReassociation = reassociationInProgress;
    bool terminatesPendingRequest = pendingAp != nullptr && pendingAp->address == address;
    if (terminatesPendingRequest)
        cancelPendingAssociation();

    EV << "Setting isAssociated flag to false\n";
    clearCurrentAssociation();
    if (terminatesPendingRequest) {
        // The primitive API has no peer-aborted result; a termination of a
        // same-peer pending request is reported as a refusal after teardown.
        if (pendingReassociation)
            sendReassociationConfirm(pendingAp, PRC_REFUSED);
        else
            sendAssociationConfirm(pendingAp, PRC_REFUSED);
    }
    return true;
}

void Ieee80211MgmtSta::stop()
{
    if (host != nullptr && isScanning && scanning.activeScan)
        host->unsubscribe(IRadio::receptionStateChangedSignal, this);
    isScanning = false;
    cancelScanTimer();
    scanning = ScanningInfo();

    clearAPList();

    if (mib->bssStationData.isAssociated)
        clearCurrentAssociation();
    else {
        cancelAndDelete(assocAP.beaconTimeoutMsg);
        assocAP.beaconTimeoutMsg = nullptr;
        assocAP = AssociatedApInfo();
    }

    Ieee80211MgmtBase::stop();
}

void Ieee80211MgmtSta::sendAuthenticationConfirm(ApInfo *ap, Ieee80211PrimResultCode resultCode)
{
    Ieee80211Prim_AuthenticateConfirm *confirm = new Ieee80211Prim_AuthenticateConfirm();
    confirm->setAddress(ap->address);
    sendConfirm(confirm, resultCode);
}

void Ieee80211MgmtSta::sendAssociationConfirm(ApInfo *ap, Ieee80211PrimResultCode resultCode)
{
    auto confirm = new Ieee80211Prim_AssociateConfirm();
    confirm->setAddress(ap->address);
    sendConfirm(confirm, resultCode);
}

void Ieee80211MgmtSta::sendReassociationConfirm(ApInfo *ap, Ieee80211PrimResultCode resultCode)
{
    auto confirm = new Ieee80211Prim_ReassociateConfirm();
    confirm->setAddress(ap->address);
    sendConfirm(confirm, resultCode);
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
        cancelAndDelete(ap->authTimeoutMsg);
        ap->authTimeoutMsg = nullptr;
        sendAuthenticationConfirm(ap, PRC_REFUSED); // TODO or what resultCode?
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
        // TODO frame length could be increased to account for challenge text length etc.
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
        cancelAndDelete(ap->authTimeoutMsg);
        ap->authTimeoutMsg = nullptr;
        sendAuthenticationConfirm(ap, statusCodeToPrimResultCode(statusCode));
    }

    delete packet;
}

void Ieee80211MgmtSta::handleDeauthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Received Deauthentication frame\n";
    // IEEE Std 802.11-2024, 9.3.3.1: Address 2 is the transmitter/source
    // address used to identify the peer that sent this frame.
    const MacAddress& address = header->getTransmitterAddress();
    ApInfo *ap = lookupAP(address);
    ApInfo *pendingAp = assocTimeoutMsg ? static_cast<ApInfo *>(assocTimeoutMsg->getContextPointer()) : nullptr;
    bool isPendingAp = pendingAp != nullptr && pendingAp->address == address;
    bool isCurrentAp = mib->bssStationData.isAssociated && address == assocAP.address;

    // IEEE Std 802.11-2024, 11.3.4.5: deauthentication from the current AP
    // returns the STA to State 1. Do this before consulting the discovery
    // cache: association state itself is authoritative for this transition.
    if (isCurrentAp) {
        // Clear the cached authentication state before the shared helper emits
        // a possible same-peer transaction refusal, so observers see the
        // complete State-1 transition in that callback as well.
        if (ap != nullptr) {
            ap->isAuthenticated = false;
            if (ap->authTimeoutMsg) {
                cancelAndDelete(ap->authTimeoutMsg);
                ap->authTimeoutMsg = nullptr;
            }
        }
        ASSERT(terminateCurrentAssociationFromPeer(address));
        delete packet;
        return;
    }

    // IEEE Std 802.11-2024, 11.3.5.1, 11.3.5.2 and 11.3.5.4: a
    // deauthentication from the target AP terminates the pending association
    // or reassociation, even when that AP is not the current AP.
    if (isPendingAp) {
        bool pendingReassociation = reassociationInProgress;
        EV << "Cancelling pending association with deauthenticated AP\n";
        pendingAp->isAuthenticated = false;
        if (pendingAp->authTimeoutMsg) {
            cancelAndDelete(pendingAp->authTimeoutMsg);
            pendingAp->authTimeoutMsg = nullptr;
        }
        mib->removePeerHtCapabilities(address);
        cancelPendingAssociation();
        if (pendingReassociation)
            sendReassociationConfirm(pendingAp, PRC_REFUSED);
        else
            sendAssociationConfirm(pendingAp, PRC_REFUSED);
        delete packet;
        return;
    }

    if (!ap || !ap->isAuthenticated) {
        EV << "Unknown AP, or not authenticated with that AP -- ignoring frame\n";
        delete packet;
        return;
    }
    if (ap->authTimeoutMsg) {
        cancelAndDelete(ap->authTimeoutMsg);
        ap->authTimeoutMsg = nullptr;
        EV << "Cancelling pending authentication\n";
        delete packet;
        return;
    }

    EV << "Setting isAuthenticated flag for that AP to false\n";
    ap->isAuthenticated = false;
    mib->removePeerHtCapabilities(address);
    delete packet;
}

void Ieee80211MgmtSta::handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSta::handleAssociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    processAssociationResponse(packet, header, false);
}

void Ieee80211MgmtSta::processAssociationResponse(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header, bool reassociation)
{
    EV << "Received Association or Reassociation Response frame\n";

    if (!assocTimeoutMsg) {
        EV << "No association in progress, ignoring frame\n";
        delete packet;
        return;
    }

    // IEEE Std 802.11-2024, 11.3.5.2 and 11.3.5.4: process only the response
    // corresponding to the association or reassociation procedure in progress.
    if (reassociation != reassociationInProgress) {
        EV_INFO << "Association response subtype does not match the pending "
                << (reassociationInProgress ? "reassociation" : "association")
                << " attempt, ignoring frame\n";
        delete packet;
        return;
    }

    MacAddress address = header->getTransmitterAddress();
    ApInfo *ap = static_cast<ApInfo *>(assocTimeoutMsg->getContextPointer());
    if (ap == nullptr || ap->address != address) {
        EV << "Association response is not from the pending AP, ignoring frame\n";
        delete packet;
        return;
    }

    // extract frame contents
    const auto& responseBody = packet->peekData<Ieee80211AssociationResponseFrame>();
    int statusCode = responseBody->getStatusCode();

    HtAssociationResponseStatus responseHtStatus = HtAssociationResponseStatus::LEGACY;
    Ieee80211HtCapabilities responseHtCapabilities;
    Ieee80211HtOperation responseHtOperation;
    std::string responseHtReason;
    if (statusCode == SC_SUCCESSFUL)
        responseHtStatus = classifyAssociationResponse(responseBody, responseHtCapabilities, responseHtOperation, responseHtReason);
    delete packet;

    cancelPendingAssociation();

    if (statusCode != SC_SUCCESSFUL) {
        EV << "Association failed with AP address=" << ap->address << "\n";
        if (reassociation)
            handleReassociationFailure(ap);
        else if (!mib->bssStationData.isAssociated || assocAP.address != ap->address)
            mib->removePeerHtCapabilities(ap->address);
    }
    else {
        EV << "Association successful, AP address=" << ap->address << "\n";

        if (mib->bssStationData.isAssociated) {
            EV << "Breaking existing association with AP address=" << assocAP.address << "\n";
            mib->bssStationData.isAssociated = false;
            mib->removePeerHtCapabilities(assocAP.address);
            cancelAndDelete(assocAP.beaconTimeoutMsg);
            assocAP.beaconTimeoutMsg = nullptr;
            assocAP = AssociatedApInfo();
        }

        // change our state to "associated"
        mib->bssData.ssid = ap->ssid;
        mib->bssData.bssid = ap->address;
        mib->bssStationData.isAssociated = true;
        (ApInfo&)assocAP = (*ap);
        if (responseHtStatus == HtAssociationResponseStatus::VALID_HT)
            mib->setPeerHtCapabilities(ap->address, responseHtCapabilities, responseHtOperation);
        else {
            mib->removePeerHtCapabilities(ap->address);
            if (responseHtStatus == HtAssociationResponseStatus::INVALID_HT) {
                EV_WARN << "Association succeeded without usable HT negotiation with AP address=" << ap->address
                        << ": " << responseHtReason << "\n";
                HtNegotiationFailure notification;
                notification.setPeerAddress(ap->address);
                notification.setReassociation(reassociation);
                notification.setStatus(responseHtStatus);
                notification.setReason(responseHtReason);
                emit(htNegotiationFailedSignal, &notification);
            }
        }

        emit(l2AssociatedSignal, myIface, ap);

        assocAP.beaconTimeoutMsg = new cMessage("beaconTimeout", MK_BEACON_TIMEOUT);
        scheduleAfter(MAX_BEACONS_MISSED * assocAP.beaconInterval, assocAP.beaconTimeoutMsg);
    }

    // report back to agent
    if (reassociation)
        sendReassociationConfirm(ap, statusCodeToPrimResultCode(statusCode));
    else
        sendAssociationConfirm(ap, statusCodeToPrimResultCode(statusCode));
}

Ieee80211MgmtSta::HtAssociationResponseStatus Ieee80211MgmtSta::classifyAssociationResponse(
        const Ptr<const Ieee80211AssociationResponseFrame>& responseBody,
        Ieee80211HtCapabilities& responseHtCapabilities, Ieee80211HtOperation& responseHtOperation,
        std::string& reason) const
{
    bool responseHtCapabilitiesPresent = responseBody->getHtCapabilitiesPresent();
    bool responseHtOperationPresent = responseBody->getHtOperationPresent();
    // A non-HT station deliberately ignores HT elements. This preserves the
    // genuine legacy association path even when an HT-capable AP includes its
    // normal response elements.
    if (!mib->isHtOperationSupported())
        return HtAssociationResponseStatus::LEGACY;
    if (!responseHtCapabilitiesPresent && !responseHtOperationPresent)
        return HtAssociationResponseStatus::LEGACY;
    if (responseHtCapabilitiesPresent != responseHtOperationPresent) {
        reason = "association response contains only one of the HT Capabilities and HT Operation elements";
        return HtAssociationResponseStatus::INVALID_HT;
    }
    // Keep the existing deserialization behavior: malformed received HT
    // elements are rejected by makeHtCapabilities/makeHtOperation.
    responseHtCapabilities = makeHtCapabilities(responseBody->getHtCapabilities());
    responseHtOperation = makeHtOperation(responseBody->getHtOperation());
    auto negotiated = negotiateHtCapabilities(mib->localHtCapabilities, responseHtCapabilities, responseHtOperation);
    if (!supportsBasicHtMcsSet(mib->localHtCapabilities, responseHtOperation) ||
            !negotiated.localTxPeerRx.valid || !negotiated.localRxPeerTx.valid) {
        reason = "HT capabilities and operation have no bidirectionally usable common mode";
        return HtAssociationResponseStatus::INVALID_HT;
    }
    return HtAssociationResponseStatus::VALID_HT;
}

const char *Ieee80211MgmtSta::getHtAssociationResponseStatusName(HtAssociationResponseStatus status)
{
    switch (status) {
        case HtAssociationResponseStatus::LEGACY: return "LEGACY";
        case HtAssociationResponseStatus::VALID_HT: return "VALID_HT";
        case HtAssociationResponseStatus::INVALID_HT: return "INVALID_HT";
        default: return "UNKNOWN";
    }
}

void Ieee80211MgmtSta::handleReassociationFailure(ApInfo *ap)
{
    // IEEE Std 802.11-2024, 11.3.5.4(f): failed or timed-out reassociation
    // disassociates the STA only when the target is its current AP.
    if (shouldDisassociateOnReassociationFailure(mib->bssStationData.isAssociated, assocAP.address, ap->address))
        disassociate();
    else {
        mib->removePeerHtCapabilities(ap->address);
        if (mib->bssStationData.isAssociated)
            changeChannel(assocAP.channel);
    }
}

bool Ieee80211MgmtSta::shouldDisassociateOnReassociationFailure(bool isAssociated,
        const MacAddress& associatedApAddress, const MacAddress& targetApAddress)
{
    return isAssociated && associatedApAddress == targetApAddress;
}

void Ieee80211MgmtSta::handleReassociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    dropManagementFrame(packet);
}

void Ieee80211MgmtSta::handleReassociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    processAssociationResponse(packet, header, true);
}

void Ieee80211MgmtSta::handleDisassociationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header)
{
    EV << "Received Disassociation frame\n";
    // IEEE Std 802.11-2024, 9.3.3.1: Address 2 carries the transmitter (TA/SA).
    const MacAddress& address = header->getTransmitterAddress();

    // IEEE Std 802.11-2024, 11.3.5.7: process a Disassociation only from the
    // AP whose peer state is State 3/4. In particular, a pending association
    // to another AP must survive an unrelated frame.
    if (!terminateCurrentAssociationFromPeer(address)) {
        EV << "Not associated with that AP -- ignoring frame\n";
        delete packet;
        return;
    }
    delete packet;
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
        rescheduleAfter(MAX_BEACONS_MISSED * assocAP.beaconInterval, assocAP.beaconTimeoutMsg);

//        ApInfo *ap = lookupAP(frame->getTransmitterAddress());
//        ASSERT(ap!=nullptr);
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

    int legacyChannel = body->getChannelNumber();
    ap->address = address;
    ap->ssid = body->getSSID();
    ap->supportedRates = body->getSupportedRates();
    ap->extendedSupportedRatesPresent = body->getExtendedSupportedRatesPresent();
    ap->extendedSupportedRates = body->getExtendedSupportedRates();
    ap->htCapabilitiesPresent = body->getHtCapabilitiesPresent();
    if (ap->htCapabilitiesPresent)
        ap->htCapabilities = makeHtCapabilities(body->getHtCapabilities());
    ap->htOperationPresent = body->getHtOperationPresent();
    if (ap->htOperationPresent) {
        ap->htOperation = makeHtOperation(body->getHtOperation());
        // IEEE Std 802.11-2024, Table 9-230 and 11.14: the HT Operation
        // element is the authoritative advertisement of the BSS primary
        // channel. The fixed channelNumber field is not serialized by the
        // management-body serializer and may therefore be its default value.
        ap->channel = ap->htOperation.primaryChannel;
    }
    else
        ap->channel = legacyChannel;
    ap->beaconInterval = body->getBeaconInterval();
    auto signalPowerInd = packet->getTag<SignalPowerInd>();
    if (signalPowerInd != nullptr) {
        ap->rxPower = signalPowerInd->getPower().get<W>();
        if (ap->address == assocAP.address)
            assocAP.rxPower = ap->rxPower;
    }
}

} // namespace ieee80211
} // namespace inet
