/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>

#include "EtherAppSrv.h"

#include "EtherApp_m.h"
#include "Ieee802Ctrl_m.h"

Define_Module(EtherAppSrv);

simsignal_t EtherAppSrv::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t EtherAppSrv::rcvdPkSignal = SIMSIGNAL_NULL;

void EtherAppSrv::initialize()
{
    localSAP = ETHERAPP_SRV_SAP;
    remoteSAP = ETHERAPP_CLI_SAP;

    // statistics
    packetsSent = packetsReceived = 0;
    sentPkSignal = registerSignal("sentPk");
    rcvdPkSignal = registerSignal("rcvdPk");

    WATCH(packetsSent);
    WATCH(packetsReceived);

    bool registerSAP = par("registerSAP");
    if (registerSAP)
        registerDSAP(localSAP);
}

void EtherAppSrv::handleMessage(cMessage *msg)
{
    EV << "Received packet `" << msg->getName() << "'\n";
    EtherAppReq *req = check_and_cast<EtherAppReq *>(msg);
    packetsReceived++;
    emit(rcvdPkSignal, req);

    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(req->removeControlInfo());
    MACAddress srcAddr = ctrl->getSrc();
    long requestId = req->getRequestId();
    long replyBytes = req->getResponseBytes();
    char msgname[30];
    strcpy(msgname, msg->getName());

    delete msg;
    delete ctrl;

    // send back packets asked by EtherAppCli side
    int k = 0;
    strcat(msgname, "-resp-");
    char *s = msgname + strlen(msgname);

    while (replyBytes > 0)
    {
        int l = replyBytes > MAX_REPLY_CHUNK_SIZE ? MAX_REPLY_CHUNK_SIZE : replyBytes;
        replyBytes -= l;

        sprintf(s, "%d", k);

        EV << "Generating packet `" << msgname << "'\n";

        EtherAppResp *datapacket = new EtherAppResp(msgname, IEEE802CTRL_DATA);
        datapacket->setRequestId(requestId);
        datapacket->setByteLength(l);
        sendPacket(datapacket, srcAddr);

        k++;
    }
}

void EtherAppSrv::sendPacket(cPacket *datapacket, const MACAddress& destAddr)
{
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSsap(localSAP);
    etherctrl->setDsap(remoteSAP);
    etherctrl->setDest(destAddr);
    datapacket->setControlInfo(etherctrl);
    emit(sentPkSignal, datapacket);
    send(datapacket, "out");
    packetsSent++;
}

void EtherAppSrv::registerDSAP(int dsap)
{
    EV << getFullPath() << " registering DSAP " << dsap << "\n";

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDsap(dsap);
    cMessage *msg = new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
    msg->setControlInfo(etherctrl);

    send(msg, "out");
}

void EtherAppSrv::finish()
{
}

