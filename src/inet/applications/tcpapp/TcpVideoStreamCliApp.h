/*
 *  TcpVideoStreamCliApp.cc
 *
 *  It's an adaptation of the code of Navarro Joaquim (https://github.com/navarrojoaquin/adaptive-video-tcp-omnet).
 *  Created on 8 de dez de 2017 by Anderson Andrei da Silva & Patrick Menani Abrahão at University of São Paulo
 *
 */

#ifndef TcpVideoSTREAMCLIAPP_H_
#define TcpVideoSTREAMCLIAPP_H_

#include <omnetpp.h>
#include <algorithm>
#include "inet/common/INETDefs.h"
#include "inet/applications/tcpapp/TcpBasicClientApp.h"
#include "inet/common/packet/Packet.h"

namespace inet {

class TcpVideoStreamCliApp: public TcpBasicClientApp {

protected:

    // Adaptive Video (AV) parameters
    std::vector<int> video_packet_size_per_second;
    int video_buffer_max_length;
    int video_duration;
    int numRequestsToSend; // requests to send in this session. Each request = 1s of video
    int video_buffer; // current lenght of the buffer in seconds
    simsignal_t DASH_buffer_length_signal;
    int video_current_quality_index; // requested quality
    simsignal_t DASH_quality_level_signal;
    bool video_is_playing;
    simsignal_t DASH_video_is_playing_signal;
    int video_playback_pointer;
    simsignal_t DASH_playback_pointer;
    bool video_is_buffering = true;
    int video_buffer_min_rebuffering = 3; // if video_buffer < video_buffer_min_rebuffering then a rebuffering event occurs
    int manifest_size;
    bool manifestAlreadySent = false;
    double video_startTime;

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

    // Flags to avoid multiple quality switches when the buffer is at full capacity
    bool can_switch = true;
    int switch_timeout = 5;
    int switch_timer = switch_timeout;
    // Enhanced switch algorithm (estimate available bit rate before switching to higher quality level)
    int packetTimeArrayLength = 5;
    simtime_t packetTime[5];
    int packetTimePointer = 0;
    simtime_t tLastPacketRequested;

    /** Utility: sends a request to the server */
    virtual void sendRequest();

    /** Utility: cancel msgTimer and if d is smaller than stopTime, then schedule it to d,
     * otherwise delete msgTimer */
    virtual void rescheduleOrDeleteTimer(simtime_t d, short int msgKind);

public:
    TcpVideoStreamCliApp();
    virtual ~TcpVideoStreamCliApp();

protected:
    /** Redefined . */
    virtual void initialize(int stage);

    /** Redefined. */
    virtual void handleTimer(cMessage *msg);

    /** Redefined. */
    virtual void socketEstablished(int connId, void *yourPtr);

    /** Redefined. */
    virtual void socketDataArrived(int connId, void *yourPtr, Packet *msg,
            bool urgent);

    /** Redefined to start another session after a delay (currently not used). */
    virtual void socketClosed(int connId, void *yourPtr);

    /** Redefined to reconnect after a delay. */
    virtual void socketFailure(int connId, void *yourPtr, int code);

    virtual void refreshDisplay() const override;

};
} //namespace inet
#endif




