/*
 *  TCPVideoStreamCliApp.cc
 *
 *  It's an adaptation of the code of Navarro Joaquim (https://github.com/navarrojoaquin/adaptive-video-tcp-omnet).
 *  Created on 8 de dez de 2017 by Anderson Andrei da Silva & Patrick Menani Abrahão at University of São Paulo
 *
 */

#include "TcpBasicVideoStreamCliApp.h"

#include "inet/applications/tcpapp/GenericAppMsg_m.h"

namespace inet {

#define MSGKIND_CONNECT     0
#define MSGKIND_SEND        1
#define MSGKIND_VIDEO_PLAY  2

//Register_Class(TcpBasicVideoStreamCliApp);
Define_Module(TcpBasicVideoStreamCliApp);

simsignal_t TcpBasicVideoStreamCliApp::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t TcpBasicVideoStreamCliApp::sentPkSignal = registerSignal("sentPk");

TcpBasicVideoStreamCliApp::~TcpBasicVideoStreamCliApp() {
    cancelAndDelete(timeoutMsg);
}

TcpBasicVideoStreamCliApp::TcpBasicVideoStreamCliApp() {
    timeoutMsg = NULL;
}


void TcpBasicVideoStreamCliApp::initialize(int stage) {
    TcpBasicClientApp::initialize(stage);
    if (stage != 3)
        return;

    // read video parameters
    const char *str = par("video_packet_size_per_second").stringValue();
    video_packet_size_per_second = cStringTokenizer(str).asIntVector();
    video_buffer_max_length = par("video_buffer_max_length");
    video_duration = par("video_duration");
    manifest_size = par("manifest_size");
    numRequestsToSend = video_duration;
    WATCH(video_buffer);
    video_buffer = 0;
    video_playback_pointer = 0;
    WATCH(video_playback_pointer);

    video_current_quality_index = par("video_current_quality_index");  // start with min quality

    video_is_playing = false;

    //statistics
    msgsRcvd = msgsSent = bytesRcvd = bytesSent = 0;

    WATCH(msgsRcvd);
    WATCH(msgsSent);
    WATCH(bytesRcvd);
    WATCH(bytesSent);

    WATCH(numRequestsToSend);

    video_startTime = par("video_startTime").doubleValue();
    stopTime = par("stopTime");
    if (stopTime != 0 && stopTime <= video_startTime)
        error("Invalid startTime/stopTime parameters");

    timeoutMsg = new cMessage("timer");
    timeoutMsg->setKind(MSGKIND_CONNECT);

    EV<< "start time: " << video_startTime << "\n";
    scheduleAt(simTime()+(simtime_t)video_startTime, timeoutMsg);
}

void TcpBasicVideoStreamCliApp::sendRequest() {
    EV<< "sending request, " << numRequestsToSend-1 << " more to go\n";

    // Request length
    long requestLength = par("requestLength");
    if (requestLength < 1) requestLength = 1;

    // Reply length
    long replyLength = -1;

    if (manifestAlreadySent) {
        replyLength = video_packet_size_per_second[video_current_quality_index] / 8 * 1000;  // kbits -> bytes
        // Log requested quality
        //emit(DASH_quality_level_signal, video_current_quality_index);

    } else {
        replyLength = manifest_size;
        EV<< "sending manifest request\n";
    }

    numRequestsToSend--;
    msgsSent++;
    bytesSent += requestLength;

//    GenericAppMsg *msg = new GenericAppMsg("data");
//    msg->setByteLength(requestLength);
//    msg->setExpectedReplyLength(replyLength);
//    msg->setServerClose(false);

    const auto& payload = makeShared<GenericAppMsg>();
    Packet *msg = new Packet("data");
    payload->setChunkLength(B(requestLength));
    payload->setExpectedReplyLength(replyLength);
    payload->setServerClose(false);
    msg->insertAtBack(payload);

    sendPacket(msg);
}

void TcpBasicVideoStreamCliApp::handleTimer(cMessage *msg) {
    switch (msg->getKind()) {
    case MSGKIND_CONNECT:
        EV<< "starting session\n";
        connect(); // active OPEN
        break;

        case MSGKIND_SEND:
        sendRequest();
        // no scheduleAt(): next request will be sent when reply to this one
        // arrives (see socketDataArrived())
        break;

        case MSGKIND_VIDEO_PLAY:
            EV<< "---------------------> Video play event";
            cancelAndDelete(msg);
            video_buffer--;
            video_playback_pointer++;
            break;

        default:
        throw cRuntimeError("Invalid timer msg: kind=%d", msg->getKind());
    }
}

void TcpBasicVideoStreamCliApp::socketEstablished(TcpSocket *socket) {
    TcpBasicClientApp::socketEstablished(socket);

    // perform first request
    sendRequest();

}

void TcpBasicVideoStreamCliApp::rescheduleOrDeleteTimer(simtime_t d,
        short int msgKind) {
    cancelEvent (timeoutMsg);

    if (stopTime == 0 || stopTime > d || timeoutMsg) {
        timeoutMsg->setKind(msgKind);
        scheduleAt(d, timeoutMsg);
    } else {
        delete timeoutMsg;
        timeoutMsg = NULL;
    }
}

void TcpBasicVideoStreamCliApp::socketDataArrived(TcpSocket *socket, Packet *msg, bool urgent) {
    TcpBasicClientApp::socketDataArrived(socket, msg, urgent);

    if (!manifestAlreadySent) {
        manifestAlreadySent = true;
        if (timeoutMsg) {
            // Send new request
            simtime_t d = simTime();
            rescheduleOrDeleteTimer(d, MSGKIND_SEND);
        }
        return;
    }

    video_buffer++;

    // Full buffer
    if (video_buffer == video_buffer_max_length) {
        video_is_buffering = false;
        // the next video fragment will be requested when the buffer gets some space, so nothing to do here.
        return;
    }
    EV<< "---------------------> Buffer=" << video_buffer << "    min= " << video_buffer_min_rebuffering << "\n";
    // Exit rebuffering state and continue the video playback
    if (video_buffer > video_buffer_min_rebuffering || (numRequestsToSend == 0 && video_playback_pointer < video_duration) ) {
        if (!video_is_playing) {
            video_is_playing = true;
            simtime_t d = simTime() + 1;   // the +1 represents the time when the video fragment has been consumed and therefore has to be removed from the buffer.
            cMessage *videoPlaybackMsg = new cMessage("playback");
            videoPlaybackMsg->setKind(MSGKIND_VIDEO_PLAY);
            scheduleAt(d, videoPlaybackMsg);
            //rescheduleOrDeleteTimer(d, MSGKIND_VIDEO_PLAY);
        }
    } else {
        video_current_quality_index = std::max(video_current_quality_index - 1, 0);
    }

    if (numRequestsToSend > 0) {
        EV<< "reply arrived\n";
        if (timeoutMsg)
        {
            // Send new request
            simtime_t d = simTime();
            rescheduleOrDeleteTimer(d, MSGKIND_SEND);
        }
        int recvd = video_packet_size_per_second[video_current_quality_index] / 8 * 1000;
        msgsRcvd++;
        bytesRcvd += recvd;
        //emit(rcvdPkSignal,recvd);
    }
    else
    {
        EV << "reply to last request arrived, closing session\n";
        close();
    }
}

void TcpBasicVideoStreamCliApp::socketClosed(TcpSocket *socket) {
    TcpBasicClientApp::socketClosed(socket);

    // Nothing to do here...
}

void TcpBasicVideoStreamCliApp::socketFailure(TcpSocket *socket, int code) {
    TcpBasicClientApp::socketFailure(socket, code);

}

void TcpBasicVideoStreamCliApp::refreshDisplay() const
{
    char buf[64];
    sprintf(buf, "rcvd: %ld pks %ld bytes\nsent: %ld pks %ld bytes", msgsRcvd, bytesRcvd, msgsSent, bytesSent);
    getDisplayString().setTagArg("t", 0, buf);
}
}


