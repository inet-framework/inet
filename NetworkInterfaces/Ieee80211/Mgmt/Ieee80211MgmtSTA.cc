//
// Copyright (C) 2006 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include "Ieee80211MgmtSTA.h"
#include "Ieee802Ctrl_m.h"
#include "NotifierConsts.h"
#include "PhyControlInfo_m.h"

//FIXME implement bitrate switching (involves notification of MAC, SnrEval, Decider)
//FIXME while scanning, discard all other requests
//FIXME beacons should overtake all other frames (inserted at front of queue)
//FIXME remove variables, message fields etc that we don't support

Define_Module(Ieee80211MgmtSTA);

// message kind values for timers
#define MK_AUTH_TIMEOUT   1
#define MK_ASSOC_TIMEOUT  2
#define MK_CHANNEL_CHANGE 3
#define MK_SEND_PROBE_REQ 4
#define MK_BEACON_TIMEOUT 5

#define MAX_BEACONS_MISSED 3.5  // beacon lost timeout, in beacon intervals (doesn't need to be integer)


std::ostream& operator<<(std::ostream& os, const Ieee80211MgmtSTA::APInfo& ap)
{
    os << "AP addr=" << ap.address
       << " chan=" << ap.channel
       << " ssid=" << ap.ssid
       //TBD supportedRates, capabilityInformation,...
       << " tstamp=" << ap.timestamp
       << " beaconIntvl=" << ap.beaconInterval
       << " rxPower=" << ap.rxPower
       << " authSeqExpected=" << ap.authSeqExpected
       << " isAuthenticated=" << ap.isAuthenticated
       << " authType=" << ap.authType
       << " rcvSeq=" << ap.receiveSequence;
    return os;
}

void Ieee80211MgmtSTA::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);
    if (stage==0)
    {
        isScanning = false;
        isAssociated = false;
        receiveSequence = 0;

        nb = NotificationBoardAccess().get();

        // determine numChannels (needed when we're told to scan "all" channels)
        //XXX find a better way than directly accessing channelControl
        cModule *cc = simulation.moduleByPath("channelcontrol");
        if (cc == NULL)
            error("cannot not find ChannelControl module");
        numChannels = cc->par("numChannels");

        WATCH(isScanning);
        WATCH(isAssociated);
        //TODO: watch "scanning"

        WATCH(apAddress);
        WATCH(receiveSequence);
        WATCH_LIST(apList);
    }
}

void Ieee80211MgmtSTA::handleTimer(cMessage *msg)
{
    if (msg->kind()==MK_AUTH_TIMEOUT)
    {
        // authentication timed out
        APInfo *ap = (APInfo *)msg->contextPointer();
        EV << "Authentication timed out, AP address = " << ap->address << "\n";

        // send back failure report to agent
        sendAuthenticationConfirm(ap, PRC_TIMEOUT);
    }
    else if (msg->kind()==MK_ASSOC_TIMEOUT)
    {
        // association timed out
        APInfo *ap = (APInfo *)msg->contextPointer();
        EV << "Association timed out, AP address = " << ap->address << "\n";

        // send back failure report to agent
        sendAssociationConfirm(ap, PRC_TIMEOUT);
    }
    else if (msg->kind()==MK_CHANNEL_CHANGE)
    {
        // go to next channel during scanning
        bool done = scanNextChannel(msg);
        if (done)
            sendScanConfirm(); // send back response to agents' "scan" command
    }
    else if (msg->kind()==MK_SEND_PROBE_REQ)
    {
        // send probe request during active scanning
        sendProbeRequest();
        delete msg;
    }
    else if (msg->kind()==MK_BEACON_TIMEOUT)
    {
        // missed a few consecutive beacons
        beaconLost();
    }
    else
    {
        error("internal error: unrecognized timer '%s'", msg->name());
    }
}

void Ieee80211MgmtSTA::handleUpperMessage(cMessage *msg)
{
    Ieee80211DataFrame *frame = encapsulate(msg);
    sendOrEnqueue(frame);
}

