//
// Copyright (C) Opensim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
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
//# Az ~Rtp az appIn gate-en a következő üzeneteket fogadja az application-tól:
//# olyan msg-t, ami ~RtpControlInfo leszármazott controlinfo-t tartalmaz.
//#   - ~RtpCiEnterSession: Enter new session
//#         Create and initialize an ~RtpProfile module for the session.
//#         params: profileName, commonName, bandwidth, destinationAddress, port
//#   - ~RtpCiCreateSenderModule:
//#         Küld a profile-nak egy msg-t, hogy hozzon létre új modult a fileName fájl küldésére.
//#         params: ssrc, payloadType, fileName
//#   - ~RtpCiDeleteSenderModule:
//#         Küld a profile-nak egy msg-t, hogy törölje a küldő modult.
//#         params: ssrc
//#   - ~RtpCiSenderControl:
//#         Küld a profile-nak egy msg-t, hogy a modul hajtsa végre a <command> parancsot.
//#         params: ssrc, command (see ~RtpSenderControlMessageCommands), parameter1, parameter2
//#   - ~RtpCiLeaveSession: Leave session
//#         Delete ~RtpProfile module.
//#         params: ---
//#   -
//# Az ~Rtp az appOut gate-en a következő üzeneteket küldi az application-nek:
//# Olyan ~RtpControlMsg msg-t, ami ~RtpControlInfo leszármazottat tartalmaz:
//#   - ~RtpCiSenderModuleCreated:
//#         Elküldi az App részére a létrehozott modulhoz tartozó ssrc-t
//#         params: ssrc
//#   - ~RtpCiSenderModuleDeleted:
//#         Értesíti az App-ot, hogy a modul törölve lett.
//#         params: ssrc
//#   - ~RtpCiSenderStatus:
//#         Elküldi az App részére a modul állapotát. (PLAYING, FINISHED, STOPPED, PAUSED)
//#         params: ssrc, status (see ~RtpSenderStatus), timeStamp
//#   - ~RtpCiSessionEntered:
//#         Értesíti az App-ot, hogy létrehozott egy új session-t (~RtpProfile-t).
//#         params: ssrc
//#   - ~RtpCiSessionLeft:
//#         Értesíti az App-ot, hogy véget lett a session-nek és törölte az ~RtpProfile-t.
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

