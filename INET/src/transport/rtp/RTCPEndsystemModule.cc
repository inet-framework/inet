/***************************************************************************
                          RTCPEndsystemModule.cc  -  description
                             -------------------
    begin                : Sun Oct 21 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** \file RTCPEndsystemModule.cc
 * This file contains the implementation of member functions of the class
 * RTCPEndsystemModule.
 */

#include <omnetpp.h>

//XXX #include "SocketInterfacePacket.h"
//XXX #include "sockets.h"
#include "tmp/defs.h"

#include "IPAddress.h"

#include "types.h"
#include "RTCPEndsystemModule.h"
#include "RTPInnerPacket.h"
#include "RTPParticipantInfo.h"
#include "RTPSenderInfo.h"
#include "RTPReceiverInfo.h"

Define_Module_Like(RTCPEndsystemModule, RTCPModule);

void RTCPEndsystemModule::initialize() {

    // initialize variables
    _ssrcChosen = false;
    _leaveSession = false;
    _socketFdIn = Socket::FILEDESC_UNDEF;
    _socketFdOut = Socket::FILEDESC_UNDEF;

    _packetsCalculated = 0;

    _averagePacketSize = 0.0;

    _participantInfos = new cArray("ParticipantInfos");

    _rtcpIntervalOutVector = new cOutVector();
};


void RTCPEndsystemModule::handleMessage(cMessage *msg) {

    // first distinguish incoming messages by arrival gate
    if (msg->arrivalGateId() == findGate("fromRTP")) {
        handleMessageFromRTP(msg);
    }
    else if (msg->arrivalGateId() == findGate("fromSocketLayer")) {
        handleMessageFromSocketLayer(msg);
    }
    else {
        handleSelfMessage(msg);
    }

    delete msg;
};


//
// handle messages from different gates
//

void RTCPEndsystemModule::handleMessageFromRTP(cMessage *msg) {

    // from the rtp module all messages are of type RTPInnerPacket
    RTPInnerPacket *rinp = (RTPInnerPacket *)msg;

    // distinguish by type
    if (rinp->type() == RTPInnerPacket::RTP_INP_INITIALIZE_RTCP) {
        initializeRTCP(rinp);
    }
    else if (rinp->type() == RTPInnerPacket::RTP_INP_SENDER_MODULE_INITIALIZED) {
        senderModuleInitialized(rinp);
    }
    else if (rinp->type() == RTPInnerPacket::RTP_INP_DATA_OUT) {
        dataOut(rinp);
    }
    else if (rinp->type() == RTPInnerPacket::RTP_INP_DATA_IN) {
        dataIn(rinp);
    }
    else if (rinp->type() == RTPInnerPacket::RTP_INP_LEAVE_SESSION) {
        leaveSession(rinp);
    }
    else {
        EV << "RTCPEndsystemModule: unknown RTPInnerPacket type !" << endl;
    }
};


void RTCPEndsystemModule::handleMessageFromSocketLayer(cMessage *msg) {
    // from SocketLayer all message are of type SocketInterfacePacket
    SocketInterfacePacket *sifpIn = (SocketInterfacePacket *)msg;

    // distinguish by action (type of SocketInterfacePacket)

    if (sifpIn->action() == SocketInterfacePacket::SA_SOCKET_RET) {
        socketRet(sifpIn);
    }
    else if (sifpIn->action() == SocketInterfacePacket::SA_CONNECT_RET) {
        connectRet(sifpIn);
    }
    else if (sifpIn->action() == SocketInterfacePacket::SA_READ_RET) {
        // we have rtcp data !
        readRet(sifpIn);
    }
    else {
        EV << "RTCPEndsystemModule: unknown SocketInterfacePacket type !" << endl;
    }
};


void RTCPEndsystemModule::handleSelfMessage(cMessage *msg) {
    // it's time to create an rtcp packet
    if (!_ssrcChosen) {
        chooseSSRC();
        RTPInnerPacket *rinp1 = new RTPInnerPacket("rtcpInitialized()");
        rinp1->rtcpInitialized(_senderInfo->ssrc());
        send(rinp1, "toRTP");
    }

    createPacket();

    if (!_leaveSession) {
        scheduleInterval();
    }
};


