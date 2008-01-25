/***************************************************************************
                          RTPPayloadSender.cc  -  description
                             -------------------
    begin                : Fri Nov 2 2001
    copyright            : (C) 2001 by Matthias Oppitz, Arndt Buschmann
    email                : <matthias.oppitz@gmx.de> <a.buschmann@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** \file RTPPayloadSender.cc
 * This file contains the implementation of member functions of the class
 * RTPPayloadSender.
 */

#include <omnetpp.h>
#include "RTPPayloadSender.h"


Define_Module(RTPPayloadSender);


RTPPayloadSender::~RTPPayloadSender() {
    closeSourceFile();
}


void RTPPayloadSender::initialize() {
    cSimpleModule::initialize();
    _mtu = 0;
    _ssrc = 0;
    _payloadType = 0;
    _clockRate = 0;
    _timeStampBase = intrand(65535);
    _timeStamp = _timeStampBase;
    _sequenceNumberBase = intrand(0x7fffffff);
    _sequenceNumber = _sequenceNumberBase;
};


void RTPPayloadSender::activity() {
    const char *command;
    while (true) {
        cMessage *msg = receive();
        if (msg->arrivalGateId() == findGate("fromProfile")) {
            RTPInnerPacket *rinpIn = (RTPInnerPacket *)msg;
            if (rinpIn->type() == RTPInnerPacket::RTP_INP_INITIALIZE_SENDER_MODULE) {
                initializeSenderModule(rinpIn);
            }
            else if (rinpIn->type() == RTPInnerPacket::RTP_INP_SENDER_MODULE_CONTROL) {
                RTPSenderControlMessage *rscm = (RTPSenderControlMessage *)(rinpIn->decapsulate());
                delete rinpIn;
                command = rscm->command();
                if (!opp_strcmp(command, "PLAY")) {
                    play();
                }
                else if (!opp_strcmp(command, "PLAY_UNTIL_TIME")) {
                    playUntilTime(rscm->commandParameter1());
                }
                else if (!opp_strcmp(command, "PLAY_UNTIL_BYTE")) {
                    playUntilByte(rscm->commandParameter1());
                }
                else if (!opp_strcmp(command, "PAUSE")) {
                    pause();
                }
                else if (!opp_strcmp(command, "STOP")) {
                    stop();
                }
                else if (!opp_strcmp(command, "SEEK_TIME")) {
                    seekTime(rscm->commandParameter1());
                }
                else if (!opp_strcmp(command, "SEEK_BYTE")) {
                    seekByte(rscm->commandParameter1());
                }
                else {
                    EV << "sender module: unknown sender control message" << endl;
                };
                delete rscm;
            }
        }
        else {
            if (!sendPacket()) {
                endOfFile();
            }
            delete msg;
        }
    }
};


void RTPPayloadSender::initializeSenderModule(RTPInnerPacket *rinpIn) {
    _mtu = rinpIn->mtu();
    _ssrc = rinpIn->ssrc();
    const char *fileName = rinpIn->fileName();
    openSourceFile(fileName);
    delete rinpIn;
    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleInitialized()");
    rinpOut->senderModuleInitialized(_ssrc, _payloadType, _clockRate, _timeStampBase, _sequenceNumberBase);
    send(rinpOut, "toProfile");
    _status = STOPPED;
};


void RTPPayloadSender::openSourceFile(const char *fileName) {
    _inputFileStream.open(fileName);
    if (!_inputFileStream) {
        opp_error("sender module: error open data file");
    }
};


void RTPPayloadSender::closeSourceFile() {
    _inputFileStream.close();
};


void RTPPayloadSender::play() {
    _status = PLAYING;
    RTPSenderStatusMessage *rssm = new RTPSenderStatusMessage("PLAYING");
    rssm->setStatus("PLAYING");
    rssm->setTimeStamp(_timeStamp);
    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleStatus(PLAYING)");
    rinpOut->senderModuleStatus(_ssrc, rssm);
    send(rinpOut, "toProfile");

    if (!sendPacket()) {
        endOfFile();
    }
};


void RTPPayloadSender::playUntilTime(simtime_t moment) {
    EV << "sender module: playUntilTime() not implemented" << endl;
};


void RTPPayloadSender::playUntilByte(int position) {
    EV << "sender module: playUntilByte() not implemented" << endl;
};


void RTPPayloadSender::pause() {
    cancelEvent(_reminderMessage);
    _status = STOPPED;
    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleStatus(PAUSED)");
    RTPSenderStatusMessage *rsim = new RTPSenderStatusMessage();
    rsim->setStatus("PAUSED");
    rinpOut->senderModuleStatus(_ssrc, rsim);
    send(rinpOut, "toProfile");
};


void RTPPayloadSender::seekTime(simtime_t moment) {
    EV << "sender module: seekTime() not implemented" << endl;
};


void RTPPayloadSender::seekByte(int position) {
    EV << "sender module: seekByte() not implemented" << endl;
};


void RTPPayloadSender::stop() {
    cancelEvent(_reminderMessage);
    _status = STOPPED;
    RTPSenderStatusMessage *rssm = new RTPSenderStatusMessage("STOPPED");
    rssm->setStatus("STOPPED");
    RTPInnerPacket *rinp = new RTPInnerPacket("senderModuleStatus(STOPPED)");
    rinp->senderModuleStatus(_ssrc, rssm);
    send(rinp, "toProfile");
};


void RTPPayloadSender::endOfFile() {
    _status = STOPPED;
    RTPSenderStatusMessage *rssm = new RTPSenderStatusMessage();
    rssm->setStatus("FINISHED");
    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleStatus(FINISHED)");
    rinpOut->senderModuleStatus(_ssrc, rssm);
    send(rinpOut, "toProfile");
};


bool RTPPayloadSender::sendPacket() {
    EV << "sender module: sendPacket() not implemented" << endl;
    return false;
};