void Ieee80211MgmtSTA::handleCommand(int msgkind, cPolymorphic *ctrl)
{
    if (dynamic_cast<Ieee80211Prim_ScanRequest *>(ctrl))
        processScanCommand((Ieee80211Prim_ScanRequest *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_AuthenticateRequest *>(ctrl))
        processAuthenticateCommand((Ieee80211Prim_AuthenticateRequest *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_DeauthenticateRequest *>(ctrl))
        processDeauthenticateCommand((Ieee80211Prim_DeauthenticateRequest *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_AssociateRequest *>(ctrl))
        processAssociateCommand((Ieee80211Prim_AssociateRequest *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_ReassociateRequest *>(ctrl))
        processReassociateCommand((Ieee80211Prim_ReassociateRequest *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_DisassociateRequest *>(ctrl))
        processDisassociateCommand((Ieee80211Prim_DisassociateRequest *)ctrl);
    else if (ctrl)
        error("handleCommand(): unrecognized control info class `%s'", ctrl->className());
    else
        error("handleCommand(): control info is NULL");
    delete ctrl;
}

Ieee80211DataFrame *Ieee80211MgmtSTA::encapsulate(cMessage *msg)
{
    Ieee80211DataFrame *frame = new Ieee80211DataFrame(msg->name());

    // frame goes to the AP
    frame->setToDS(true);

    // receiver is the AP
    frame->setReceiverAddress(apAddress);

    // destination address is in address3
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    frame->setAddress3(ctrl->getDest());
    delete ctrl;

    frame->encapsulate(msg);
    return frame;
}

Ieee80211MgmtSTA::APInfo *Ieee80211MgmtSTA::lookupAP(const MACAddress& address)
{
    for (AccessPointList::iterator it=apList.begin(); it!=apList.end(); ++it)
        if (it->address == address)
            return &(*it);
    return NULL;
}

void Ieee80211MgmtSTA::clearAPList()
{
    for (AccessPointList::iterator it=apList.begin(); it!=apList.end(); ++it)
        if (it->timeoutMsg)
            delete cancelEvent(it->timeoutMsg);
    apList.clear();
}

void Ieee80211MgmtSTA::changeChannel(int channelNum)
{
    EV << "Tuning to channel " << channelNum << "\n";

    // sending PHY_C_CHANGECHANNEL command to MAC
    PhyControlInfo *phyCtrl = new PhyControlInfo();
    phyCtrl->setChannelNumber(channelNum);
    cMessage *msg = new cMessage("changeChannel", PHY_C_CHANGECHANNEL);
    msg->setControlInfo(phyCtrl);
    send(msg, "macOut");
}

void Ieee80211MgmtSTA::beaconLost()
{
    EV << "Missed a few consecutive beacons -- AP is considered lost\n";
    nb->fireChangeNotification(NF_L2_BEACON_LOST, NULL);  //FIXME use InterfaceEntry as detail, etc...

    //XXX break existing association with AP? send Disassociation frame?
    //XXX delete beaconTimeout timer?
}

void Ieee80211MgmtSTA::sendManagementFrame(Ieee80211ManagementFrame *frame, const MACAddress& address)
{
    // frame goes to the specified AP
    frame->setToDS(true);
    frame->setReceiverAddress(address);
    //XXX set sequenceNumber?

    sendOrEnqueue(frame); //XXX or should mgmt frames take priority over normal frames?
}

void Ieee80211MgmtSTA::startAuthentication(APInfo *ap, int authType, double timeout)
{
    changeChannel(ap->channel);
    ap->authType = authType;

    // create and send first authentication frame
    Ieee80211AuthenticationFrame *frame = new Ieee80211AuthenticationFrame("Auth");
    frame->getBody().setSequenceNumber(1);
    //XXX frame length could be increased to account for challenge text length etc.
    //XXX obey authType
    sendManagementFrame(frame, ap->address);

    // schedule timeout
    ASSERT(ap->timeoutMsg==NULL);
    ap->timeoutMsg = new cMessage("authTimeout", MK_AUTH_TIMEOUT);
    ap->timeoutMsg->setContextPointer(ap);
    scheduleAt(simTime()+timeout, ap->timeoutMsg);
}

void Ieee80211MgmtSTA::startAssociation(APInfo *ap, double timeout)
{
    //XXX check authentication status? or is it enough if AP does that?

    // switch to that channel
    changeChannel(ap->channel);

    // create and send association request
    Ieee80211AssociationRequestFrame *frame = new Ieee80211AssociationRequestFrame("Assoc");

    //XXX set the following too?
    // string SSID;
    // Ieee80211SupportedRatesElement supportedRates;
    // Ieee80211CapabilityInformation capabilityInformation;

    sendManagementFrame(frame, ap->address);

    // schedule timeout
    ASSERT(ap->timeoutMsg==NULL);
    ap->timeoutMsg = new cMessage("assocTimeout", MK_ASSOC_TIMEOUT);
    ap->timeoutMsg->setContextPointer(ap);
    scheduleAt(simTime()+timeout, ap->timeoutMsg);
}

void Ieee80211MgmtSTA::receiveChangeNotification(int category, cPolymorphic *details)
{
    Enter_Method_Silent();
    //TBD
}

void Ieee80211MgmtSTA::processScanCommand(Ieee80211Prim_ScanRequest *ctrl)
{
    EV << "Received Scan Request from agent, clearing AP list and starting scanning...\n";

    // clear existing AP list (and cancel any pending authentications) -- we want to start with a clean page
    clearAPList();

    // fill in scanning state
    ASSERT(ctrl->getBSSType()==BSSTYPE_INFRASTRUCTURE);
    scanning.bssid = ctrl->getBSSID().isUnspecified() ? MACAddress::BROADCAST_ADDRESS : ctrl->getBSSID();
    scanning.ssid = ctrl->getSSID();
    scanning.isActiveScan = ctrl->getActiveScan();
    scanning.probeDelay = ctrl->getProbeDelay();
    scanning.channelList.clear();
    scanning.minChannelTime = ctrl->getMinChannelTime();
    scanning.maxChannelTime = ctrl->getMaxChannelTime();
    ASSERT(scanning.probeDelay < scanning.minChannelTime);
    ASSERT(scanning.minChannelTime <= scanning.maxChannelTime);

    // channel list to scan (deafult: all channels)
    for (int i=0; i<ctrl->getChannelListArraySize(); i++)
        scanning.channelList.push_back(ctrl->getChannelList(i));
    if (scanning.channelList.empty())
        for (int i=0; i<numChannels; i++)
            scanning.channelList.push_back(i);

    // start scanning
    scanning.currentChannelIndex = -1; // so we'll start with index==0
    cMessage *timerMsg = new cMessage("nextChannelChange", MK_CHANNEL_CHANGE);
    scanNextChannel(timerMsg);
}

bool Ieee80211MgmtSTA::scanNextChannel(cMessage *reuseTimerMsg)
{
    // if we're already at the last channel, we're through
    if (scanning.currentChannelIndex==scanning.channelList.size()-1)
    {
        EV << "Finished scanning last channel\n";
        delete reuseTimerMsg;
        return true; // we're done
    }

    // tune to first channel and schedule next channel switch
    int newChannel = scanning.channelList[++scanning.currentChannelIndex];
    changeChannel(newChannel);
    double channelTime = (scanning.minChannelTime+scanning.maxChannelTime)/2;
    scheduleAt(simTime()+channelTime, reuseTimerMsg);

    if (scanning.isActiveScan)
        scheduleAt(simTime()+scanning.probeDelay, new cMessage("sendProbe", MK_SEND_PROBE_REQ));

    return false;
}

void Ieee80211MgmtSTA::sendProbeRequest()
{
    EV << "Sending Probe Request, BSSID=" << scanning.bssid << ", SSID=\"" << scanning.ssid << "\"\n";
    Ieee80211ProbeRequestFrame *frame = new Ieee80211ProbeRequestFrame("ProbeReq");
    frame->getBody().setSSID(scanning.ssid.c_str());
    sendManagementFrame(frame, scanning.bssid);
}

void Ieee80211MgmtSTA::sendScanConfirm()
{
    EV << "Scanning complete, found " << apList.size() << " APs, sending confirmation to agent\n";

    // copy apList contents into a ScanConfirm primitive and send it back
    int n = apList.size();
    Ieee80211Prim_ScanConfirm *confirm = new Ieee80211Prim_ScanConfirm();
    confirm->setBssListArraySize(n);
    AccessPointList::iterator it = apList.begin();
    //XXX filter for req'd bssid and ssid
    for (int i=0; i<n; i++, it++)
    {
        APInfo *ap = &(*it);
        Ieee80211Prim_BSSDescription& bss = confirm->getBssList(i);
        bss.setChannel(ap->channel);
        bss.setBSSID(ap->address);
        bss.setSSID(ap->ssid.c_str());
        bss.setSupportedRates(ap->supportedRates);
        bss.setCapabilityInfo(ap->capabilityInformation);
        bss.setBeaconInterval(ap->beaconInterval);
        bss.setRxPower(ap->rxPower);
    }
    sendConfirm(confirm, PRC_SUCCESS);
}

void Ieee80211MgmtSTA::processAuthenticateCommand(Ieee80211Prim_AuthenticateRequest *ctrl)
{
    const MACAddress& address = ctrl->getAddress();
    APInfo *ap = lookupAP(address);
    if (!ap)
        error("processAuthenticateCommand: AP not known: address = %s", address.str().c_str());
    startAuthentication(ap, ctrl->getAuthType(), ctrl->getTimeout());
}

void Ieee80211MgmtSTA::processDeauthenticateCommand(Ieee80211Prim_DeauthenticateRequest *ctrl)
{
    const MACAddress& address = ctrl->getAddress();
    APInfo *ap = lookupAP(address);
    if (!ap)
        error("processDeauthenticateCommand: AP not known: address = %s", address.str().c_str());

    if (ap->isAuthenticated)
    {
        //XXX what if we are not authenticated?
        ap->isAuthenticated = false;
    }

    // create and send deauthentication request
    Ieee80211DeauthenticationFrame *frame = new Ieee80211DeauthenticationFrame("Deauth");
    frame->getBody().setReasonCode(ctrl->getReasonCode());
    sendManagementFrame(frame, address);

    // send confirm to agent
    sendConfirm(new Ieee80211Prim_DeauthenticateConfirm(), PRC_SUCCESS);
}

void Ieee80211MgmtSTA::processAssociateCommand(Ieee80211Prim_AssociateRequest *ctrl)
{
    const MACAddress& address = ctrl->getAddress();
    APInfo *ap = lookupAP(address);
    if (!ap)
        error("processAssociateCommand: AP not known: address = %s", address.str().c_str());
    if (!ap->isAuthenticated)
        error("processAssociateCommand: not authenticated with AP address = %s", address.str().c_str());
    startAssociation(ap, ctrl->getTimeout());
}

void Ieee80211MgmtSTA::processReassociateCommand(Ieee80211Prim_ReassociateRequest *ctrl)
{
    // treat the same way as association
    processAssociateCommand(ctrl);
}

void Ieee80211MgmtSTA::processDisassociateCommand(Ieee80211Prim_DisassociateRequest *ctrl)
{
    const MACAddress& address = ctrl->getAddress();

    //XXX what if we are not associated?
    //XXX what if address is not our associated AP address??
    if (isAssociated)
    {
        isAssociated = false;
        delete cancelEvent(beaconTimeout);
    }

    // create and send disassociation request
    Ieee80211DisassociationFrame *frame = new Ieee80211DisassociationFrame("Disass");
    frame->getBody().setReasonCode(ctrl->getReasonCode());
    sendManagementFrame(frame, address);

    // send confirm to agent
    sendConfirm(new Ieee80211Prim_DisassociateConfirm(), PRC_SUCCESS);
}

void Ieee80211MgmtSTA::sendAuthenticationConfirm(APInfo *ap, int resultCode)
{
    Ieee80211Prim_AuthenticateConfirm *confirm = new Ieee80211Prim_AuthenticateConfirm();
    confirm->setAddress(ap->address);
    confirm->setAuthType(ap->authType);
    sendConfirm(confirm, resultCode);
}

void Ieee80211MgmtSTA::sendAssociationConfirm(APInfo *ap, int resultCode)
{
    sendConfirm(new Ieee80211Prim_AssociateConfirm(), resultCode);
}

void Ieee80211MgmtSTA::sendConfirm(Ieee80211PrimConfirm *confirm, int resultCode)
{
    confirm->setResultCode(resultCode);
    cMessage *msg = new cMessage(confirm->className());
    msg->setControlInfo(confirm);
    send(msg, "agentOut");
}

int Ieee80211MgmtSTA::statusCodeToPrimResultCode(int statusCode)
{
    return statusCode==SC_SUCCESSFUL ? PRC_SUCCESS : PRC_REFUSED;
}

void Ieee80211MgmtSTA::handleDataFrame(Ieee80211DataFrame *frame)
{
    sendUp(decapsulate(frame));
}

void Ieee80211MgmtSTA::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    EV << "Received Authentication frame\n";

    MACAddress address = frame->getTransmitterAddress();
    APInfo *ap = lookupAP(address);
    if (!ap)
    {
        EV << "AP not known (address=" << address.str() << "), discarding authentication frame\n";
        delete frame;
        return;
    }

    //XXX what if already authenticated with AP?
    //XXX check authentication is currently in progress
    //XXX check sequenceNumber if in sequence
/*
    // check authentication sequence number is OK
    int frameAuthSeq = frame->getBody().getSequenceNumber();
    if (frameAuthSeq != ap->authSeqExpected)
    {
        // wrong sequence number: send error and return
        Ieee80211AuthenticationFrame *resp = new Ieee80211AuthenticationFrame("Auth(ERR)");
        resp->getBody().setStatusCode(SC_AUTH_OUT_OF_SEQ);
        sendManagementFrame(resp, frame->getTransmitterAddress());
        delete frame;
        ap->authSeqExpected = 1; // go back to start square XXX
        //XXX send error to agent
        return;
    }
*/
    //XXX how many exchanges are needed for auth to be complete? (check seqNum)
    int statusCode = frame->getBody().getStatusCode();

    if (statusCode!=SC_SUCCESSFUL)
        EV << "Authentication failed with AP address=" << ap->address << "\n";

    //TBD if auth complete, set isAuthenticated=true and send back response
/*
    if (frameAuthSeq < numAuthSteps)
    {
        // send OK response (we don't model the cryptography part, just assume
        // successful authentication every time)
        Ieee80211AuthenticationFrame *resp = new Ieee80211AuthenticationFrame("Auth(OK)");
        resp->getBody().setSequenceNumber(frameAuthSeq);
        resp->getBody().setStatusCode(SC_SUCCESSFUL);
        // XXX frame length could be increased to account for challenge text length etc.
        sendManagementFrame(resp, ap->address);
    }

    // we are authenticated with AP if we made it through the required number of steps
    if (frameAuthSeq+1 == numAuthSteps)
    {
        ap->isAuthenticated = true;
    }
    else
    {
        ap->authSeqExpected += 2;
    }
*/
    //XXX the following should only be done only if completed numSteps
    ap->isAuthenticated = (statusCode==SC_SUCCESSFUL);
    delete cancelEvent(ap->timeoutMsg);
    ap->timeoutMsg = NULL;
    sendAuthenticationConfirm(ap, statusCodeToPrimResultCode(statusCode));
}

void Ieee80211MgmtSTA::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    EV << "Received Deauthentication frame\n";
    //TBD
}

void Ieee80211MgmtSTA::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTA::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    EV << "Received Association Response frame\n";

    // extract frame contents
    MACAddress address = frame->getTransmitterAddress();
    int statusCode = frame->getBody().getStatusCode();
    //XXX Ieee80211CapabilityInformation capabilityInformation;
    //XXX short aid;
    //XXX Ieee80211SupportedRatesElement supportedRates;
    delete frame;

    // look up AP data structure
    APInfo *ap = lookupAP(address);
    if (!ap)
        error("handleAssociationResponseFrame: AP not known: address=%s", address.str().c_str());

    if (isAssociated)
    {
        //XXX something to do about our existing association...?
    }

    //XXX check if we have actually sent an AssocReq to this AP!
    delete cancelEvent(ap->timeoutMsg);
    ap->timeoutMsg = NULL;

    if (statusCode!=SC_SUCCESSFUL)
    {
        EV << "Association failed with AP address=" << ap->address << "\n";
    }
    else
    {
        EV << "Association successful, AP address=" << ap->address << "\n";

        // change our state to "associated"
        isAssociated = true;
        apAddress = address;
        receiveSequence = 1; //XXX ???

        nb->fireChangeNotification(NF_L2_ASSOCIATED, NULL); //XXX detail: InterfaceEntry?

        beaconTimeout = new cMessage("beaconTimeout", MK_BEACON_TIMEOUT);
        scheduleAt(simTime()+MAX_BEACONS_MISSED*beaconInterval, beaconTimeout);
    }

    // report back to agent
    sendAssociationConfirm(ap, statusCodeToPrimResultCode(statusCode));
}

void Ieee80211MgmtSTA::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTA::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    EV << "Received Reassociation Response frame\n";
    //TBD handle with the same code as Association Response????
}

void Ieee80211MgmtSTA::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    EV << "Received Disassociation frame\n";
    //TBD
}

