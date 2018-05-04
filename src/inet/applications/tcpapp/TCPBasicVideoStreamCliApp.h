/*
 *  TCPVideoStreamCliApp.cc
 *
 *  It's an adaptation of the code of Navarro Joaquim (https://github.com/navarrojoaquin/adaptive-video-tcp-omnet).
 *  Created on 8 de dez de 2017 by Anderson Andrei da Silva & Patrick Menani Abrahão at University of São Paulo
 *
 */

#ifndef INET_APPLICATIONS_TCPAPP_TCPBASICVIDEOSTREAMCLIAPP_H_
#define INET_APPLICATIONS_TCPAPP_TCPBASICVIDEOSTREAMCLIAPP_H_

#include <omnetpp.h>
#include <algorithm>
#include "inet/common/INETDefs.h"
#include "inet/applications/tcpapp/TCPBasicClientApp.h"

namespace inet {

class TCPBasicVideoStreamCliApp: public TCPBasicClientApp {

protected:

    // Video parameters
    std::vector<int> video_packet_size_per_second;
    int video_buffer_max_length;
    int video_duration;
    int numRequestsToSend; // requests to send in this session. Each request = 1s of video
    int video_buffer; // current lenght of the buffer in seconds
    int video_current_quality_index; // requested quality
    bool video_is_playing;
    int video_playback_pointer;
    bool video_is_buffering = true;
    int video_buffer_min_rebuffering = 3; // if video_buffer < video_buffer_min_rebuffering then a rebuffering event occurs
    double video_startTime;
    int manifest_size;
    bool manifestAlreadySent = false;

    // statistics:
    static simsignal_t rcvdPkSignal;
    static simsignal_t sentPkSignal;

    long msgsRcvd;
    long msgsSent;
    long bytesRcvd;
    long bytesSent;

    simtime_t startTime;
    cMessage *timeoutMsg;
    simtime_t stopTime;

    /** Utility: sends a request to the server */
    virtual void sendRequest();

    /** Utility: cancel msgTimer and if d is smaller than stopTime, then schedule it to d,
     * otherwise delete msgTimer */
    virtual void rescheduleOrDeleteTimer(simtime_t d, short int msgKind);

public:
    TCPBasicVideoStreamCliApp();
    virtual ~TCPBasicVideoStreamCliApp();

protected:
    /** Redefined . */
    virtual void initialize(int stage);

    /** Redefined. */
    virtual void handleTimer(cMessage *msg);

    /** Redefined. */
    virtual void socketEstablished(int connId, void *yourPtr);

    /** Redefined. */
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg,
            bool urgent);

    /** Redefined to start another session after a delay (currently not used). */
    virtual void socketClosed(int connId, void *yourPtr);

    /** Redefined to reconnect after a delay. */
    virtual void socketFailure(int connId, void *yourPtr, int code);

    virtual void refreshDisplay() const override;

};
} //namespace inet
#endif




