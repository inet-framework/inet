//
// Copyright (C) 2001 Matthias Oppitz <Matthias.Oppitz@gmx.de>
// Copyright (C) 2007 Ahmed Ayadi  <ahmed.ayadi@sophia.inria.fr>
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.applications.rtpapp;

//
// Implements a versatile RTP (Real-time Transport Protocol) application that can
// function as either a sender or receiver based on configuration. When configured
// with a filename, operates as a media sender, transmitting the file contents over
// RTP with precise timing control. Without a filename, functions as a receiver that
// processes incoming RTP streams. Provides comprehensive session management including
// controlled entry, participation, and departure from RTP sessions.
//
// Offers extensive configuration options for RTP session parameters including bandwidth
// management, payload type specification, and timing controls for transmission start/stop
// and session participation. Designed to work with standard RTP profiles (typically
// RtpAvProfile) and supports the fundamental RTP mechanisms for real-time media
// transmission in network simulations.
//
// @see ~RtpProfile, ~RtpAvProfile, ~RtpInterfacePacket
//
simple RtpApplication
{
    parameters:
        @display("i=block/app");
        string commonName;  // The common name (CNAME) of this host
        string profileName;  // Which rtp profile is to be used (usually RtpAvProfile)
        double bandwidth;  // Bandwidth in bytes per second for this session
        string destinationAddress;  // Destination address (the L3AddressResolver class is used to resolve the address)
        int portNumber;  // Used port
        string fileName;    // Filename of file to be transmitted, "" means this system acts only as a receiver
        int payloadType;  // Payload type of file to transmit
        double sessionEnterDelay @unit(s);  // Delay before starting a new session
        double transmissionStartDelay @unit(s); // Delay before starting transmission (related to "sender module created")
        double transmissionStopDelay @unit(s);  // Delay before stopping transmission (related to "start transmission")
        double sessionLeaveDelay @unit(s); // Delay before leaving session (related to "file transmission finished/stopped" or "session entered" when fileName is "")
        @selfMessageKinds(inet::RtpAppSelfMsgKinds);    // Kind used in self messages

    gates:
        output rtpOut @labels(RtpInterfacePacket/down);
        input rtpIn @labels(RtpInterfacePacket/up);
}
