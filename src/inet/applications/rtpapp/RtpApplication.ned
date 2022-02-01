//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.rtpapp;

//
// An application which uses RTP. It acts as a sender if the parameter fileName is
// set, and as a receiver if the parameter is empty.
//
simple RtpApplication
{
    parameters:
        @display("i=block/app");
        string commonName;  // the common name (CNAME) of this host
        string profileName;  // which rtp profile is to be used (usually RtpAvProfile)
        double bandwidth;  // bandwidth in bytes per second for this session
        string destinationAddress;  // destination address (the L3AddressResolver class is used to resolve the address)
        int portNumber;  // used port
        string fileName;    // fileName of file to be transmitted, "" means this system acts only as a receiver
        int payloadType;  // payload type of file to transmit
        double sessionEnterDelay @unit(s);  // delay before starts a new session
        double transmissionStartDelay @unit(s); // delay before start transmission (related to "sender module created")
        double transmissionStopDelay @unit(s);  // delay before stops transmission (related to "start transmission")
        double sessionLeaveDelay @unit(s); // delay before leave session (related to "file transmission finished/stopped" or "session entered" when fileName is "")
        @selfMessageKinds(inet::RtpAppSelfMsgKinds);    // kind used in self messages

    gates:
        output rtpOut @labels(RtpInterfacePacket/down);
        input rtpIn @labels(RtpInterfacePacket/up);
}

