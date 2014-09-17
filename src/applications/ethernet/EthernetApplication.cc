/**
 ******************************************************
 * @file EthernetApplication.cc
 * @brief Simple traffic generator.
 * It generates Etherapp requests and responses. Based in EtherAppCli and EtherAppSrv.
 *
 * @author Juan Luis Garrote Molinero
 * @version 1.0
 * @date Feb 2011
 *
 *
 ******************************************************/

#include "inet/applications/ethernet/EthernetApplication.h"
#include "inet/applications/ethernet/EtherApp_m.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"

namespace inet {

Define_Module(EthernetApplication);
simsignal_t EthernetApplication::sentPkSignal = SIMSIGNAL_NULL;
simsignal_t EthernetApplication::rcvdPkSignal = SIMSIGNAL_NULL;
void EthernetApplication::initialize(int stage)
{
    // we can only initialize in the 2nd stage (stage==1), because
    // assignment of "auto" MAC addresses takes place in stage 0
    if (stage == INITSTAGE_APPLICATION_LAYER) {
        reqLength = &par("reqLength");
        respLength = &par("respLength");
        waitTime = &par("waitTime");

        seqNum = 0;
        WATCH(seqNum);

        // statistics
        packetsSent = packetsReceived = 0;
        sentPkSignal = registerSignal("sentPk");
        rcvdPkSignal = registerSignal("rcvdPk");
        WATCH(packetsSent);
        WATCH(packetsReceived);

        destMACAddress = resolveDestMACAddress();

        // if no dest address is given, nothing to do
        if (destMACAddress.isUnspecified())
            return;

        cMessage *timermsg = new cMessage("generateNextPacket");
        simtime_t d = par("startTime").doubleValue();
        scheduleAt(simTime() + d, timermsg);
    }
}

MACAddress EthernetApplication::resolveDestMACAddress()
{
    MACAddress destMACAddress;
    const char *destAddress = par("destAddress");
    if (destAddress[0]) {
        // try as mac address first, then as a module
        if (!destMACAddress.tryParse(destAddress)) {
            cModule *destStation = simulation.getModuleByPath(destAddress);
            if (!destStation)
                error("cannot resolve MAC address '%s': not a 12-hex-digit MAC address or a valid module path name", destAddress);
            cModule *destMAC = destStation->getSubmodule("mac");
            if (!destMAC)
                error("module '%s' has no 'mac' submodule", destAddress);
            destMACAddress.setAddress(destMAC->par("address"));
        }
    }
    return destMACAddress;
}

void EthernetApplication::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        sendPacket();
        simtime_t d = waitTime->doubleValue();
        scheduleAt(simTime() + d, msg);
    }
    else {
        receivePacket(msg);
    }
}

void EthernetApplication::sendPacket()
{
    seqNum++;

    char msgname[30];
    sprintf(msgname, "req-%d-%ld", getId(), seqNum);
    EV << "Generating packet `" << msgname << "'\n";

    EtherAppReq *datapacket = new EtherAppReq(msgname, IEEE802CTRL_DATA);

    datapacket->setRequestId(seqNum);

    long len = reqLength->longValue();
    datapacket->setByteLength(len);

    long respLen = respLength->longValue();
    datapacket->setResponseBytes(respLen);

    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDest(destMACAddress);
    datapacket->setControlInfo(etherctrl);

    send(datapacket, "out");
    packetsSent++;
}

void EthernetApplication::receivePacket(cMessage *msg)
{
    EV << "Received packet `" << msg->getName() << "'\n";

    packetsReceived++;
    // simtime_t lastEED = simTime() - msg->getCreationTime();

    if (dynamic_cast<EtherAppReq *>(msg)) {
        EtherAppReq *req = check_and_cast<EtherAppReq *>(msg);
        emit(rcvdPkSignal, req);
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(req->removeControlInfo());
        MACAddress srcAddr = ctrl->getSrc();
        long requestId = req->getRequestId();
        long replyBytes = req->getResponseBytes();
        char msgname[30];
        strcpy(msgname, msg->getName());
        delete ctrl;

        // send back packets asked by EthernetApplication Client side
        int k = 0;
        strcat(msgname, "-resp-");
        char *s = msgname + strlen(msgname);
        while (replyBytes > 0) {
            int l = replyBytes > MAX_REPLY_CHUNK_SIZE ? MAX_REPLY_CHUNK_SIZE : replyBytes;
            replyBytes -= l;

            sprintf(s, "%d", k);

            EV << "Generating packet `" << msgname << "'\n";

            EtherAppResp *datapacket = new EtherAppResp(msgname, IEEE802CTRL_DATA);
            datapacket->setRequestId(requestId);
            datapacket->setByteLength(l);
            sendPacket(datapacket, srcAddr);
            packetsSent++;

            k++;
        }
    }

    delete msg;
}

void EthernetApplication::sendPacket(cMessage *datapacket, const MACAddress& destAddr)
{
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setDest(destAddr);
    datapacket->setControlInfo(etherctrl);
    emit(sentPkSignal, datapacket);
    send(datapacket, "out");
}

void EthernetApplication::finish()
{
}

} // namespace inet

