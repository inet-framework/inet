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

#include "inet/linklayer/ieee80211/mgmt/Ieee80211AgentSTA.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211Primitives_m.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/NotifierConsts.h"
#include "inet/common/INETUtils.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211AgentSTA);

#define MK_STARTUP    1

simsignal_t Ieee80211AgentSTA::sentRequestSignal = registerSignal("sentRequest");
simsignal_t Ieee80211AgentSTA::acceptConfirmSignal = registerSignal("acceptConfirm");
simsignal_t Ieee80211AgentSTA::dropConfirmSignal = registerSignal("dropConfirm");

void Ieee80211AgentSTA::initialize(int stage)
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
        while ((token = tokenizer.nextToken()) != NULL)
            channelsToScan.push_back(atoi(token));

        cModule *host = getContainingNode(this);
        host->subscribe(NF_L2_BEACON_LOST, this);

        // JcM add: get the default ssid, if there is one.
        default_ssid = par("default_ssid").stringValue();

        // start up: send scan request
        simtime_t startingTime = par("startingTime").doubleValue();
        if (startingTime < SIMTIME_ZERO)
            startingTime = uniform(SIMTIME_ZERO, maxChannelTime);
        scheduleAt(simTime() + startingTime, new cMessage("startUp", MK_STARTUP));

        myIface = NULL;
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        if (ift) {
            myIface = ift->getInterfaceByName(utils::stripnonalnum(findModuleUnderContainingNode(this)->getFullName()).c_str());
        }
    }
}

void Ieee80211AgentSTA::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        handleTimer(msg);
    else
        handleResponse(msg);
}

void Ieee80211AgentSTA::handleTimer(cMessage *msg)
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

