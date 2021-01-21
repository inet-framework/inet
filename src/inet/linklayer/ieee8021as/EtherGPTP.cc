//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
// 

#include "EtherGPTP.h"
#include "tableGPTP.h"
#include "Clock.h"
#include "gPtp.h"
#include "gPtpPacket_m.h"
#include <omnetpp.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <list>
#include <utility>

#include "inet/linklayer/ethernet/EtherMACBase.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/base/MACBase.h"

Define_Module(EtherGPTP);

using namespace omnetpp;

EtherGPTP::EtherGPTP()
    : selfMsgSync(NULL), selfMsgDelayReq(NULL), selfMsgDelayResp(NULL), selfMsgFollowUp(NULL)
{
}

void EtherGPTP::initialize(int stage)
{
    cModule* gPtpNode=getParentModule()->getParentModule();
    if(gPtpNode->getSubmodule("tableGPTP")!=nullptr)
    {
        tableGptp = check_and_cast<TableGPTP *>(gPtpNode->getSubmodule("tableGPTP"));
    }
    if(gPtpNode->getSubmodule("clock")!=nullptr)
     {
        clockGptp = check_and_cast<Clock *>(gPtpNode->getSubmodule("clock"));
     }

    stepCounter = 0;
    peerDelay= tableGptp->getPeerDelay();
    portType = par("portType");
    nodeType = gPtpNode->par("gPtpNodeType");
    syncInterval = par("syncInterval");

    rateRatio = 1;
//    sentTimeFollowUp1 = 0;
//    sentTimeFollowUp2 = 0;
//    receivedTimeFollowUp1 = 0;
//    receivedTimeFollowUp2 = 0;

    /* following parameters are used to schedule follow_up and pdelay_resp messages.
     * These numbers must be enough large to prevent creating queue in MAC layer.
     * It means it should be large than transmission time of message sent before */
    PDelayRespInterval = 0.000008;
    FollowUpInterval = 0.000007;

    /* Only grandmaster in the domain can initialize the synchronization message periodically
     * so below condition checks whether it is grandmaster and then schedule first sync message */
    if(portType == MASTER_PORT && nodeType == MASTER_NODE)
    {
        // Schedule Sync message to be sent
        if (NULL == selfMsgSync)
            selfMsgSync = new cMessage("selfMsgSync");

        scheduleSync = syncInterval + 0.01;
        tableGptp->setOriginTimestamp(scheduleSync);
        scheduleAt(scheduleSync, selfMsgSync);
    }
    else if(portType == SLAVE_PORT)
    {
        vLocalTime.setName("Clock local");
        vMasterTime.setName("Clock master");
        vTimeDifference.setName("Clock difference to neighbor");
        vRateRatio.setName("Rate ratio");
        vPeerDelay.setName("Peer delay");
        vTimeDifferenceGMafterSync.setName("Clock difference to GM after Sync");
        vTimeDifferenceGMbeforeSync.setName("Clock difference to GM before Sync");

        requestMsg = new cMessage("requestToSendSync");

        // Schedule Pdelay_Req message is sent by slave port
        // without depending on node type which is grandmaster or bridge
        if (NULL == selfMsgDelayReq)
            selfMsgDelayReq = new cMessage("selfMsgPdelay");
        pdelayInterval = par("pdelayInterval");

        schedulePdelay = pdelayInterval;
        scheduleAt(schedulePdelay, selfMsgDelayReq);
    }
}

void EtherGPTP::handleMessage(cMessage *msg)
{
    tableGptp->setReceivedTimeAtHandleMessage(simTime());

    if(portType == MASTER_PORT)
    {
        masterPort(msg);
    }
    else if(portType == SLAVE_PORT)
    {
        slavePort(msg);
    }
    else
    {
        // Forward message to upperLayerOut gate since packet is not gPtp
        if(msg->arrivedOn("lowerLayerIn"))
        {
            EV_INFO << "EtherGPTP: Received " << msg << " from LOWER LAYER." << endl;
            send(msg, "upperLayerOut");
        }
        else
        {
            EV_INFO << "EtherGPTP: Received " << msg << " from UPPER LAYER." << endl;
            send(msg, "lowerLayerOut");
        }
    }
}

/*********************************/
/* Master port related functions */
/*********************************/