void Ieee80211MgmtSTA::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    EV << "Received Beacon frame\n";
    storeAPInfo(frame->getTransmitterAddress(), frame->getBody());

    // if it is out associate AP, restart beacon timeout
    if (isAssociated && frame->getTransmitterAddress()==apAddress)
    {
        EV << "Beacon is from associated AP, restarting beacon timeout timer\n";
        ASSERT(beaconTimeout!=NULL);
        cancelEvent(beaconTimeout);
        scheduleAt(simTime()+MAX_BEACONS_MISSED*beaconInterval, beaconTimeout);

        //APInfo *ap = lookupAP(frame->getTransmitterAddress());
        //ASSERT(ap!=NULL);
    }

    delete frame;
}

void Ieee80211MgmtSTA::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtSTA::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    EV << "Received Probe Response frame\n";
    storeAPInfo(frame->getTransmitterAddress(), frame->getBody());
    delete frame;
}

void Ieee80211MgmtSTA::storeAPInfo(const MACAddress& address, const Ieee80211BeaconFrameBody& body)
{
    APInfo *ap = lookupAP(address);
    if (ap)
    {
        EV << "AP address=" << address << ", SSID=" << body.getSSID() << " already in our AP list, refreshing the info\n";
    }
    else
    {
        EV << "Inserting AP address=" << address << ", SSID=" << body.getSSID() << " into our AP list\n";
        apList.push_back(APInfo());
        ap = &apList.back();
    }

    ap->channel = body.getChannel();
    ap->address = address;
    ap->ssid = body.getSSID();
    ap->supportedRates = body.getSupportedRates();
    ap->capabilityInformation = body.getCapabilityInformation();
    ap->timestamp = body.getTimestamp();
    ap->beaconInterval = body.getBeaconInterval();

    //XXX where to get this from???    ap->rxPower = ...;
}

