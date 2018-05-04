package inet.applications.tcpapp;
import inet.applications.contract.ITCPApp;

//
//  TCPVideoStreamCliApp
//
//  It's an adaptation of the code of Navarro Joaquim (https://github.com/navarrojoaquin/adaptive-video-tcp-omnet).
//  Created on 8 de dez de 2017 by Anderson Andrei da Silva & Patrick Menani Abrahão at University of São Paulo
//
//
// Client for video streaming over TCP.
//

simple TCPBasicVideoStreamCliApp like ITCPApp {
    
    parameters:
        @class(TCPBasicVideoStreamCliApp);
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
        
        // Video parameters
        string 			video_packet_size_per_second @units(kbps)= default("1000 1500 2000 4000 8000 12000"); // how many kbits are required for 1 second of video for each representation (quality levels)
        int             video_current_quality_index = default(0); //wich of the qualitys above that like to test
        int 			video_buffer_max_length @units(s) = default(10); // buffer max length in seconds
        int 			video_duration @units(s) = default(30); // video length in seconds
        int 			manifest_size @units(byte) = default(100000);
        
        // General statistics
        @display("i=block/app");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @signal[connect](type=long);  // 1 for open, -1 for close
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
        @statistic[numActiveSessions](title="number of active sessions"; source="sum(connect)"; record=max,timeavg,vector; interpolationmode=sample-hold);
        @statistic[numSessions](title="total number of sessions"; source="sum(connect+1)/2"; record=last);

        
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/down);
}