void EtherGPTP::masterPort(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if(selfMsgSync == msg)
        {
            sendSync(scheduleSync);

            /* Schedule next Sync message at next sync interval
             * Grand master always works at simulation time */
            scheduleSync = simTime() + syncInterval;
            scheduleAt(scheduleSync, msg);
        }
        else if(selfMsgFollowUp == msg)
        {
            sendFollowUp();
        }
        else if(selfMsgDelayResp == msg)
        {
            sendPdelayResp();
        }
    }
    else if (msg->arrivedOn("gptpLayerIn"))
    {
        // Only sync message is sent if its node is bridge
        if(nodeType == BRIDGE_NODE)
        {
            sendSync(clockGptp->getCurrentTime());
        }
        delete msg;
    }
    else if(msg->arrivedOn("lowerLayerIn"))
    {
        if (inet::EthernetIIFrame* etherFrame = dynamic_cast<inet::EthernetIIFrame*>(msg))
        {
            if(msg->getKind() == PDELAY_REQ)
            {
                gPtp_PdelayReq* gptp = dynamic_cast<gPtp_PdelayReq*>(etherFrame->decapsulate());
                processPdelayReq(gptp);
            }
            else
            {
                // Forward message to upperLayerOut gate since packet is not gPtp
                send(msg, "upperLayerOut");
            }
        }
        else
        {
            // Forward message to upperLayerOut gate since packet is not gPtp
            send(msg, "upperLayerOut");
        }
    }
    else if (msg->arrivedOn("upperLayerIn"))
    {
        // Forward message to lowerLayerOut gate because the message from upper layer doesn't need to be touched
        // and we are interested in only message from lowerLayerIn
        send(msg, "lowerLayerOut");
    }
}

void EtherGPTP::sendSync(SimTime value)
{
    // Create EtherFrame to be used to encapsulate gPtpPacket because lower layer MAC only supports EtherFrame
    inet::EthernetIIFrame* frame=new inet::EthernetIIFrame("gPtpPacket");
    frame->setDest(inet::MACAddress::BROADCAST_ADDRESS);
    frame->setEtherType(inet::ETHERTYPE_IPv4);  // So far INET doesn't support gPTP (etherType = 0x88f7)
    frame->setKind(SYNC);
    gPtp_Sync* gptp = gPtp::newSyncPacket();

    /* OriginTimestamp always get Sync departure time from grand master */
    if (nodeType == MASTER_NODE)
    {
        gptp->setOriginTimestamp(value);
        tableGptp->setOriginTimestamp(value);
    }
    else if(nodeType == BRIDGE_NODE)
    {
        gptp->setOriginTimestamp(tableGptp->getOriginTimestamp());
    }

    gptp->setLocalDrift(clockGptp->getCalculatedDrift(syncInterval));
    sentTimeSyncSync = clockGptp->getCurrentTime();
    gptp->setSentTime(sentTimeSyncSync);
    frame->encapsulate(gptp);

    send(frame, "lowerLayerOut");

    if (NULL == selfMsgFollowUp)
        selfMsgFollowUp = new cMessage("selfMsgFollowUp");
    scheduleAt(simTime() + FollowUpInterval, selfMsgFollowUp);
}

void EtherGPTP::sendFollowUp()
{
    // Create EtherFrame to be used to encapsulate gPtpPacket because lower layer MAC only supports EtherFrame
    inet::EthernetIIFrame* frame=new inet::EthernetIIFrame("gPtpPacket");
    frame->setDest(inet::MACAddress::BROADCAST_ADDRESS);
    frame->setEtherType(inet::ETHERTYPE_IPv4);  // So far INET doesn't support gPTP (etherType = 0x88f7)
    frame->setKind(FOLLOW_UP);

    gPtp_FollowUp* gptp = gPtp::newFollowUpPacket();
    gptp->setSentTime(clockGptp->getCurrentTime());        // simTime()
    gptp->setPreciseOriginTimestamp(tableGptp->getOriginTimestamp());

    if (nodeType == MASTER_NODE)
        gptp->setCorrectionField(0);
    else if (nodeType == BRIDGE_NODE)
    {
        /**************** Correction field calculation *********************************************
         * It is calculated by adding peer delay, residence time and packet transmission time      *
         * correctionField(i)=correctionField(i-1)+peerDelay+(timeReceivedSync-timeSentSync)*(1-f) *
         *******************************************************************************************/
        double bits = (MAC_HEADER + SYNC_PACKET_SIZE + CRC_CHECKSUM)*8;
        SimTime packetTransmissionTime = (SimTime)bits/100000000;
        gptp->setCorrectionField(tableGptp->getCorrectionField() + tableGptp->getPeerDelay() + packetTransmissionTime + sentTimeSyncSync - tableGptp->getReceivedTimeSync());
//        gptp->setCorrectionField(tableGptp->getCorrectionField() + tableGptp->getPeerDelay() + packetTransmissionTime + clockGptp->getCurrentTime() - tableGptp->getReceivedTimeSync());
    }
    gptp->setRateRatio(tableGptp->getRateRatio());
    frame->encapsulate(gptp);

    send(frame, "lowerLayerOut");
}