//
// methods for different messages
//

void RTCPEndsystemModule::initializeRTCP(RTPInnerPacket *rinp) {
    _mtu = rinp->mtu();
    _bandwidth = rinp->bandwidth();
    _rtcpPercentage = rinp->rtcpPercentage();
    _destinationAddress = rinp->address();
    _port = rinp->port();

    _senderInfo = new RTPSenderInfo();

    SDESItem *sdesItem = new SDESItem(SDESItem::SDES_CNAME, rinp->commonName());
    _senderInfo->addSDESItem(sdesItem);


    // create server socket for receiving rtcp packets
    createServerSocket();
};


void RTCPEndsystemModule::senderModuleInitialized(RTPInnerPacket *rinp) {
    _senderInfo->setStartTime(simTime());
    _senderInfo->setClockRate(rinp->clockRate());
    _senderInfo->setTimeStampBase(rinp->timeStampBase());
    _senderInfo->setSequenceNumberBase(rinp->sequenceNumberBase());
};


void RTCPEndsystemModule::dataOut(RTPInnerPacket *packet) {
    RTPPacket *rtpPacket = (RTPPacket *)(packet->decapsulate());
    processOutgoingRTPPacket(rtpPacket);
};


void RTCPEndsystemModule::dataIn(RTPInnerPacket *rinp) {
    RTPPacket *rtpPacket = (RTPPacket *)(rinp->decapsulate());
    processIncomingRTPPacket(rtpPacket, rinp->address(), rinp->port());
};


void RTCPEndsystemModule::leaveSession(RTPInnerPacket *rinp) {
    _leaveSession = true;
};


void RTCPEndsystemModule::socketRet(SocketInterfacePacket *sifpIn) {

    // first the server socket is created
    if (_socketFdIn == Socket::FILEDESC_UNDEF) {
        _socketFdIn = sifpIn->filedesc();
        // when file descriptor for the server socket is known
        // we can bind it to the port
        SocketInterfacePacket *sifpOut1 = new SocketInterfacePacket("bind()");
        IPAddress ipaddr(_destinationAddress);
        if (ipaddr.isMulticast()) {
            sifpOut1->bind(_socketFdIn, IN_Addr(_destinationAddress), IN_Port(_port));
        }
        else {
            sifpOut1->bind(_socketFdIn, IPADDRESS_UNDEF, IN_Port(_port));
        }
        send(sifpOut1, "toSocketLayer");

        createClientSocket();
    }

    // server socket already exists, client socket is created
    else if (_socketFdOut == Socket::FILEDESC_UNDEF) {

        _socketFdOut = sifpIn->filedesc();

        // now connect it, which means set foreign address and port
        SocketInterfacePacket *sifpOut = new SocketInterfacePacket("connect()");
        sifpOut->connect(_socketFdOut, _destinationAddress, _port);
        send(sifpOut, "toSocketLayer");

    }
    else {
        EV << "RTCPEndsystemModule: received unrequested socket from socket layer !" << endl;
    }
};


void RTCPEndsystemModule::connectRet(SocketInterfacePacket *sifpIn) {
    // schedule first rtcp packet
    double intervalLength = 2.5 * (dblrand() + 0.5);
    cMessage *reminderMessage = new cMessage("Interval");
    scheduleAt(simTime() + intervalLength, reminderMessage);
};


void RTCPEndsystemModule::readRet(SocketInterfacePacket *sifpIn) {
    RTCPCompoundPacket *packet = (RTCPCompoundPacket *)(sifpIn->decapsulate());
    processIncomingRTCPPacket(packet, IN_Addr(sifpIn->fAddr()), IN_Port(sifpIn->fPort()));
};


//
//
//

