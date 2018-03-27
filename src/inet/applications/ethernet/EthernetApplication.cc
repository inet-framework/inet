/**
 ******************************************************
 * @file EthernetApplication.cc
 * @brief Simple traffic generator.
 * It generates Etherapp requests and responses. Based in EtherAppClient and EtherAppServer.
 *
 * @author Juan Luis Garrote Molinero
 * @version 1.0
 * @date Feb 2011
 *
 *
 ******************************************************/

#include "inet/applications/ethernet/EtherApp_m.h"
#include "inet/applications/ethernet/EthernetApplication.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

namespace inet {

Define_Module(EthernetApplication);

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
        WATCH(packetsSent);
        WATCH(packetsReceived);

        destMACAddress = resolveDestMACAddress();

        // if no dest address is given, nothing to do
        if (destMACAddress.isUnspecified())
            return;

        cMessage *timermsg = new cMessage("generateNextPacket");
        simtime_t d(par("startTime"));
        scheduleAt(simTime() + d, timermsg);
    }
}

MacAddress EthernetApplication::resolveDestMACAddress()
{
    MacAddress destMACAddress;
    const char *destAddress = par("destAddress");
    if (destAddress[0]) {
        // try as mac address first, then as a module
        if (!destMACAddress.tryParse(destAddress)) {
            cModule *destStation = getModuleByPath(destAddress);
            if (!destStation)
                throw cRuntimeError("cannot resolve MAC address '%s': not a 12-hex-digit MAC address or a valid module path name", destAddress);
            cModule *destMAC = destStation->getSubmodule("mac");
            if (!destMAC)
                throw cRuntimeError("module '%s' has no 'mac' submodule", destAddress);
            destMACAddress.setAddress(destMAC->par("address"));
        }
    }
    return destMACAddress;
}

void EthernetApplication::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        sendPacket();
        simtime_t d(*waitTime);
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

    Packet *datapacket = new Packet(msgname, IEEE802CTRL_DATA);
    const auto& data = makeShared<EtherAppReq>();
    data->setRequestId(seqNum);
    long len = *reqLength;
    data->setChunkLength(B(len));
    long respLen = *respLength;
    data->setResponseBytes(respLen);
    datapacket->insertAtBack(data);
    datapacket->addTagIfAbsent<MacAddressReq>()->setDestAddress(destMACAddress);
    send(datapacket, "out");
    packetsSent++;
}

void EthernetApplication::receivePacket(cMessage *msg)
{
    EV << "Received packet `" << msg->getName() << "'\n";

    packetsReceived++;
    // simtime_t lastEED = simTime() - msg->getCreationTime();

    Packet *reqPk = check_and_cast<Packet *>(msg);
    emit(packetReceivedSignal, reqPk);
    const auto& req = reqPk->peekDataAt<EtherAppReq>(B(0));

    if (req != nullptr) {
        MacAddress srcAddr = reqPk->getTag<MacAddressInd>()->getSrcAddress();
        long requestId = req->getRequestId();
        long replyBytes = req->getResponseBytes();

        // send back packets asked by EthernetApplication Client side
        for (int k = 0; replyBytes > 0; k++) {
            int l = replyBytes > MAX_REPLY_CHUNK_SIZE ? MAX_REPLY_CHUNK_SIZE : replyBytes;
            replyBytes -= l;

            std::ostringstream s;
            s << msg->getName() << "-resp-" << k;

            EV << "Generating packet `" << s.str() << "'\n";
            Packet *outPacket = new Packet(s.str().c_str(), IEEE802CTRL_DATA);
            const auto& outPayload = makeShared<EtherAppResp>();
            outPayload->setRequestId(requestId);
            outPayload->setChunkLength(B(l));
            outPacket->insertAtBack(outPayload);

            sendPacket(outPacket, srcAddr);
            packetsSent++;
        }
    }

    delete msg;
}

void EthernetApplication::sendPacket(Packet *datapacket, const MacAddress& destAddr)
{
    datapacket->addTagIfAbsent<MacAddressReq>()->setDestAddress(destAddr);
    emit(packetSentSignal, datapacket);
    send(datapacket, "out");
}

void EthernetApplication::finish()
{
}

} // namespace inet