void EtherGPTP::processPdelayReq(gPtp_PdelayReq* gptp)
{
    receivedTimeResponder = clockGptp->getCurrentTime(); // simTime();

    if (NULL == selfMsgDelayResp)
        selfMsgDelayResp = new cMessage("selfMsgPdelayResp");

    schedulePdelayResp = simTime() + (SimTime)PDelayRespInterval;
    scheduleAt(schedulePdelayResp, selfMsgDelayResp);
}

void EtherGPTP::sendPdelayResp()
{
    // Create EtherFrame to be used to encapsulate gPtpPacket because lower layer MAC only supports EtherFrame
    inet::EthernetIIFrame* frame=new inet::EthernetIIFrame("gPtpPacket");
    frame->setDest(inet::MACAddress::BROADCAST_ADDRESS);
    frame->setEtherType(inet::ETHERTYPE_IPv4);  // So far INET doesn't support gPTP (etherType = 0x88f7)
    frame->setKind(PDELAY_RESP);

    gPtp_PdelayResp* gptp = gPtp::newDelayRespPacket();
    gptp->setSentTime(clockGptp->getCurrentTime());     // simTime()
    gptp->setRequestReceiptTimestamp(receivedTimeResponder);
    frame->encapsulate(gptp);

    send(frame, "lowerLayerOut");
    sendPdelayRespFollowUp();
}

void EtherGPTP::sendPdelayRespFollowUp()
{
    inet::EthernetIIFrame* frame=new inet::EthernetIIFrame("gPtpPacket");
    frame->setDest(inet::MACAddress::BROADCAST_ADDRESS);
    frame->setEtherType(inet::ETHERTYPE_IPv4);  // So far INET doesn't support gPTP (etherType = 0x88f7)
    frame->setKind(PDELAY_RESP_FOLLOW_UP);

    gPtp_PdelayRespFollowUp* gptp = gPtp::newDelayRespFollowUpPacket();
    gptp->setSentTime(clockGptp->getCurrentTime());               //  simTime()
    gptp->setResponseOriginTimestamp(receivedTimeResponder + (SimTime)PDelayRespInterval + clockGptp->getCalculatedDrift((SimTime)PDelayRespInterval));
    frame->encapsulate(gptp);

    send(frame, "lowerLayerOut");
}

/********************************/
/* Slave port related functions */
/********************************/

void EtherGPTP::slavePort(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if(selfMsgDelayReq == msg)
        {
            sendPdelayReq();

            schedulePdelay = simTime() + pdelayInterval;
            scheduleAt(schedulePdelay, msg);
        }
    }
    else if(msg->arrivedOn("lowerLayerIn"))
    {
        if (inet::EthernetIIFrame* etherFrame = dynamic_cast<inet::EthernetIIFrame*>(msg))
        {
            cPacket* ppp;
            switch(msg->getKind())
            {
            case SYNC:
                ppp=etherFrame->decapsulate();
                if(gPtp_Sync* gptp = dynamic_cast<gPtp_Sync*>(ppp))
                {
                    processSync(gptp);
                }
                break;
            case FOLLOW_UP:
                ppp=etherFrame->decapsulate();
                if(gPtp_FollowUp* gptp = dynamic_cast<gPtp_FollowUp*>(ppp))
                {
                    processFollowUp(gptp);

                    // Send a request to send Sync message
                    // through other gPtp Ethernet interfaces
                    send(requestMsg->dup(), "gptpLayerOut");
                }
                break;
            case PDELAY_RESP:
                ppp=etherFrame->decapsulate();
                if(gPtp_PdelayResp* gptp = dynamic_cast<gPtp_PdelayResp*>(ppp))
                {
                    processPdelayResp(gptp);
                }
                break;
            case PDELAY_RESP_FOLLOW_UP:
                ppp=etherFrame->decapsulate();
                if(gPtp_PdelayRespFollowUp* gptp = dynamic_cast<gPtp_PdelayRespFollowUp*>(ppp))
                {
                    processPdelayRespFollowUp(gptp);
                }
                break;
            default:
                send(msg, "upperLayerOut");
                break;
            }
        }
        else
        {
            // Forward message to upperLayerOut gate since packet is not gPtp
            send(msg, "upperLayerOut");
        }
    }
    else if (msg->arrivedOn("upperLayerIn"))
    {
        // Forward message to lowerLayerOut gate because the message from upper layer doesn't need to be touched
        // and we are interested in only message from lowerLayerIn
        send(msg, "lowerLayerOut");
    }
}