void RTCPEndsystemModule::createServerSocket() {
    SocketInterfacePacket *sifp = new SocketInterfacePacket("socket()");
    sifp->socket(Socket::IPSuite_AF_INET, Socket::IPSuite_SOCK_DGRAM, Socket::UDP);
    send(sifp, "toSocketLayer");
};

void RTCPEndsystemModule::createClientSocket() {
    SocketInterfacePacket *sifp = new SocketInterfacePacket("socket()");
    sifp->socket(Socket::IPSuite_AF_INET, Socket::IPSuite_SOCK_DGRAM, Socket::UDP);
    send(sifp, "toSocketLayer");
};


void RTCPEndsystemModule::scheduleInterval() {

    simtime_t intervalLength = (simtime_t)(_averagePacketSize) * (simtime_t)(_participantInfos->items()) / (simtime_t)(_bandwidth * _rtcpPercentage * (_senderInfo->isSender() ? 1.0 : 0.75) / 100.0);


    // write the calculated interval into file
    _rtcpIntervalOutVector->record(intervalLength);

    // interval length must be at least 5 seconds
    if (intervalLength < 5.0)
        intervalLength = 5.0;


    // to avoid rtcp packet bursts multiply calculated interval length
    // with a random number between 0.5 and 1.5
    intervalLength = intervalLength * (0.5 + dblrand());


    cMessage *reminderMessage = new cMessage("Interval");
    scheduleAt(simTime() + intervalLength, reminderMessage);
};


void RTCPEndsystemModule::chooseSSRC() {

    u_int32 ssrc = 0;
    bool ssrcConflict = false;
    do {
        ssrc = intrand(0x7fffffff);
        ssrcConflict = findParticipantInfo(ssrc) != NULL;
    } while (ssrcConflict);
    _senderInfo->setSSRC(ssrc);
    _participantInfos->add(_senderInfo);
    _ssrcChosen = true;
};


void RTCPEndsystemModule::createPacket() {
    // first packet in an rtcp compound packet must
    // be a sender or receiver report
    RTCPReceiverReportPacket *reportPacket;

    // if this rtcp end system is a sender (see SenderInformation::isSender() for
    // details) insert a sender report
    if (_senderInfo->isSender()) {
        RTCPSenderReportPacket *senderReportPacket = new RTCPSenderReportPacket("SenderReportPacket");
        senderReportPacket->setSenderReport(_senderInfo->senderReport(simTime()));
        reportPacket = senderReportPacket;
    }
    else {
        reportPacket = new RTCPReceiverReportPacket("ReceiverReportPacket");
    }
    reportPacket->setSSRC(_senderInfo->ssrc());

    // insert receiver reports for packets from other sources
    for (int i = 0; i < _participantInfos->items(); i++) {

        if (_participantInfos->exist(i)) {
            RTPParticipantInfo *participantInfo = (RTPParticipantInfo *)(_participantInfos->get(i));
            if (participantInfo->ssrc() != _senderInfo->ssrc()) {
                ReceptionReport *report = ((RTPReceiverInfo *)participantInfo)->receptionReport(simTime());
                if (report != NULL) {
                    reportPacket->addReceptionReport(report);

                };
            };
            participantInfo->nextInterval(simTime());

            if (participantInfo->toBeDeleted(simTime())) {

                _participantInfos->remove(participantInfo);
                delete participantInfo;
                // perhaps inform the profile
            };
        };
    };


    // insert source description items (at least common name)
    RTCPSDESPacket *sdesPacket = new RTCPSDESPacket("SDESPacket");

    SDESChunk *chunk = _senderInfo->sdesChunk();
    sdesPacket->addSDESChunk(chunk);

    RTCPCompoundPacket *compoundPacket = new RTCPCompoundPacket("RTCPCompoundPacket");

    compoundPacket->addRTCPPacket(reportPacket);

    compoundPacket->addRTCPPacket(sdesPacket);

    // create rtcp app/bye packets if needed
    if (_leaveSession) {
        RTCPByePacket *byePacket = new RTCPByePacket("ByePacket");
        byePacket->setSSRC(_senderInfo->ssrc());
        compoundPacket->addRTCPPacket(byePacket);
    };

    calculateAveragePacketSize(compoundPacket->length());

    SocketInterfacePacket *sifp = new SocketInterfacePacket("write()");

    sifp->write(_socketFdOut, compoundPacket);

    send(sifp, "toSocketLayer");

    if (_leaveSession) {
        RTPInnerPacket *rinp = new RTPInnerPacket("sessionLeft()");
        rinp->sessionLeft();
        send(rinp, "toRTP");
    };

};


