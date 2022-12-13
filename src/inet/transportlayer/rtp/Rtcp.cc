//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi <ahmed.ayadi@sophia.inria.fr>
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/transportlayer/rtp/Rtcp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/rtp/RtcpPacket_m.h"
#include "inet/transportlayer/rtp/RtpInnerPacket_m.h"
#include "inet/transportlayer/rtp/RtpParticipantInfo.h"
#include "inet/transportlayer/rtp/RtpReceiverInfo.h"
#include "inet/transportlayer/rtp/RtpSenderInfo.h"

namespace inet {
namespace rtp {

Define_Module(Rtcp);

Rtcp::Rtcp()
{
}

void Rtcp::initialize(int stage)
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
        cModule *node = findContainingNode(this);
        NodeStatus *nodeStatus = node ? check_and_cast_nullable<NodeStatus *>(node->getSubmodule("status")) : nullptr;
        bool isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

Rtcp::~Rtcp()
{
    if (!_ssrcChosen)
        delete _senderInfo;
}

void Rtcp::handleMessage(cMessage *msg)
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
}

//
// handle messages from different gates
//

void Rtcp::handleMessageFromRTP(cMessage *msg)
{
    // from the rtp module all messages are of type RtpInnerPacket
    RtpInnerPacket *rinp = check_and_cast<RtpInnerPacket *>(msg);

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
            throw cRuntimeError("Unknown RtpInnerPacket type");
    }

    delete msg;
}

void Rtcp::handleMessageFromUDP(cMessage *msg)
{
    // from SocketLayer all message are of type cMessage
    readRet(check_and_cast<Packet *>(msg));
}

void Rtcp::handleSelfMessage(cMessage *msg)
{
    // it's time to create an rtcp packet
    if (!_ssrcChosen) {
        chooseSSRC();
        RtpInnerPacket *rinp1 = new RtpInnerPacket("rtcpInitialized()");
        rinp1->setRtcpInitializedPkt(_senderInfo->getSsrc());
        send(rinp1, "rtpOut");
    }

    createPacket();

    if (!_leaveSession) {
        scheduleInterval();
    }
    delete msg;
}

//
// methods for different messages
//

void Rtcp::handleInitializeRTCP(RtpInnerPacket *rinp)
{
    _mtu = rinp->getMtu();
    _bandwidth = rinp->getBandwidth();
    _rtcpPercentage = rinp->getRtcpPercentage();
    _destinationAddress = rinp->getAddress();
    _port = rinp->getPort();

    _senderInfo = new RtpSenderInfo();

    SdesItem *sdesItem = new SdesItem(SdesItem::SDES_CNAME, rinp->getCommonName());
    _senderInfo->addSDESItem(sdesItem);

    // create server socket for receiving rtcp packets
    createSocket();
}

void Rtcp::handleSenderModuleInitialized(RtpInnerPacket *rinp)
{
    _senderInfo->setStartTime(simTime());
    _senderInfo->setClockRate(rinp->getClockRate());
    _senderInfo->setTimeStampBase(rinp->getTimeStampBase());
    _senderInfo->setSequenceNumberBase(rinp->getSequenceNumberBase());
}

void Rtcp::handleDataOut(RtpInnerPacket *innerPacket)
{
    Packet *packet = check_and_cast<Packet *>(innerPacket->decapsulate());
    processOutgoingRTPPacket(packet);
}

void Rtcp::handleDataIn(RtpInnerPacket *rinp)
{
    Packet *packet = check_and_cast<Packet *>(rinp->decapsulate());
//    rtpPacket->dump();
    processIncomingRTPPacket(packet, rinp->getAddress(), rinp->getPort());
}

void Rtcp::handleLeaveSession(RtpInnerPacket *rinp)
{
    _leaveSession = true;
}

void Rtcp::connectRet()
{
    // schedule first rtcp packet
    double intervalLength = 2.5 * (dblrand() + 0.5);
    cMessage *reminderMessage = new cMessage("Interval");
    scheduleAfter(intervalLength, reminderMessage);
}

void Rtcp::readRet(Packet *sifpIn)
{
    emit(packetReceivedSignal, sifpIn);
    processIncomingRTCPPacket(sifpIn, Ipv4Address(_destinationAddress), _port);
}

void Rtcp::createSocket()
{
    _udpSocket.bind(_port); // TODO this will fail if this function is invoked multiple times; not sure that may (or is expected to) happen
    MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
    _udpSocket.joinLocalMulticastGroups(mgl); // TODO make it parameter-dependent
    connectRet();
}

