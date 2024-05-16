//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.rtp;

//
// The Rtp module is the center of the Rtp layer of an endsystem.
// It communicates with the application, and sends and receives Rtp data
// packets.
//
// Much of the work is done by dynamically created Profile, and Payload
// Sender and Receiver modules (the latter two are created by Profile).
//
// See ~RtpProfile, ~RtpAvProfile
//
//# TODO Translate:
//# The Rtp module receives the following messages from the application through the `appIn` gate:
//# a msg containing a ~RtpControlInfo derived controlinfo.
//#   - ~RtpCiEnterSession: Enter new session
//#         Create and initialize an ~RtpProfile module for the session.
//#         params: `profileName`, `commonName`, bandwidth, `destinationAddress`, port
//#   - ~RtpCiCreateSenderModule:
//#         Send a msg to the profile to create a new module for sending the `fileName` file.
//#         params: ssrc, `payloadType`, `fileName`
//#   - ~RtpCiDeleteSenderModule:
//#         Send a msg to the profile to delete the sender module.
//#         params: ssrc
//#   - ~RtpCiSenderControl:
//#         Send a msg to the profile to execute the <command> command.
//#         params: ssrc, command (see ~RtpSenderControlMessageCommands), parameter1, parameter2
//#   - ~RtpCiLeaveSession: Leave session
//#         Delete ~RtpProfile module.
//#         params: ---
//#   -
//# The Rtp module sends the following messages to the application through the `appOut` gate:
//# A ~RtpControlMsg msg containing a ~RtpControlInfo derived:
//#   - ~RtpCiSenderModuleCreated:
//#         Sends the ssrc associated with the created module to the App.
//#         params: ssrc
//#   - ~RtpCiSenderModuleDeleted:
//#         Notifies the App that the module has been deleted.
//#         params: ssrc
//#   - ~RtpCiSenderStatus:
//#         Sends the module status to the App. (PLAYING, FINISHED, STOPPED, PAUSED)
//#         params: ssrc, status (see ~RtpSenderStatus), `timeStamp`
//#   - ~RtpCiSessionEntered:
//#         Notifies the App that a new session (~RtpProfile) has been created.
//#         params: ssrc
//#   - ~RtpCiSessionLeft:
//#         Notifies the App that the session has ended and deletes the ~RtpProfile.
//#         params: ---
simple Rtp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        @display("i=block/buffer");
        @signal[packetReceived](type=cPacket);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,vector; interpolationmode=none);

    gates:
        input appIn @labels(RtpInterfacePacket/down);
        input profileIn @labels(RtpInnerPacket);
        input rtcpIn @labels(RtpInnerPacket);
        input udpIn @labels(UdpControlInfo/up);
        output appOut @labels(RtpInterfacePacket/up);
        output profileOut @labels(RtpInnerPacket);
        output rtcpOut @labels(RtpInnerPacket);
        output udpOut @labels(UdpControlInfo/down);
}

