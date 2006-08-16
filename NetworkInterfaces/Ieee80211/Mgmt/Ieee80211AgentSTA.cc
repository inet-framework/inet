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


#include "Ieee80211AgentSTA.h"
#include "Ieee80211Primitives_m.h"


#define DEFAULT_AUTH_TIMEOUT    120 /*sec*/
#define DEFAULT_ASSOC_TIMEOUT   120 /*sec*/


Define_Module(Ieee80211AgentSTA);


void Ieee80211AgentSTA::initialize(int stage)
{
    //TODO schedule scan request?...
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
    //...
    error("internal error: unrecognized timer '%s'", msg->name());
}

void Ieee80211AgentSTA::handleResponse(cMessage *msg)
{
    cPolymorphic *ctrl = msg->removeControlInfo();
    delete msg;

    if (dynamic_cast<Ieee80211Prim_ScanConfirm *>(ctrl))
        processScanConfirm((Ieee80211Prim_ScanConfirm *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_AuthenticateConfirm *>(ctrl))
        processAuthenticateConfirm((Ieee80211Prim_AuthenticateConfirm *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_AssociateConfirm *>(ctrl))
        processAssociateConfirm((Ieee80211Prim_AssociateConfirm *)ctrl);
    else if (ctrl)
        error("handleResponse(): unrecognized control info class `%s'", ctrl->className());
    else
        error("handleResponse(): control info is NULL");
    delete ctrl;
}

void Ieee80211AgentSTA::sendRequest(Ieee80211Prim *req)
{
    cMessage *msg = new cMessage(req->className());
    msg->setControlInfo(req);
    send(msg, "mgmtOut");
}


void Ieee80211AgentSTA::sendScanRequest()
{
    Ieee80211Prim_ScanRequest *req = new Ieee80211Prim_ScanRequest();

    //TODO req params -- should come from module parameters?
    // bool BSSType;
    // MACAddress BSSID;
    // string SSID;
    // bool activeScan;
    // double probeDelay;
    // int channelList[];
    // double minChannelTime;
    // double maxChannelTime;

    sendRequest(req);
}

void Ieee80211AgentSTA::sendAuthenticateRequest(const MACAddress& address, int authType)
{
    Ieee80211Prim_AuthenticateRequest *req = new Ieee80211Prim_AuthenticateRequest();
    req->setAddress(address);
    req->setAuthType(authType);
    req->setTimeout(DEFAULT_AUTH_TIMEOUT);
    sendRequest(req);
}

void Ieee80211AgentSTA::sendAssociateRequest(const MACAddress& address)
{
    Ieee80211Prim_AssociateRequest *req = new Ieee80211Prim_AssociateRequest();
    req->setAddress(address);
    req->setTimeout(DEFAULT_ASSOC_TIMEOUT);
    //XXX    Ieee80211CapabilityInformation capabilityInfo;
    //XXX    int listenInterval; // unsupported by MAC
    sendRequest(req);
}

void Ieee80211AgentSTA::processScanConfirm(Ieee80211Prim_ScanConfirm *resp)
{
    // choose best AP
    int bssIndex = chooseBSS(resp);
    Ieee80211Prim_BSSDescription& bssDesc = resp->getBssList(bssIndex);
    sendAuthenticateRequest(bssDesc.getBSSID(), AUTHTYPE_SHAREDKEY); //XXX or AUTHTYPE_OPENSYSTEM -- should be parameter?
}

int Ieee80211AgentSTA::chooseBSS(Ieee80211Prim_ScanConfirm *resp)
{
    // here, just choose the one with the greatest receive power
    // TODO and which supports a good data rate we support
    int bestIndex = 0;
    for (int i=0; i<resp->getBssListArraySize(); i++)
        if (resp->getBssList(i).getRxPower() > resp->getBssList(bestIndex).getRxPower())
            bestIndex = i;
    return bestIndex;
}

void Ieee80211AgentSTA::processAuthenticateConfirm(Ieee80211Prim_AuthenticateConfirm *resp)
{
    //XXX
}

void Ieee80211AgentSTA::processAssociateConfirm(Ieee80211Prim_AssociateConfirm *resp)
{
    //XXX
}




