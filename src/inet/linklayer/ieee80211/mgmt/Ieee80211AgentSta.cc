//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mgmt/Ieee80211AgentSta.h"

#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211Primitives_m.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211AgentSta);

#define MK_STARTUP    1

simsignal_t Ieee80211AgentSta::sentRequestSignal = registerSignal("sentRequest");
simsignal_t Ieee80211AgentSta::acceptConfirmSignal = registerSignal("acceptConfirm");
simsignal_t Ieee80211AgentSta::dropConfirmSignal = registerSignal("dropConfirm");

void Ieee80211AgentSta::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // read parameters
        activeScan = par("activeScan");
        probeDelay = par("probeDelay");
        minChannelTime = par("minChannelTime");
        maxChannelTime = par("maxChannelTime");
        authenticationTimeout = par("authenticationTimeout");
        associationTimeout = par("associationTimeout");
        cStringTokenizer tokenizer(par("channelsToScan"));
        const char *token;
        while ((token = tokenizer.nextToken()) != nullptr)
            channelsToScan.push_back(atoi(token));

        cModule *host = getContainingNode(this);
        host->subscribe(l2BeaconLostSignal, this);

        // JcM add: get the default ssid, if there is one.
        defaultSsid = par("defaultSsid").stdstringValue();

        // start up: send scan request
        simtime_t startingTime = par("startingTime");
        if (startingTime < SIMTIME_ZERO)
            startingTime = uniform(SIMTIME_ZERO, maxChannelTime);
        scheduleAfter(startingTime, new cMessage("startUp", MK_STARTUP));

        myIface = nullptr;
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        if (ift) {
            myIface = getContainingNicModule(this);
        }
    }
}

void Ieee80211AgentSta::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else
        handleResponse(msg);
}

void Ieee80211AgentSta::handleTimer(cMessage *msg)
{
    if (msg->getKind() == MK_STARTUP) {
        EV << "Starting up\n";
        sendScanRequest();
        delete msg;
    }
    else {
        throw cRuntimeError("internal error: unrecognized timer '%s'", msg->getName());
    }
}

void Ieee80211AgentSta::handleResponse(cMessage *msg)
{
    cObject *ctrl = msg->removeControlInfo();
    delete msg;

    EV << "Processing confirmation from mgmt: " << ctrl->getClassName() << "\n";

    if (auto ptr = dynamic_cast<Ieee80211Prim_ScanConfirm *>(ctrl))
        processScanConfirm(ptr);
    else if (auto ptr = dynamic_cast<Ieee80211Prim_AuthenticateConfirm *>(ctrl))
        processAuthenticateConfirm(ptr);
    else if (auto ptr = dynamic_cast<Ieee80211Prim_AssociateConfirm *>(ctrl))
        processAssociateConfirm(ptr);
    else if (auto ptr = dynamic_cast<Ieee80211Prim_ReassociateConfirm *>(ctrl))
        processReassociateConfirm(ptr);
    else if (ctrl)
        throw cRuntimeError("handleResponse(): unrecognized control info class `%s'", ctrl->getClassName());
    else
        throw cRuntimeError("handleResponse(): control info is nullptr");
    delete ctrl;
}

void Ieee80211AgentSta::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    printSignalBanner(signalID, obj, details);

    if (signalID == l2BeaconLostSignal) {
        // TODO should check details if it's about this NIC
        EV << "beacon lost, starting scanning again\n";
        getContainingNode(this)->bubble("Beacon lost!");
//        sendDisassociateRequest();
        sendScanRequest();
        emit(l2DisassociatedSignal, myIface);
    }
}

void Ieee80211AgentSta::sendRequest(Ieee80211PrimRequest *req)
{
    cMessage *msg = new cMessage(req->getClassName());
    msg->setControlInfo(req);
    send(msg, "mgmtOut");
}

