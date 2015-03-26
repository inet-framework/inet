/***************************************************************************
                       RTCP.cc  -  description
                             -------------------
    (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
    (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "inet/transportlayer/rtp/RTCP.h"

#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/transportlayer/rtp/RTCPPacket.h"
#include "inet/transportlayer/rtp/RTPInnerPacket.h"
#include "inet/transportlayer/rtp/RTPParticipantInfo.h"
#include "inet/transportlayer/rtp/RTPReceiverInfo.h"
#include "inet/transportlayer/rtp/RTPSenderInfo.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/transportlayer/contract/udp/UDPSocket.h"

namespace inet {

namespace rtp {

Define_Module(RTCP);

simsignal_t RTCP::rcvdPkSignal = registerSignal("rcvdPk");

RTCP::RTCP()
{
}

void RTCP::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // initialize variables
        _ssrcChosen = false;
        _leaveSession = false;
        _udpSocket.setOutputGate(gate("udpOut"));

        _packetsCalculated = 0;
        _averagePacketSize = 0.0;

        _participantInfos.setName("ParticipantInfos");
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

RTCP::~RTCP()
{
    if (!_ssrcChosen)
        delete _senderInfo;
}

void RTCP::handleMessage(cMessage *msg)
{
    // first distinguish incoming messages by arrival gate
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    }
    else if (msg->getArrivalGateId() == findGate("rtpIn")) {
        handleMessageFromRTP(msg);
    }
    else if (msg->getArrivalGateId() == findGate("udpIn")) {
        handleMessageFromUDP(msg);
    }
    else {
        throw cRuntimeError("Message from unknown gate");
    }

    delete msg;
}

//
// handle messages from different gates
//

void RTCP::handleMessageFromRTP(cMessage *msg)
{
    // from the rtp module all messages are of type RTPInnerPacket
    RTPInnerPacket *rinp = check_and_cast<RTPInnerPacket *>(msg);

    // distinguish by type
    switch (rinp->getType()) {
        case RTP_INP_INITIALIZE_RTCP:
            handleInitializeRTCP(rinp);
            break;

        case RTP_INP_SENDER_MODULE_INITIALIZED:
            handleSenderModuleInitialized(rinp);
            break;

        case RTP_INP_DATA_OUT:
            handleDataOut(rinp);
            break;

        case RTP_INP_DATA_IN:
            handleDataIn(rinp);
            break;

        case RTP_INP_LEAVE_SESSION:
            handleLeaveSession(rinp);
            break;

        default:
            throw cRuntimeError("Unknown RTPInnerPacket type");
    }
}

void RTCP::handleMessageFromUDP(cMessage *msg)
{
    // from SocketLayer all message are of type cMessage
    readRet(PK(msg));
}

void RTCP::handleSelfMessage(cMessage *msg)
{
    // it's time to create an rtcp packet
    if (!_ssrcChosen) {
        chooseSSRC();
        RTPInnerPacket *rinp1 = new RTPInnerPacket("rtcpInitialized()");
        rinp1->setRtcpInitializedPkt(_senderInfo->getSsrc());
        send(rinp1, "rtpOut");
    }

    createPacket();

    if (!_leaveSession) {
        scheduleInterval();
    }
}

//
// methods for different messages
//

void RTCP::handleInitializeRTCP(RTPInnerPacket *rinp)
{
    _mtu = rinp->getMTU();
    _bandwidth = rinp->getBandwidth();
    _rtcpPercentage = rinp->getRtcpPercentage();
    _destinationAddress = rinp->getAddress();
    _port = rinp->getPort();

    _senderInfo = new RTPSenderInfo();

    SDESItem *sdesItem = new SDESItem(SDESItem::SDES_CNAME, rinp->getCommonName());
    _senderInfo->addSDESItem(sdesItem);

    // create server socket for receiving rtcp packets
    createSocket();
}

void RTCP::handleSenderModuleInitialized(RTPInnerPacket *rinp)
{
    _senderInfo->setStartTime(simTime());
    _senderInfo->setClockRate(rinp->getClockRate());
    _senderInfo->setTimeStampBase(rinp->getTimeStampBase());
    _senderInfo->setSequenceNumberBase(rinp->getSequenceNumberBase());
}

void RTCP::handleDataOut(RTPInnerPacket *packet)
{
    RTPPacket *rtpPacket = check_and_cast<RTPPacket *>(packet->decapsulate());
    processOutgoingRTPPacket(rtpPacket);
}

void RTCP::handleDataIn(RTPInnerPacket *rinp)
{
    RTPPacket *rtpPacket = check_and_cast<RTPPacket *>(rinp->decapsulate());
    //rtpPacket->dump();
    processIncomingRTPPacket(rtpPacket, rinp->getAddress(), rinp->getPort());
}

void RTCP::handleLeaveSession(RTPInnerPacket *rinp)
{
    _leaveSession = true;
}

void RTCP::connectRet()
{
    // schedule first rtcp packet
    double intervalLength = 2.5 * (dblrand() + 0.5);
    cMessage *reminderMessage = new cMessage("Interval");
    scheduleAt(simTime() + intervalLength, reminderMessage);
}

void RTCP::readRet(cPacket *sifpIn)
{
    emit(rcvdPkSignal, sifpIn);
    RTCPCompoundPacket *packet = check_and_cast<RTCPCompoundPacket *>(sifpIn->decapsulate());
    processIncomingRTCPPacket(packet, IPv4Address(_destinationAddress), _port);
}

void RTCP::createSocket()
{
    _udpSocket.bind(_port);    //XXX this will fail if this function is invoked multiple times; not sure that may (or is expected to) happen
    MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
    _udpSocket.joinLocalMulticastGroups(mgl);    //TODO make it parameter-dependent
    connectRet();
}

void RTCP::scheduleInterval()
{
    simtime_t intervalLength = _averagePacketSize * (simtime_t)(_participantInfos.size())
        / (simtime_t)(_bandwidth * _rtcpPercentage * (_senderInfo->isSender() ? 1.0 : 0.75) / 100.0);

    // interval length must be at least 5 seconds
    if (intervalLength < 5.0)
        intervalLength = 5.0;

    // to avoid rtcp packet bursts multiply calculated interval length
    // with a random number between 0.5 and 1.5
    intervalLength = intervalLength * (0.5 + dblrand());

    intervalLength /= (double)(2.71828 - 1.5);    // [RFC 3550] , by Ahmed ayadi

    cMessage *reminderMessage = new cMessage("Interval");
    scheduleAt(simTime() + intervalLength, reminderMessage);
}

void RTCP::chooseSSRC()
{
    uint32 ssrc = 0;
    bool ssrcConflict = false;
    do {
        ssrc = intrand(0x7fffffff);
        ssrcConflict = findParticipantInfo(ssrc) != nullptr;
    } while (ssrcConflict);

    EV_INFO << "chooseSSRC" << ssrc;
    _senderInfo->setSsrc(ssrc);
    _participantInfos.add(_senderInfo);
    _ssrcChosen = true;
}

void RTCP::createPacket()
{
    // first packet in an rtcp compound packet must
    // be a sender or receiver report
    RTCPReceiverReportPacket *reportPacket;

    // if this rtcp end system is a sender (see SenderInformation::isSender() for
    // details) insert a sender report
    if (_senderInfo->isSender()) {
        RTCPSenderReportPacket *senderReportPacket = new RTCPSenderReportPacket("SenderReportPacket");
        SenderReport *senderReport = _senderInfo->senderReport(simTime());
        senderReportPacket->setSenderReport(*senderReport);
        delete senderReport;
        reportPacket = senderReportPacket;
    }
    else
        reportPacket = new RTCPReceiverReportPacket("ReceiverReportPacket");
    reportPacket->setSsrc(_senderInfo->getSsrc());

    // insert receiver reports for packets from other sources
    for (int i = 0; i < _participantInfos.size(); i++) {
        if (_participantInfos.exist(i)) {
            RTPParticipantInfo *participantInfo = (RTPParticipantInfo *)(_participantInfos.get(i));
            if (participantInfo->getSsrc() != _senderInfo->getSsrc()) {
                ReceptionReport *report = ((RTPReceiverInfo *)participantInfo)->receptionReport(simTime());
                if (report != nullptr) {
                    reportPacket->addReceptionReport(report);
                }
            }
            participantInfo->nextInterval(simTime());

            if (participantInfo->toBeDeleted(simTime())) {
                _participantInfos.remove(participantInfo);
                delete participantInfo;
                // perhaps inform the profile
            }
        }
    }

    // insert source description items (at least common name)
    RTCPSDESPacket *sdesPacket = new RTCPSDESPacket("SDESPacket");

    SDESChunk *chunk = _senderInfo->getSDESChunk();
    sdesPacket->addSDESChunk(chunk);

    RTCPCompoundPacket *compoundPacket = new RTCPCompoundPacket("RTCPCompoundPacket");

    compoundPacket->addRTCPPacket(reportPacket);
    compoundPacket->addRTCPPacket(sdesPacket);

    // create rtcp app/bye packets if needed
    if (_leaveSession) {
        RTCPByePacket *byePacket = new RTCPByePacket("ByePacket");
        byePacket->setSsrc(_senderInfo->getSsrc());
        compoundPacket->addRTCPPacket(byePacket);
    }

    calculateAveragePacketSize(compoundPacket->getByteLength());

    cPacket *msg = new cPacket("RTCPCompoundPacket");
    msg->encapsulate(compoundPacket);
    _udpSocket.sendTo(msg, _destinationAddress, _port);

    if (_leaveSession) {
        RTPInnerPacket *rinp = new RTPInnerPacket("sessionLeft()");
        rinp->setSessionLeftPkt();
        send(rinp, "rtpOut");
    }
}

void RTCP::processOutgoingRTPPacket(RTPPacket *packet)
{
    _senderInfo->processRTPPacket(packet, getId(), simTime());
}

void RTCP::processIncomingRTPPacket(RTPPacket *packet, IPv4Address address, int port)
{
    bool good = false;
    uint32 ssrc = packet->getSsrc();
    RTPParticipantInfo *participantInfo = findParticipantInfo(ssrc);
    if (participantInfo == nullptr) {
        participantInfo = new RTPParticipantInfo(ssrc);
        participantInfo->setAddress(address);
        participantInfo->setRTPPort(port);
        _participantInfos.add(participantInfo);
        good = true;
    }
    else {
        // check for ssrc conflict
        if (participantInfo->getAddress() == address) {
            if (participantInfo->getRTPPort() == port) {
                good = true;
            }
            else if (participantInfo->getRTPPort() == PORT_UNDEF) {
                participantInfo->setRTPPort(port);
                good = true;
            }
        }
    }
    if (good) {
        participantInfo->processRTPPacket(packet, getId(), packet->getArrivalTime());
    }
    else {
        EV_INFO << "Incoming packet address/port conflict, packet dropped.\n";
        delete packet;
    }
}

void RTCP::processIncomingRTCPPacket(RTCPCompoundPacket *packet, IPv4Address address, int port)
{
    calculateAveragePacketSize(packet->getByteLength());
    cArray& rtcpPackets = packet->getRtcpPackets();
    simtime_t arrivalTime = packet->getArrivalTime();

    for (int i = 0; i < rtcpPackets.size(); i++) {
        RTCPPacket *rtcpPacket = (RTCPPacket *)(rtcpPackets.remove(i));
        if (rtcpPacket) {
            // remove the rtcp packet from the rtcp compound packet
            switch (rtcpPacket->getPacketType()) {
                case RTCP_PT_SR:
                    processIncomingRTCPSenderReportPacket(
                        (RTCPSenderReportPacket *)rtcpPacket, address, port);
                    break;

                case RTCP_PT_RR:
                    processIncomingRTCPReceiverReportPacket(
                        (RTCPReceiverReportPacket *)rtcpPacket, address, port);
                    break;

                case RTCP_PT_SDES:
                    processIncomingRTCPSDESPacket(
                        (RTCPSDESPacket *)rtcpPacket, address, port, arrivalTime);
                    break;

                case RTCP_PT_BYE:
                    processIncomingRTCPByePacket(
                        (RTCPByePacket *)rtcpPacket, address, port);
                    break;

                default:
                    // app rtcp packets
                    break;
            }
            delete rtcpPacket;
        }
    }
    delete packet;
}

void RTCP::processIncomingRTCPSenderReportPacket(RTCPSenderReportPacket *rtcpSenderReportPacket, IPv4Address address, int port)
{
    uint32 ssrc = rtcpSenderReportPacket->getSsrc();
    RTPParticipantInfo *participantInfo = findParticipantInfo(ssrc);

    if (participantInfo == nullptr) {
        participantInfo = new RTPReceiverInfo(ssrc);
        participantInfo->setAddress(address);
        participantInfo->setRTCPPort(port);
        _participantInfos.add(participantInfo);
    }
    else {
        if ((participantInfo->getAddress() == address) &&
            (participantInfo->getRTCPPort() == PORT_UNDEF))
        {
            participantInfo->setRTCPPort(port);
        }
        else {
            // check for ssrc conflict
        }
    }
    participantInfo->processSenderReport(rtcpSenderReportPacket->getSenderReport(), simTime());

    cArray& receptionReports = rtcpSenderReportPacket->getReceptionReports();
    for (int j = 0; j < receptionReports.size(); j++) {
        if (receptionReports.exist(j)) {
            ReceptionReport *receptionReport = (ReceptionReport *)(receptionReports.remove(j));
            if (_senderInfo && (receptionReport->getSsrc() == _senderInfo->getSsrc())) {
                _senderInfo->processReceptionReport(receptionReport, simTime());
            }
            else
                //cancelAndDelete(receptionReport);
                delete receptionReport;
        }
    }
}

void RTCP::processIncomingRTCPReceiverReportPacket(RTCPReceiverReportPacket *rtcpReceiverReportPacket, IPv4Address address, int port)
{
    uint32 ssrc = rtcpReceiverReportPacket->getSsrc();
    RTPParticipantInfo *participantInfo = findParticipantInfo(ssrc);
    if (participantInfo == nullptr) {
        participantInfo = new RTPReceiverInfo(ssrc);
        participantInfo->setAddress(address);
        participantInfo->setRTCPPort(port);
        _participantInfos.add(participantInfo);
    }
    else {
        if ((participantInfo->getAddress() == address) &&
            (participantInfo->getRTCPPort() == PORT_UNDEF))
        {
            participantInfo->setRTCPPort(port);
        }
        else {
            // check for ssrc conflict
        }
    }

    cArray& receptionReports = rtcpReceiverReportPacket->getReceptionReports();
    for (int j = 0; j < receptionReports.size(); j++) {
        if (receptionReports.exist(j)) {
            ReceptionReport *receptionReport = (ReceptionReport *)(receptionReports.remove(j));
            if (_senderInfo && (receptionReport->getSsrc() == _senderInfo->getSsrc())) {
                _senderInfo->processReceptionReport(receptionReport, simTime());
            }
            else
                //cancelAndDelete(receptionReport);
                delete receptionReport;
        }
    }
}

void RTCP::processIncomingRTCPSDESPacket(RTCPSDESPacket *rtcpSDESPacket, IPv4Address address, int port, simtime_t arrivalTime)
{
    cArray& sdesChunks = rtcpSDESPacket->getSdesChunks();

    for (int j = 0; j < sdesChunks.size(); j++) {
        if (sdesChunks.exist(j)) {
            // remove the sdes chunk from the cArray of sdes chunks
            SDESChunk *sdesChunk = (SDESChunk *)(sdesChunks.remove(j));
            // this is needed to avoid seg faults
            //sdesChunk->setOwner(this);
            uint32 ssrc = sdesChunk->getSsrc();
            RTPParticipantInfo *participantInfo = findParticipantInfo(ssrc);
            if (participantInfo == nullptr) {
                participantInfo = new RTPReceiverInfo(ssrc);
                participantInfo->setAddress(address);
                participantInfo->setRTCPPort(port);
                _participantInfos.add(participantInfo);
            }
            else {
                // check for ssrc conflict
            }
            participantInfo->processSDESChunk(sdesChunk, arrivalTime);
        }
    }
}

void RTCP::processIncomingRTCPByePacket(RTCPByePacket *rtcpByePacket, IPv4Address address, int port)
{
    uint32 ssrc = rtcpByePacket->getSsrc();
    RTPParticipantInfo *participantInfo = findParticipantInfo(ssrc);

    if (participantInfo != nullptr && participantInfo != _senderInfo) {
        _participantInfos.remove(participantInfo);

        delete participantInfo;
        // perhaps it would be useful to inform
        // the profile to remove the corresponding
        // receiver module
    }
}

RTPParticipantInfo *RTCP::findParticipantInfo(uint32 ssrc)
{
    std::string ssrcString = RTPParticipantInfo::ssrcToName(ssrc);
    return (RTPParticipantInfo *)(_participantInfos.get(ssrcString.c_str()));
}

void RTCP::calculateAveragePacketSize(int size)
{
    // add size of ip and udp header to given size before calculating
#if 1
    double sumPacketSize = (double)(_packetsCalculated) * _averagePacketSize + (double)(size + 20 + 8);
    _averagePacketSize = sumPacketSize / (double)(++_packetsCalculated);
#else // if 1
    _averagePacketSize += ((double)(size + 20 + 8) - _averagePacketSize) / (double)(++_packetsCalculated);
#endif // if 1
}

bool RTCP::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

} // namespace rtp

} // namespace inet

