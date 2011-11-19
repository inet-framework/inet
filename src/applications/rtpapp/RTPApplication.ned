package inet.applications.rtpapp;

//
// The RTPApplication is just a very simple sample for an application
// which uses RTP. It acts as a sender if the parameter fileName is
// set, and as a receiver if the parameter is empty.
//
simple RTPApplication
{
    parameters:
        @display("i=block/app");
        string commonName;  // the common name (CNAME) of this host
        string profileName;  // which rtp profile is to be used (usually RTPAVProfile)
        double bandwidth;  // bandwidth in bytes per second for this session 
        string destinationAddress;  // destination address (the IPvXAddressResolver class is used to resolve the address)
        int portNumber;  // used port
        string fileName;    // fileName of file to be transmitted, "" means this system acts only as a receiver
        int payloadType;  // payload type of file to transmit
        double sessionEnterDelay @unit(s);  // delay before starts a new session
        double transmissionStartDelay @unit(s); // delay before start transmission (related to "sender module created")
        double transmissionStopDelay @unit(s);  // delay before stops transmission (related to "start transmission")
        double sessionLeaveDelay @unit(s); // delay before leave session (related to "file transmission finished/stopped" or "session entered" when fileName is "")

    gates:
        output rtpOut @labels(RTPInterfacePacket/down);
        input rtpIn @labels(RTPInterfacePacket/up);
}