void Ieee80211AgentSTA::handleResponse(cMessage *msg)
{
    cObject *ctrl = msg->removeControlInfo();
    delete msg;

    EV << "Processing confirmation from mgmt: " << ctrl->getClassName() << "\n";

    if (dynamic_cast<Ieee80211Prim_ScanConfirm *>(ctrl))
        processScanConfirm((Ieee80211Prim_ScanConfirm *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_AuthenticateConfirm *>(ctrl))
        processAuthenticateConfirm((Ieee80211Prim_AuthenticateConfirm *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_AssociateConfirm *>(ctrl))
        processAssociateConfirm((Ieee80211Prim_AssociateConfirm *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_ReassociateConfirm *>(ctrl))
        processReassociateConfirm((Ieee80211Prim_ReassociateConfirm *)ctrl);
    else if (ctrl)
        throw cRuntimeError("handleResponse(): unrecognized control info class `%s'", ctrl->getClassName());
    else
        throw cRuntimeError("handleResponse(): control info is NULL");
    delete ctrl;
}

void Ieee80211AgentSTA::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);

    if (signalID == NF_L2_BEACON_LOST) {
        //XXX should check details if it's about this NIC
        EV << "beacon lost, starting scanning again\n";
        getParentModule()->getParentModule()->bubble("Beacon lost!");
        //sendDisassociateRequest();
        sendScanRequest();
        emit(NF_L2_DISASSOCIATED, myIface);
    }
}

void Ieee80211AgentSTA::sendRequest(Ieee80211PrimRequest *req)
{
    cMessage *msg = new cMessage(req->getClassName());
    msg->setControlInfo(req);
    send(msg, "mgmtOut");
}

void Ieee80211AgentSTA::sendScanRequest()
{
    EV << "Sending ScanRequest primitive to mgmt\n";
    Ieee80211Prim_ScanRequest *req = new Ieee80211Prim_ScanRequest();
    req->setBSSType(BSSTYPE_INFRASTRUCTURE);
    req->setActiveScan(activeScan);
    req->setProbeDelay(probeDelay);
    req->setMinChannelTime(minChannelTime);
    req->setMaxChannelTime(maxChannelTime);
    req->setChannelListArraySize(channelsToScan.size());
    for (int i = 0; i < (int)channelsToScan.size(); i++)
        req->setChannelList(i, channelsToScan[i]);
    //XXX BSSID, SSID are left at default ("any")

    emit(sentRequestSignal, PR_SCAN_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSTA::sendAuthenticateRequest(const MACAddress& address)
{
    EV << "Sending AuthenticateRequest primitive to mgmt\n";
    Ieee80211Prim_AuthenticateRequest *req = new Ieee80211Prim_AuthenticateRequest();
    req->setAddress(address);
    req->setTimeout(authenticationTimeout);
    emit(sentRequestSignal, PR_AUTHENTICATE_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSTA::sendDeauthenticateRequest(const MACAddress& address, int reasonCode)
{
    EV << "Sending DeauthenticateRequest primitive to mgmt\n";
    Ieee80211Prim_DeauthenticateRequest *req = new Ieee80211Prim_DeauthenticateRequest();
    req->setAddress(address);
    req->setReasonCode(reasonCode);
    emit(sentRequestSignal, PR_DEAUTHENTICATE_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSTA::sendAssociateRequest(const MACAddress& address)
{
    EV << "Sending AssociateRequest primitive to mgmt\n";
    Ieee80211Prim_AssociateRequest *req = new Ieee80211Prim_AssociateRequest();
    req->setAddress(address);
    req->setTimeout(associationTimeout);
    emit(sentRequestSignal, PR_ASSOCIATE_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSTA::sendReassociateRequest(const MACAddress& address)
{
    EV << "Sending ReassociateRequest primitive to mgmt\n";
    Ieee80211Prim_ReassociateRequest *req = new Ieee80211Prim_ReassociateRequest();
    req->setAddress(address);
    req->setTimeout(associationTimeout);
    emit(sentRequestSignal, PR_REASSOCIATE_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSTA::sendDisassociateRequest(const MACAddress& address, int reasonCode)
{
    EV << "Sending DisassociateRequest primitive to mgmt\n";
    Ieee80211Prim_DisassociateRequest *req = new Ieee80211Prim_DisassociateRequest();
    req->setAddress(address);
    req->setReasonCode(reasonCode);
    emit(sentRequestSignal, PR_DISASSOCIATE_REQUEST);
    sendRequest(req);
}

void Ieee80211AgentSTA::processScanConfirm(Ieee80211Prim_ScanConfirm *resp)
{
    // choose best AP

    int bssIndex = -1;
    if (this->default_ssid == "") {
        // no default ssid, so pick the best one
        bssIndex = chooseBSS(resp);
    }
    else {
        // search if the default_ssid is in the list, otherwise
        // keep searching.
        for (int i = 0; i < (int)resp->getBssListArraySize(); i++) {
            std::string resp_ssid = resp->getBssList(i).getSSID();
            if (resp_ssid == this->default_ssid) {
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

    Ieee80211Prim_BSSDescription& bssDesc = resp->getBssList(bssIndex);
    EV << "Chosen AP address=" << bssDesc.getBSSID() << " from list, starting authentication\n";
    sendAuthenticateRequest(bssDesc.getBSSID());
}

void Ieee80211AgentSTA::dumpAPList(Ieee80211Prim_ScanConfirm *resp)
{
    EV << "Received AP list:\n";
    for (int i = 0; i < (int)resp->getBssListArraySize(); i++) {
        Ieee80211Prim_BSSDescription& bssDesc = resp->getBssList(i);
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

int Ieee80211AgentSTA::chooseBSS(Ieee80211Prim_ScanConfirm *resp)
{
    if (resp->getBssListArraySize() == 0)
        return -1;

    // here, just choose the one with the greatest receive power
    // TODO and which supports a good data rate we support
    int bestIndex = 0;
    for (int i = 0; i < (int)resp->getBssListArraySize(); i++)
        if (resp->getBssList(i).getRxPower() > resp->getBssList(bestIndex).getRxPower())
            bestIndex = i;

    return bestIndex;
}

void Ieee80211AgentSTA::processAuthenticateConfirm(Ieee80211Prim_AuthenticateConfirm *resp)
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

void Ieee80211AgentSTA::processAssociateConfirm(Ieee80211Prim_AssociateConfirm *resp)
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
        getParentModule()->getParentModule()->bubble("Associated with AP");
        if (prevAP.isUnspecified() || prevAP != resp->getAddress()) {
            emit(NF_L2_ASSOCIATED_NEWAP, myIface);    //XXX detail: InterfaceEntry?
            prevAP = resp->getAddress();
        }
        else
            emit(NF_L2_ASSOCIATED_OLDAP, myIface);
    }
}

void Ieee80211AgentSTA::processReassociateConfirm(Ieee80211Prim_ReassociateConfirm *resp)
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
        emit(NF_L2_ASSOCIATED_OLDAP, myIface);    //XXX detail: InterfaceEntry?
        emit(acceptConfirmSignal, PR_REASSOCIATE_CONFIRM);
        // we are happy!
    }
}

} // namespace ieee80211

} // namespace inet