void RTCPEndsystemModule::processOutgoingRTPPacket(RTPPacket *packet) {
    _senderInfo->processRTPPacket(packet, simTime());
};


void RTCPEndsystemModule::processIncomingRTPPacket(RTPPacket *packet, IN_Addr address, IN_Port port) {

    u_int32 ssrc = packet->ssrc();
    RTPParticipantInfo *participantInfo = findParticipantInfo(ssrc);
    if (participantInfo == NULL) {
        participantInfo = new RTPParticipantInfo(ssrc);
        participantInfo->setAddress(address);
        participantInfo->setRTPPort(port);
        _participantInfos->add(participantInfo);
    }
    else {
        // check for ssrc conflict
        if (participantInfo->address() != address) {
            // we have an address conflict
        }
        if (participantInfo->rtpPort() == IPSuite_PORT_UNDEF) {
            participantInfo->setRTPPort(port);
        }
        else if (participantInfo->rtpPort() != port) {
            // we have an rtp port conflict
        }
    }
    participantInfo->processRTPPacket(packet, packet->arrivalTime());
};


void RTCPEndsystemModule::processIncomingRTCPPacket(RTCPCompoundPacket *packet, IN_Addr address, IN_Port port) {

    calculateAveragePacketSize(packet->length());

    cArray *rtcpPackets = packet->rtcpPackets();

    simtime_t arrivalTime = packet->arrivalTime();
    delete packet;

    for (int i = 0; i < rtcpPackets->items(); i++) {
        if (rtcpPackets->exist(i)) {
            // remove the rtcp packet from the rtcp compound packet
            RTCPPacket *rtcpPacket = (RTCPPacket *)(rtcpPackets->remove(i));

            if (rtcpPacket->packetType() == RTCPPacket::RTCP_PT_SR) {

                RTCPSenderReportPacket *rtcpSenderReportPacket = (RTCPSenderReportPacket *)rtcpPacket;
                u_int32 ssrc = rtcpSenderReportPacket->ssrc();
                RTPParticipantInfo *participantInfo = findParticipantInfo(ssrc);

                if (participantInfo == NULL) {
                    participantInfo = new RTPReceiverInfo(ssrc);
                    participantInfo->setAddress(address);
                    participantInfo->setRTCPPort(port);
                    _participantInfos->add(participantInfo);
                }
                else {
                    if (participantInfo->address() == address) {
                        if (participantInfo->rtcpPort() == IPSuite_PORT_UNDEF) {
                            participantInfo->setRTCPPort(port);
                        }
                        else {
                            // check for ssrc conflict
                        }
                    }
                    else {
                        // check for ssrc conflict
                    };
                }
                participantInfo->processSenderReport(rtcpSenderReportPacket->senderReport(), simTime());

                cArray *receptionReports = rtcpSenderReportPacket->receptionReports();
                for (int j = 0; j < receptionReports->items(); j++) {
                    if (receptionReports->exist(j)) {
                        ReceptionReport *receptionReport = (ReceptionReport *)(receptionReports->remove(j));
                        if (_senderInfo) {
                            if (receptionReport->ssrc() == _senderInfo->ssrc()) {
                                _senderInfo->processReceptionReport(receptionReport, simTime());
                            }
                        }
                        //else
                        //    delete receiverReport;
                    }
                };
                delete receptionReports;

            }
            else if (rtcpPacket->packetType() == RTCPPacket::RTCP_PT_RR) {

                RTCPReceiverReportPacket *rtcpReceiverReportPacket = (RTCPReceiverReportPacket *)rtcpPacket;
                u_int32 ssrc = rtcpReceiverReportPacket->ssrc();
                RTPParticipantInfo *participantInfo = findParticipantInfo(ssrc);
                if (participantInfo == NULL) {
                    participantInfo = new RTPReceiverInfo(ssrc);
                    participantInfo->setAddress(address);
                    participantInfo->setRTCPPort(port);
                    _participantInfos->add(participantInfo);
                }
                else {
                    if (participantInfo->address() == address) {
                        if (participantInfo->rtcpPort() == IPSuite_PORT_UNDEF) {
                            participantInfo->setRTCPPort(port);
                        }
                        else {
                            // check for ssrc conflict
                        }
                    }
                    else {
                        // check for ssrc conflict
                    };
                }

                cArray *receptionReports = rtcpReceiverReportPacket->receptionReports();
                for (int j = 0; j < receptionReports->items(); j++) {
                    if (receptionReports->exist(j)) {
                        ReceptionReport *receptionReport = (ReceptionReport *)(receptionReports->remove(j));
                        if (_senderInfo) {

                            if (receptionReport->ssrc() == _senderInfo->ssrc()) {
                                _senderInfo->processReceptionReport(receptionReport, simTime());
                            }
                        }

                        //else
                        //    delete receiverReport;
                    }
                };
                delete receptionReports;
            }
            else if (rtcpPacket->packetType() == RTCPPacket::RTCP_PT_SDES) {

                RTCPSDESPacket *rtcpSDESPacket = (RTCPSDESPacket *)rtcpPacket;
                cArray *sdesChunks = rtcpSDESPacket->sdesChunks();

                for (int j = 0; j < sdesChunks->items(); j++) {
                    if (sdesChunks->exist(j)) {
                        // remove the sdes chunk from the cArray of sdes chunks
                        SDESChunk *sdesChunk = (SDESChunk *)(sdesChunks->remove(j));
                        // this is needed to avoid seg faults
                        //sdesChunk->setOwner(this);
                        u_int32 ssrc = sdesChunk->ssrc();
                        RTPParticipantInfo *participantInfo = findParticipantInfo(ssrc);
                        if (participantInfo == NULL) {
                            participantInfo = new RTPReceiverInfo(ssrc);
                            participantInfo->setAddress(address);
                            participantInfo->setRTCPPort(port);
                            _participantInfos->add(participantInfo);
                        }
                        else {
                            // check for ssrc conflict
                        }
                        participantInfo->processSDESChunk(sdesChunk, arrivalTime);
                    }
                }
                delete sdesChunks;

            }
            else if (rtcpPacket->packetType() == RTCPPacket::RTCP_PT_BYE) {
                RTCPByePacket *rtcpByePacket = (RTCPByePacket *)rtcpPacket;
                u_int32 ssrc = rtcpByePacket->ssrc();
                RTPParticipantInfo *participantInfo = findParticipantInfo(ssrc);

                if (participantInfo != NULL && participantInfo != _senderInfo) {
                    _participantInfos->remove(participantInfo);

                    delete participantInfo;
                    // perhaps it would be useful to inform
                    // the profile to remove the corresponding
                    // receiver module
                };
            }
            else {
                // app rtcp packets
            }
        delete rtcpPacket;
        }
    }
    delete rtcpPackets;
};


RTPParticipantInfo *RTCPEndsystemModule::findParticipantInfo(u_int32 ssrc) {
    char *ssrcString = RTPParticipantInfo::ssrcToName(ssrc);
    int participantIndex = _participantInfos->find(ssrcString);
    if (participantIndex != -1) {
        return (RTPParticipantInfo *)(_participantInfos->get(participantIndex));
    }
    else {
        return NULL;
    };
};


void RTCPEndsystemModule::calculateAveragePacketSize(int size) {
    // add size of ip and udp header to given size before calculating
    _averagePacketSize = ((double)(_packetsCalculated) * _averagePacketSize + (double)(size + 20 + 8)) / (double)(++_packetsCalculated);
};
