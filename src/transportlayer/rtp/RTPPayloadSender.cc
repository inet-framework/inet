/***************************************************************************
                          RTPPayloadSender.cc  -  description
                             -------------------
    (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
    (C) 2001 Matthias Oppitz, Arndt Buschmann <Matthias.Oppitz@gmx.de> <a.buschmann@gmx.de>

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "inet/transportlayer/rtp/RTPPayloadSender.h"

#include "inet/transportlayer/rtp/RTPInterfacePacket_m.h"
#include "inet/transportlayer/rtp/RTPInnerPacket.h"

namespace inet {

namespace rtp {

Define_Module(RTPPayloadSender);

RTPPayloadSender::RTPPayloadSender()
{
    _reminderMessage = NULL;
}

RTPPayloadSender::~RTPPayloadSender()
{
    closeSourceFile();
    cancelAndDelete(_reminderMessage);
}

void RTPPayloadSender::initialize()
{
    cSimpleModule::initialize();
    _mtu = 0;
    _ssrc = 0;
    _payloadType = 0;
    _clockRate = 0;
    _timeStampBase = intrand(65535);
    _timeStamp = _timeStampBase;
    _sequenceNumberBase = intrand(0x7fffffff);
    _sequenceNumber = _sequenceNumberBase;
    _reminderMessage = NULL;
}

void RTPPayloadSender::handleMessage(cMessage *msg)
{
    if (msg == _reminderMessage) {
        delete msg;
        _reminderMessage = NULL;
        if (!sendPacket()) {
            endOfFile();
        }
    }
    else if (msg->getArrivalGateId() == findGate("profileIn")) {
        RTPInnerPacket *rinpIn = check_and_cast<RTPInnerPacket *>(msg);
        if (rinpIn->getType() == RTP_INP_INITIALIZE_SENDER_MODULE) {
            initializeSenderModule(rinpIn);
        }
        else if (rinpIn->getType() == RTP_INP_SENDER_MODULE_CONTROL) {
            RTPSenderControlMessage *rscm = (RTPSenderControlMessage *)(rinpIn->decapsulate());
            delete rinpIn;
            switch (rscm->getCommand()) {
                case RTP_CONTROL_PLAY:
                    play();
                    break;

                case RTP_CONTROL_PLAY_UNTIL_TIME:
                    playUntilTime(rscm->getCommandParameter1());
                    break;

                case RTP_CONTROL_PLAY_UNTIL_BYTE:
                    playUntilByte(rscm->getCommandParameter1());
                    break;

                case RTP_CONTROL_PAUSE:
                    pause();
                    break;

                case RTP_CONTROL_STOP:
                    stop();
                    break;

                case RTP_CONTROL_SEEK_TIME:
                    seekTime(rscm->getCommandParameter1());
                    break;

                case RTP_CONTROL_SEEK_BYTE:
                    seekByte(rscm->getCommandParameter1());
                    break;

                default:
                    throw cRuntimeError("unknown sender control message");
                    break;
            }
            delete rscm;
        }
    }
    else {
        throw cRuntimeError("Unknown message!");
    }
}

void RTPPayloadSender::initializeSenderModule(RTPInnerPacket *rinpIn)
{
    EV_TRACE << "initializeSenderModule Enter" << endl;
    _mtu = rinpIn->getMTU();
    _ssrc = rinpIn->getSsrc();
    const char *fileName = rinpIn->getFileName();
    openSourceFile(fileName);
    delete rinpIn;
    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleInitialized()");
    rinpOut->setSenderModuleInitializedPkt(_ssrc, _payloadType, _clockRate, _timeStampBase, _sequenceNumberBase);
    send(rinpOut, "profileOut");
    _status = STOPPED;
    EV_TRACE << "initializeSenderModule Exit" << endl;
}

void RTPPayloadSender::openSourceFile(const char *fileName)
{
    _inputFileStream.open(fileName);
    if (!_inputFileStream)
        throw cRuntimeError("Error opening data file '%s'", fileName);
}

void RTPPayloadSender::closeSourceFile()
{
    _inputFileStream.close();
}

void RTPPayloadSender::play()
{
    _status = PLAYING;
    RTPSenderStatusMessage *rssm = new RTPSenderStatusMessage("PLAYING");
    rssm->setStatus(RTP_SENDER_STATUS_PLAYING);
    rssm->setTimeStamp(_timeStamp);
    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleStatus(PLAYING)");
    rinpOut->setSenderModuleStatusPkt(_ssrc, rssm);
    send(rinpOut, "profileOut");

    if (!sendPacket()) {
        endOfFile();
    }
}

void RTPPayloadSender::playUntilTime(simtime_t moment)
{
    throw cRuntimeError("playUntilTime() not implemented");
}

void RTPPayloadSender::playUntilByte(int position)
{
    throw cRuntimeError("playUntilByte() not implemented");
}

void RTPPayloadSender::pause()
{
    cancelAndDelete(_reminderMessage);
    _reminderMessage = NULL;
    _status = STOPPED;
    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleStatus(PAUSED)");
    RTPSenderStatusMessage *rsim = new RTPSenderStatusMessage();
    rsim->setStatus(RTP_SENDER_STATUS_PAUSED);
    rinpOut->setSenderModuleStatusPkt(_ssrc, rsim);
    send(rinpOut, "profileOut");
}

void RTPPayloadSender::seekTime(simtime_t moment)
{
    throw cRuntimeError("seekTime() not implemented");
}

void RTPPayloadSender::seekByte(int position)
{
    throw cRuntimeError("seekByte() not implemented");
}

void RTPPayloadSender::stop()
{
    cancelAndDelete(_reminderMessage);
    _reminderMessage = NULL;
    _status = STOPPED;
    RTPSenderStatusMessage *rssm = new RTPSenderStatusMessage("STOPPED");
    rssm->setStatus(RTP_SENDER_STATUS_STOPPED);
    RTPInnerPacket *rinp = new RTPInnerPacket("senderModuleStatus(STOPPED)");
    rinp->setSenderModuleStatusPkt(_ssrc, rssm);
    send(rinp, "profileOut");
}

void RTPPayloadSender::endOfFile()
{
    _status = STOPPED;
    RTPSenderStatusMessage *rssm = new RTPSenderStatusMessage("FINISHED");
    rssm->setStatus(RTP_SENDER_STATUS_FINISHED);
    RTPInnerPacket *rinpOut = new RTPInnerPacket("senderModuleStatus(FINISHED)");
    rinpOut->setSenderModuleStatusPkt(_ssrc, rssm);
    send(rinpOut, "profileOut");
}

bool RTPPayloadSender::sendPacket()
{
    throw cRuntimeError("sendPacket() not implemented");
    return false;
}

} // namespace rtp

} // namespace inet

