package inet.applications.tcpapp;
import inet.applications.contract.IApp;
//
//  TcpVideoStreamCliApp
//
//  It's an adaptation of the code of Navarro Joaquim (https://github.com/navarrojoaquin/adaptive-video-Tcp-omnet).
//  Created on 8 de dez de 2017 by Anderson Andrei da Silva & Patrick Menani Abrahao at University of Sao Paulo
//
//
//
// Client for adaptive video streaming over Tcp. This client requests a server
// 1s of video with different quality level depending on the buffer ocuppation.
// May be used as a rough model of MPEG-DASH.
//
// The model communicates with the server in sessions. During a session,
// the client opens a single Tcp connection to the server, sends several
// requests (always waiting for the complete reply to arrive before
// sending a new request), and closes the connection.
//
// The server app should be ~TcpGenericSrvApp; the model sends ~GenericAppMsg
// messages.
//
// <b>Configuring App</b>
//
// See the parameter list to get an overview of the supported options
//
// The module parameter dataTransferMode should be set the transfer mode in Tcp layer.
// You should set dataTransferMode to "object"
//
//   -# use "object", which transmits
//      cMessage objects (and subclasses) over a Tcp connection. The same
//      message object sequence that was sent by the client to the
//      sender-side Tcp entity will be reproduced on the receiver side.
//      If a client sends a cMessage with length = 1 megabyte, the
//      receiver-side client will receive the same message object (or a clone)
//      after the Tcp entities have completed simulating the transmission
//      of 1 megabyte over the connection. This is a different behaviour
//      from TcpVirtualDataSendQueue/RcvQueue.
//      This mode is not implemented in ~Tcp_NSC yet.
//
//
// @see ~TcpGenericSrvApp, ~GenericAppMsg, ~TelnetApp
//

simple TcpVideoStreamCliApp like IApp {
    
    parameters:
        @class(TcpVideoStreamCliApp);
        // General parameters
        string 			localAddress = default(""); // may be left empty ("")
        int 			localPort = default(-1); // port number to listen on
        string 			connectAddress = default("");  // server address (may be symbolic)
        int 			connectPort = default(1000); // port number to connect to
        string 			dataTransferMode @enum("bytecount","object","bytestream") = default("bytecount");
        volatile double video_startTime @unit(s) = default(1s); // time first session begins
        double          startTime @unit(s) = default(1s); // time first session begins
        double 			stopTime @unit(s) = default(0);  // time of finish sending, 0 means infinity
        volatile double idleInterval @unit(s); // time gap between sessions
        volatile int 	requestLength @unit(B) = default(200B); // length of a request
        volatile double reconnectInterval @unit(s) = default(10s);  // if connection breaks, waits this much before trying to reconnect
        volatile int    numRequestsPerSession = default(1);  // number of requests sent per session
        volatile double thinkTime @unit(s); // time gap between requests
        
        // Adaptive Video parameters
        string 			video_packet_size_per_second @units(kbps)= default("1000 1500 2000 4000 8000 12000"); // how many kbits are required for 1 second of video for each representation (quality levels)
        int 			manifest_size @units(byte) = default(100000);
        int 			video_buffer_max_length @units(s) = default(10); // buffer max length in seconds
        int 			video_duration @units(s) = default(30); // video length in seconds
        
        // General statistics
        @display("i=block/app");
        @signal[packetSent](type=Packet);
        @signal[packetReceived](type=Packet);
        @signal[connect](type=long);  // 1 for open, -1 for close
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(packetReceived)"; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[numActiveSessions](title="number of active sessions"; source="sum(connect)"; record=max,timeavg,vector; interpolationmode=sample-hold);
        @statistic[numSessions](title="total number of sessions"; source="sum(connect+1)/2"; record=last);
        
        // Adaptive Video statistics
        @signal[DASHBufferLength](type="int");
        @statistic[DASHBufferLength](title="Video buffer length (in seconds)"; record=vector);
        @signal[DASHQualityLevel](type="int");
        @statistic[DASHQualityLevel](title="Requested quality level"; record=vector);
        @signal[DASHVideoPlaybackPointer](type="int");
        @statistic[DASHVideoPlaybackPointer](title="Video playback pointer"; record=vector);
        @signal[DASHVideoPlaybackStatus](type="bool");
        @statistic[DASHVideoPlaybackStatus](title="Video playback status (playing=1, buffering=0)"; record=vector);
        
    gates:
        input socketIn @labels(TcpCommand/up);
        output socketOut @labels(TcpCommand/down);
}