void Rtcp::scheduleInterval()
{
    simtime_t intervalLength = _averagePacketSize * (simtime_t)(_participantInfos.size())
        / (simtime_t)(_bandwidth * _rtcpPercentage * (_senderInfo->isSender() ? 1.0 : 0.75) / 100.0);

    // interval length must be at least 5 seconds
    if (intervalLength < 5.0)
        intervalLength = 5.0;

    // to avoid rtcp packet bursts multiply calculated interval length
    // with a random number between 0.5 and 1.5
    intervalLength = intervalLength * (0.5 + dblrand());

    intervalLength /= (double)(2.71828 - 1.5); // [RFC 3550] , by Ahmed ayadi

    cMessage *reminderMessage = new cMessage("Interval");
    scheduleAfter(intervalLength, reminderMessage);
}

void Rtcp::chooseSSRC()
{
    uint32_t ssrc = 0;
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

void Rtcp::createPacket()
{
    // first packet in an rtcp compound packet must
    // be a sender or receiver report
    Ptr<RtcpReceiverReportPacket> reportPacket = nullptr;

    // if this rtcp end system is a sender (see SenderInformation::isSender() for
    // details) insert a sender report
    if (_senderInfo->isSender()) {
        const auto& senderReportPacket = makeShared<RtcpSenderReportPacket>();
        SenderReport *senderReport = _senderInfo->senderReport(simTime());
        senderReportPacket->setSenderReport(*senderReport);
        delete senderReport;
        reportPacket = senderReportPacket;
    }
    else {
        reportPacket = makeShared<RtcpReceiverReportPacket>();
    }
    reportPacket->setSsrc(_senderInfo->getSsrc());

    // insert receiver reports for packets from other sources
    for (int i = 0; i < _participantInfos.size(); i++) {
        if (_participantInfos.exist(i)) {
            RtpParticipantInfo *participantInfo = check_and_cast<RtpParticipantInfo *>(_participantInfos.get(i));
            if (participantInfo->getSsrc() != _senderInfo->getSsrc()) {
                ReceptionReport *report = check_and_cast<RtpReceiverInfo *>(participantInfo)->receptionReport(simTime());
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
    reportPacket->paddingAndSetLength();

    // insert source description items (at least common name)
    const auto& sdesPacket = makeShared<RtcpSdesPacket>();

    SdesChunk *chunk = _senderInfo->getSDESChunk();
    sdesPacket->addSDESChunk(chunk);
    sdesPacket->paddingAndSetLength();

    Packet *compoundPacket = new Packet("RtcpCompoundPacket");

    compoundPacket->insertAtBack(reportPacket);
    compoundPacket->insertAtBack(sdesPacket);

    // create rtcp app/bye packets if needed
    if (_leaveSession) {
        const auto& byePacket = makeShared<RtcpByePacket>();
        byePacket->setSsrc(_senderInfo->getSsrc());
        byePacket->paddingAndSetLength();
        compoundPacket->insertAtBack(byePacket);
    }

    calculateAveragePacketSize(compoundPacket->getByteLength());

    _udpSocket.sendTo(compoundPacket, _destinationAddress, _port);

    if (_leaveSession) {
        RtpInnerPacket *rinp = new RtpInnerPacket("sessionLeft()");
        rinp->setSessionLeftPkt();
        send(rinp, "rtpOut");
    }
}

void Rtcp::processOutgoingRTPPacket(Packet *packet)
{
    _senderInfo->processRTPPacket(packet, getId(), simTime());
}

void Rtcp::processIncomingRTPPacket(Packet *packet, Ipv4Address address, int port)
{
    bool good = false;
    const auto& rtpHeader = packet->peekAtFront<RtpHeader>();
    uint32_t ssrc = rtpHeader->getSsrc();
    RtpParticipantInfo *participantInfo = findParticipantInfo(ssrc);
    if (participantInfo == nullptr) {
        participantInfo = new RtpParticipantInfo(ssrc);
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

void Rtcp::processIncomingRTCPPacket(Packet *packet, Ipv4Address address, int port)
{
    calculateAveragePacketSize(packet->getByteLength());
    simtime_t arrivalTime = packet->getArrivalTime();

    while (packet->getByteLength() > 0) {
        // remove the rtcp packet from the rtcp compound packet
        const auto& rtcpPacket = packet->popAtFront<RtcpPacket>();
        if (rtcpPacket) {
            switch (rtcpPacket->getPacketType()) {
                case RTCP_PT_SR:
                    processIncomingRTCPSenderReportPacket(CHK(dynamicPtrCast<const RtcpSenderReportPacket>(rtcpPacket)), address, port);
                    break;

                case RTCP_PT_RR:
                    processIncomingRTCPReceiverReportPacket(CHK(dynamicPtrCast<const RtcpReceiverReportPacket>(rtcpPacket)), address, port);
                    break;

                case RTCP_PT_SDES:
                    processIncomingRTCPSDESPacket(CHK(dynamicPtrCast<const RtcpSdesPacket>(rtcpPacket)), address, port, arrivalTime);
                    break;

                case RTCP_PT_BYE:
                    processIncomingRTCPByePacket(CHK(dynamicPtrCast<const RtcpByePacket>(rtcpPacket)), address, port);
                    break;

                default:
                    throw cRuntimeError("unknown Rtcp packet type");
                    break;
            }
        }
    }
    delete packet;
}

void Rtcp::processIncomingRTCPSenderReportPacket(const Ptr<const RtcpSenderReportPacket>& rtcpSenderReportPacket, Ipv4Address address, int port)
{
    uint32_t ssrc = rtcpSenderReportPacket->getSsrc();
    RtpParticipantInfo *participantInfo = findParticipantInfo(ssrc);

    if (participantInfo == nullptr) {
        participantInfo = new RtpReceiverInfo(ssrc);
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

    const cArray& receptionReports = rtcpSenderReportPacket->getReceptionReports();
    for (int j = 0; j < receptionReports.size(); j++) {
        if (receptionReports.exist(j)) {
            const ReceptionReport *receptionReport = check_and_cast<const ReceptionReport *>(receptionReports.get(j));
            if (_senderInfo && (receptionReport->getSsrc() == _senderInfo->getSsrc())) {
                _senderInfo->processReceptionReport(receptionReport, simTime());
            }
        }
    }
}

void Rtcp::processIncomingRTCPReceiverReportPacket(const Ptr<const RtcpReceiverReportPacket>& rtcpReceiverReportPacket, Ipv4Address address, int port)
{
    uint32_t ssrc = rtcpReceiverReportPacket->getSsrc();
    RtpParticipantInfo *participantInfo = findParticipantInfo(ssrc);
    if (participantInfo == nullptr) {
        participantInfo = new RtpReceiverInfo(ssrc);
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

    const cArray& receptionReports = rtcpReceiverReportPacket->getReceptionReports();
    for (int j = 0; j < receptionReports.size(); j++) {
        if (receptionReports.exist(j)) {
            const ReceptionReport *receptionReport = check_and_cast<const ReceptionReport *>(receptionReports.get(j));
            if (_senderInfo && (receptionReport->getSsrc() == _senderInfo->getSsrc())) {
                _senderInfo->processReceptionReport(receptionReport, simTime());
            }
        }
    }
}

void Rtcp::processIncomingRTCPSDESPacket(const Ptr<const RtcpSdesPacket>& rtcpSDESPacket, Ipv4Address address, int port, simtime_t arrivalTime)
{
    const cArray& sdesChunks = rtcpSDESPacket->getSdesChunks();

    for (int j = 0; j < sdesChunks.size(); j++) {
        if (sdesChunks.exist(j)) {
            // remove the sdes chunk from the cArray of sdes chunks
            const SdesChunk *sdesChunk = check_and_cast<const SdesChunk *>(sdesChunks.get(j));
            // this is needed to avoid seg faults
//            sdesChunk->setOwner(this);
            uint32_t ssrc = sdesChunk->getSsrc();
            RtpParticipantInfo *participantInfo = findParticipantInfo(ssrc);
            if (participantInfo == nullptr) {
                participantInfo = new RtpReceiverInfo(ssrc);
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

void Rtcp::processIncomingRTCPByePacket(const Ptr<const RtcpByePacket>& rtcpByePacket, Ipv4Address address, int port)
{
    uint32_t ssrc = rtcpByePacket->getSsrc();
    RtpParticipantInfo *participantInfo = findParticipantInfo(ssrc);

    if (participantInfo != nullptr && participantInfo != _senderInfo) {
        _participantInfos.remove(participantInfo);

        delete participantInfo;
        // perhaps it would be useful to inform
        // the profile to remove the corresponding
        // receiver module
    }
}

RtpParticipantInfo *Rtcp::findParticipantInfo(uint32_t ssrc)
{
    std::string ssrcString = RtpParticipantInfo::ssrcToName(ssrc);
    return check_and_cast_nullable<RtpParticipantInfo *>(_participantInfos.get(ssrcString.c_str()));
}

void Rtcp::calculateAveragePacketSize(int size)
{
    // add size of ip and udp header to given size before calculating
    double sumPacketSize = (double)(_packetsCalculated) * _averagePacketSize + (double)(size + 20 + 8);
    _averagePacketSize = sumPacketSize / (double)(++_packetsCalculated);
}

} // namespace rtp
} // namespace inet