void EtherGPTP::sendPdelayReq()
{
    // Create EtherFrame to be used to encapsulate gPtpPacket because lower layer MAC only supports EtherFrame
    inet::EthernetIIFrame* frame=new inet::EthernetIIFrame("gPtpPacket");
    frame->setDest(inet::MACAddress::BROADCAST_ADDRESS);
    frame->setEtherType(inet::ETHERTYPE_IPv4);  // So far INET doesn't support gPTP (etherType = 0x88f7)
    frame->setKind(PDELAY_REQ);

    gPtp_PdelayReq* gptp = gPtp::newDelayReqPacket();
    gptp->setSentTime(clockGptp->getCurrentTime());
    gptp->setOriginTimestamp(schedulePdelay);
    frame->encapsulate(gptp);

    send(frame, "lowerLayerOut");
    transmittedTimeRequester = clockGptp->getCurrentTime();
}

void EtherGPTP::processSync(gPtp_Sync* gptp)
{
    sentTimeSync = gptp->getOriginTimestamp();
    residenceTime = simTime() - tableGptp->getReceivedTimeAtHandleMessage();
    receivedTimeSyncBeforeSync = clockGptp->getCurrentTime();

    /************** Time synchronization *****************************************
     * Local time is adjusted using peer delay, correction field, residence time *
     * and packet transmission time based departure time of Sync message from GM *
     *****************************************************************************/
    double bits = (MAC_HEADER + SYNC_PACKET_SIZE + CRC_CHECKSUM + 2)*8;
    SimTime packetTransmissionTime = (SimTime)bits/100000000;
    clockGptp->adjustTime(sentTimeSync + tableGptp->getPeerDelay() + tableGptp->getCorrectionField() + residenceTime + packetTransmissionTime);

    receivedTimeSyncAfterSync = clockGptp->getCurrentTime();
    tableGptp->setReceivedTimeSync(receivedTimeSyncAfterSync);

    /************** Rate ratio calculation *************************************
     * It is calculated based on interval between two successive Sync messages *
     ***************************************************************************/
    neighborDrift = gptp->getLocalDrift();
    rateRatio = (neighborDrift + syncInterval)/(clockGptp->getCalculatedDrift(syncInterval) + syncInterval);

    EV_INFO << "############## SYNC #####################################"<< endl;
    EV_INFO << "RECEIVED TIME AFTER SYNC - " << receivedTimeSyncAfterSync << endl;
    EV_INFO << "RECEIVED SIM TIME        - " << simTime() << endl;
    EV_INFO << "ORIGIN TIME SYNC         - " << sentTimeSync << endl;
    EV_INFO << "RESIDENCE TIME           - " << residenceTime << endl;
    EV_INFO << "CORRECTION FIELD         - " << tableGptp->getCorrectionField() << endl;
    EV_INFO << "PROPAGATION DELAY        - " << tableGptp->getPeerDelay() << endl;
    EV_INFO << "TRANSMISSION TIME        - " << packetTransmissionTime << endl;

    // Transmission time of 2 more bytes is going here
    // in mac layer? or in our implementation?
    EV_INFO << "TIME DIFFERENCE TO STIME - " << receivedTimeSyncAfterSync - simTime() << endl;

    tableGptp->setRateRatio(rateRatio);
    vRateRatio.record(rateRatio);
    vLocalTime.record(receivedTimeSyncAfterSync);
    vMasterTime.record(sentTimeSync);
    vTimeDifference.record(receivedTimeSyncBeforeSync - sentTimeSync - tableGptp->getPeerDelay());
}

