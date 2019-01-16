/***************************************************************************
                          RtpPayloadSender.cc  -  description
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

#include "inet/transportlayer/rtp/RtpInnerPacket_m.h"
#include "inet/transportlayer/rtp/RtpInterfacePacket_m.h"
#include "inet/transportlayer/rtp/RtpPayloadSender.h"

namespace inet {
namespace rtp {

Define_Module(RtpPayloadSender);

RtpPayloadSender::RtpPayloadSender()
{
}

RtpPayloadSender::~RtpPayloadSender()
{
    closeSourceFile();
    cancelAndDelete(_reminderMessage);
}

void RtpPayloadSender::initialize()
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
    _reminderMessage = nullptr;
}

void RtpPayloadSender::handleMessage(cMessage *msg)
{
    if (msg == _reminderMessage) {
        delete msg;
        _reminderMessage = nullptr;
        if (!sendPacket()) {
            endOfFile();
        }
    }
    else if (msg->getArrivalGateId() == findGate("profileIn")) {
        RtpInnerPacket *rinpIn = check_and_cast<RtpInnerPacket *>(msg);
        if (rinpIn->getType() == RTP_INP_INITIALIZE_SENDER_MODULE) {
            initializeSenderModule(rinpIn);
        }
        else if (rinpIn->getType() == RTP_INP_SENDER_MODULE_CONTROL) {
            RtpSenderControlMessage *rscm = check_and_cast<RtpSenderControlMessage *>(rinpIn->decapsulate());
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

void RtpPayloadSender::initializeSenderModule(RtpInnerPacket *rinpIn)
{
    EV_TRACE << "initializeSenderModule Enter" << endl;
    _mtu = rinpIn->getMtu();
    _ssrc = rinpIn->getSsrc();
    const char *fileName = rinpIn->getFileName();
    openSourceFile(fileName);
    delete rinpIn;
    RtpInnerPacket *rinpOut = new RtpInnerPacket("senderModuleInitialized()");
    rinpOut->setSenderModuleInitializedPkt(_ssrc, _payloadType, _clockRate, _timeStampBase, _sequenceNumberBase);
    send(rinpOut, "profileOut");
    _status = STOPPED;
    EV_TRACE << "initializeSenderModule Exit" << endl;
}

void RtpPayloadSender::openSourceFile(const char *fileName)
{
    _inputFileStream.open(fileName);
    if (!_inputFileStream)
        throw cRuntimeError("Error opening data file '%s'", fileName);
}

void RtpPayloadSender::closeSourceFile()
{
    _inputFileStream.close();
}

void RtpPayloadSender::play()
{
    _status = PLAYING;
    RtpSenderStatusMessage *rssm = new RtpSenderStatusMessage("PLAYING");
    rssm->setStatus(RTP_SENDER_STATUS_PLAYING);
    rssm->setTimeStamp(_timeStamp);
    RtpInnerPacket *rinpOut = new RtpInnerPacket("senderModuleStatus(PLAYING)");
    rinpOut->setSenderModuleStatusPkt(_ssrc, rssm);
    send(rinpOut, "profileOut");

    if (!sendPacket()) {
        endOfFile();
    }
}

void RtpPayloadSender::playUntilTime(simtime_t moment)
{
    throw cRuntimeError("playUntilTime() not implemented");
}

void RtpPayloadSender::playUntilByte(int position)
{
    throw cRuntimeError("playUntilByte() not implemented");
}

void RtpPayloadSender::pause()
{
    cancelAndDelete(_reminderMessage);
    _reminderMessage = nullptr;
    _status = STOPPED;
    RtpInnerPacket *rinpOut = new RtpInnerPacket("senderModuleStatus(PAUSED)");
    RtpSenderStatusMessage *rsim = new RtpSenderStatusMessage();
    rsim->setStatus(RTP_SENDER_STATUS_PAUSED);
    rinpOut->setSenderModuleStatusPkt(_ssrc, rsim);
    send(rinpOut, "profileOut");
}

void RtpPayloadSender::seekTime(simtime_t moment)
{
    throw cRuntimeError("seekTime() not implemented");
}

void RtpPayloadSender::seekByte(int position)
{
    throw cRuntimeError("seekByte() not implemented");
}

void RtpPayloadSender::stop()
{
    cancelAndDelete(_reminderMessage);
    _reminderMessage = nullptr;
    _status = STOPPED;
    RtpSenderStatusMessage *rssm = new RtpSenderStatusMessage("STOPPED");
    rssm->setStatus(RTP_SENDER_STATUS_STOPPED);
    RtpInnerPacket *rinp = new RtpInnerPacket("senderModuleStatus(STOPPED)");
    rinp->setSenderModuleStatusPkt(_ssrc, rssm);
    send(rinp, "profileOut");
}

void RtpPayloadSender::endOfFile()
{
    _status = STOPPED;
    RtpSenderStatusMessage *rssm = new RtpSenderStatusMessage("FINISHED");
    rssm->setStatus(RTP_SENDER_STATUS_FINISHED);
    RtpInnerPacket *rinpOut = new RtpInnerPacket("senderModuleStatus(FINISHED)");
    rinpOut->setSenderModuleStatusPkt(_ssrc, rssm);
    send(rinpOut, "profileOut");
}

bool RtpPayloadSender::sendPacket()
{
    throw cRuntimeError("sendPacket() not implemented");
    return false;
}

} // namespace rtp
} // namespace inet