void Ieee80211AgentSta::sendScanRequest()
{
    EV << "Sending ScanRequest primitive to mgmt\n";
    Ieee80211Prim_ScanRequest *req = new Ieee80211Prim_ScanRequest();
    req->setBSSType(BSSTYPE_INFRASTRUCTURE);
    req->setActiveScan(activeScan);
    req->setProbeDelay(probeDelay);
    req->setMinChannelTime(minChannelTime);
    req->setMaxChannelTime(maxChannelTime);
    req->setChannelListArraySize(channelsToScan.size());
    for (size_t i = 0; i < channelsToScan.size(); i++)
        req->setChannelList(i, channelsToScan[i]);
    // TODO BSSID, SSID are left at default ("any")

    emit(sentRequestSignal, PR_SCAN_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSta::sendAuthenticateRequest(const MacAddress& address)
{
    EV << "Sending AuthenticateRequest primitive to mgmt\n";
    Ieee80211Prim_AuthenticateRequest *req = new Ieee80211Prim_AuthenticateRequest();
    req->setAddress(address);
    req->setTimeout(authenticationTimeout);
    emit(sentRequestSignal, PR_AUTHENTICATE_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSta::sendDeauthenticateRequest(const MacAddress& address, Ieee80211ReasonCode reasonCode)
{
    EV << "Sending DeauthenticateRequest primitive to mgmt\n";
    Ieee80211Prim_DeauthenticateRequest *req = new Ieee80211Prim_DeauthenticateRequest();
    req->setAddress(address);
    req->setReasonCode(reasonCode);
    emit(sentRequestSignal, PR_DEAUTHENTICATE_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSta::sendAssociateRequest(const MacAddress& address)
{
    EV << "Sending AssociateRequest primitive to mgmt\n";
    Ieee80211Prim_AssociateRequest *req = new Ieee80211Prim_AssociateRequest();
    req->setAddress(address);
    req->setTimeout(associationTimeout);
    emit(sentRequestSignal, PR_ASSOCIATE_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSta::sendReassociateRequest(const MacAddress& address)
{
    EV << "Sending ReassociateRequest primitive to mgmt\n";
    Ieee80211Prim_ReassociateRequest *req = new Ieee80211Prim_ReassociateRequest();
    req->setAddress(address);
    req->setTimeout(associationTimeout);
    emit(sentRequestSignal, PR_REASSOCIATE_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSta::sendDisassociateRequest(const MacAddress& address, Ieee80211ReasonCode reasonCode)
{
    EV << "Sending DisassociateRequest primitive to mgmt\n";
    Ieee80211Prim_DisassociateRequest *req = new Ieee80211Prim_DisassociateRequest();
    req->setAddress(address);
    req->setReasonCode(reasonCode);
    emit(sentRequestSignal, PR_DISASSOCIATE_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSta::processScanConfirm(Ieee80211Prim_ScanConfirm *resp)
{
    // choose best AP

    int bssIndex = -1;
    if (this->defaultSsid == "") {
        // no default ssid, so pick the best one
        bssIndex = chooseBSS(resp);
    }
    else {
        // search if the defaultSsid is in the list, otherwise
        // keep searching.
        for (size_t i = 0; i < resp->getBssListArraySize(); i++) {
            std::string resp_ssid = resp->getBssList(i).getSSID();
            if (resp_ssid == this->defaultSsid) {
                EV << "found default SSID " << resp_ssid << endl;
                bssIndex = i;
                break;
            }
        }
    }

    if (bssIndex == -1) {
        EV << "No (suitable) AP found, continue scanning\n";
        emit(dropConfirmSignal, PR_SCAN_CONFIRM);
        sendScanRequest();
        return;
    }

    dumpAPList(resp);
    emit(acceptConfirmSignal, PR_SCAN_CONFIRM);

    const Ieee80211Prim_BssDescription& bssDesc = resp->getBssList(bssIndex);
    EV << "Chosen AP address=" << bssDesc.getBSSID() << " from list, starting authentication\n";
    sendAuthenticateRequest(bssDesc.getBSSID());
}

void Ieee80211AgentSta::dumpAPList(Ieee80211Prim_ScanConfirm *resp)
{
    EV << "Received AP list:\n";
    for (size_t i = 0; i < resp->getBssListArraySize(); i++) {
        const Ieee80211Prim_BssDescription& bssDesc = resp->getBssList(i);
        EV << "    " << i << ". "
           << " address=" << bssDesc.getBSSID()
           << " channel=" << bssDesc.getChannelNumber()
           << " SSID=" << bssDesc.getSSID()
           << " beaconIntvl=" << bssDesc.getBeaconInterval()
           << " rxPower=" << bssDesc.getRxPower()
           << endl;
        // later: supportedRates
    }
}

int Ieee80211AgentSta::chooseBSS(Ieee80211Prim_ScanConfirm *resp)
{
    if (resp->getBssListArraySize() == 0)
        return -1;

    // here, just choose the one with the greatest receive power
    // TODO and which supports a good data rate we support
    int bestIndex = 0;
    for (size_t i = 0; i < resp->getBssListArraySize(); i++)
        if (resp->getBssList(i).getRxPower() > resp->getBssList(bestIndex).getRxPower())
            bestIndex = i;

    return bestIndex;
}

void Ieee80211AgentSta::processAuthenticateConfirm(Ieee80211Prim_AuthenticateConfirm *resp)
{
    if (resp->getResultCode() != PRC_SUCCESS) {
        EV << "Authentication error\n";
        emit(dropConfirmSignal, PR_AUTHENTICATE_CONFIRM);

        // try scanning again, maybe we'll have better luck next time, possibly with a different AP
        EV << "Going back to scanning\n";
        sendScanRequest();
    }
    else {
        EV << "Authentication successful, let's try to associate\n";
        emit(acceptConfirmSignal, PR_AUTHENTICATE_CONFIRM);
        sendAssociateRequest(resp->getAddress());
    }
}

void Ieee80211AgentSta::processAssociateConfirm(Ieee80211Prim_AssociateConfirm *resp)
{
    if (resp->getResultCode() != PRC_SUCCESS) {
        EV << "Association error\n";
        emit(dropConfirmSignal, PR_ASSOCIATE_CONFIRM);

        // try scanning again, maybe we'll have better luck next time, possibly with a different AP
        EV << "Going back to scanning\n";
        sendScanRequest();
    }
    else {
        EV << "Association successful\n";
        emit(acceptConfirmSignal, PR_ASSOCIATE_CONFIRM);
        // we are happy!
        getContainingNode(this)->bubble("Associated with AP");
        if (prevAP.isUnspecified() || prevAP != resp->getAddress()) {
            emit(l2AssociatedNewApSignal, myIface); // TODO detail: NetworkInterface?
            prevAP = resp->getAddress();
        }
        else
            emit(l2AssociatedOldApSignal, myIface);
    }
}

void Ieee80211AgentSta::processReassociateConfirm(Ieee80211Prim_ReassociateConfirm *resp)
{
    // treat the same way as AssociateConfirm
    if (resp->getResultCode() != PRC_SUCCESS) {
        EV << "Reassociation error\n";
        emit(dropConfirmSignal, PR_REASSOCIATE_CONFIRM);
        EV << "Going back to scanning\n";
        sendScanRequest();
    }
    else {
        EV << "Reassociation successful\n";
        emit(l2AssociatedOldApSignal, myIface); // TODO detail: NetworkInterface?
        emit(acceptConfirmSignal, PR_REASSOCIATE_CONFIRM);
        // we are happy!
    }
}

} // namespace ieee80211

} // namespace inet