void EtherGPTP::processFollowUp(gPtp_FollowUp* gptp)
{
    tableGptp->setReceivedTimeFollowUp(simTime());
    tableGptp->setOriginTimestamp(gptp->getPreciseOriginTimestamp());
    tableGptp->setCorrectionField(gptp->getCorrectionField());

    /************* Time difference to Grand master *******************************************
     * Time difference before synchronize local time and after synchronization of local time *
     *****************************************************************************************/
    double bits = (MAC_HEADER + SYNC_PACKET_SIZE + CRC_CHECKSUM + 2)*8;
    SimTime packetTransmissionTime = (SimTime)bits/100000000;
    SimTime timeDifferenceAfter  = receivedTimeSyncAfterSync - tableGptp->getOriginTimestamp() - tableGptp->getPeerDelay() - tableGptp->getCorrectionField() - packetTransmissionTime;
    SimTime timeDifferenceBefore = receivedTimeSyncBeforeSync - tableGptp->getOriginTimestamp() - tableGptp->getPeerDelay() - tableGptp->getCorrectionField() - packetTransmissionTime;
    vTimeDifferenceGMafterSync.record(timeDifferenceAfter);
    vTimeDifferenceGMbeforeSync.record(timeDifferenceBefore);

    EV_INFO << "############## FOLLOW_UP ################################"<< endl;
    EV_INFO << "RECEIVED TIME AFTER SYNC - " << receivedTimeSyncAfterSync << endl;
    EV_INFO << "ORIGIN TIME SYNC         - " << tableGptp->getOriginTimestamp() << endl;
    EV_INFO << "CORRECTION FIELD         - " << tableGptp->getCorrectionField() << endl;
    EV_INFO << "PROPAGATION DELAY        - " << tableGptp->getPeerDelay() << endl;
    EV_INFO << "TRANSMISSION TIME        - " << packetTransmissionTime << endl;
    EV_INFO << "TIME DIFFERENCE TO GM    - " << timeDifferenceAfter << endl;
    EV_INFO << "TIME DIFFERENCE TO GM BEF- " << timeDifferenceBefore << endl;

//    double bits = (MAC_HEADER + FOLLOW_UP_PACKET_SIZE + CRC_CHECKSUM)*8;
//    SimTime packetTransmissionTime = (SimTime)bits/100000000;
//    vTimeDifferenceGMafterSync.record(receivedTimeSyncAfterSync - simTime() + FollowUpInterval + packetTransmissionTime + tableGptp->getPeerDelay());
//    vTimeDifferenceGMbeforeSync.record(receivedTimeSyncBeforeSync - simTime() + FollowUpInterval + packetTransmissionTime + tableGptp->getPeerDelay());
}

void EtherGPTP::processPdelayResp(gPtp_PdelayResp* gptp)
{
    receivedTimeRequester = clockGptp->getCurrentTime();        // simTime();
    receivedTimeResponder = gptp->getRequestReceiptTimestamp();
    transmittedTimeResponder = gptp->getSentTime();
}

void EtherGPTP::processPdelayRespFollowUp(gPtp_PdelayRespFollowUp* gptp)
{
    /************* Peer delay measurement ********************************************
     * It doesn't contain packet transmission time which is equal to (byte/datarate) *
     * on responder side, pdelay_resp is scheduled using PDelayRespInterval time.    *
     * PDelayRespInterval needs to be deducted as well as packet transmission time   *
     *********************************************************************************/
    double bits = (MAC_HEADER + PDELAY_RESP_PACKET_SIZE + CRC_CHECKSUM)*8;
    SimTime packetTransmissionTime = (SimTime)bits/100000000;
    peerDelay= (tableGptp->getRateRatio().dbl()*(receivedTimeRequester.dbl() - transmittedTimeRequester.dbl()) + transmittedTimeResponder.dbl() - receivedTimeResponder.dbl())/2
            - PDelayRespInterval - packetTransmissionTime;

    EV_INFO << "transmittedTimeRequester - " << transmittedTimeRequester << endl;
    EV_INFO << "transmittedTimeResponder - " << transmittedTimeResponder << endl;
    EV_INFO << "receivedTimeRequester    - " << receivedTimeRequester << endl;
    EV_INFO << "receivedTimeResponder    - " << receivedTimeResponder << endl;
    EV_INFO << "packetTransmissionTime   - " << packetTransmissionTime << endl;
    EV_INFO << "PEER DELAY               - " << peerDelay << endl;

    tableGptp->setPeerDelay(peerDelay);
    vPeerDelay.record(peerDelay);
}

