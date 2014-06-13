package inet.transportlayer.rtp;

//
// The RTP module is the center of the RTP layer of an endsystem.
// It communicates with the application, and sends and receives RTP data
// packets.
//
// Much of the work is done by dynamically created Profile, and Payload
// Sender and Receiver modules (the latter two are created by Profile).
//
// See ~RTPProfile, ~RTPAVProfile
//
// TODO Translate:
// Az ~RTP az appIn gate-en a következő üzeneteket fogadja az application-tól:
// olyan msg-t, ami ~RTPControlInfo leszármazott controlinfo-t tartalmaz.
//   - ~RTPCIEnterSession: Enter new session
//         Create and initialize an ~RTPProfile module for the session.
//         params: profileName, commonName, bandwidth, destinationAddress, port
//   - ~RTPCICreateSenderModule:
//         Küld a profile-nak egy msg-t, hogy hozzon létre új modult a fileName fájl küldésére.
//         params: ssrc, payloadType, fileName
//   - ~RTPCIDeleteSenderModule:
//         Küld a profile-nak egy msg-t, hogy törölje a küldő modult.
//         params: ssrc
//   - ~RTPCISenderControl:
//         Küld a profile-nak egy msg-t, hogy a modul hajtsa végre a <command> parancsot.
//         params: ssrc, command (see ~RTPSenderControlMessageCommands), parameter1, parameter2
//   - ~RTPCILeaveSession: Leave session
//         Delete ~RTPProfile module.
//         params: ---
//   -
// Az ~RTP az appOut gate-en a következő üzeneteket küldi az application-nek:
// Olyan ~RTPControlMsg msg-t, ami ~RTPControlInfo leszármazottat tartalmaz:
//   - ~RTPCISenderModuleCreated:
//         Elküldi az App részére a létrehozott modulhoz tartozó ssrc-t
//         params: ssrc
//   - ~RTPCISenderModuleDeleted:
//         Értesíti az App-ot, hogy a modul törölve lett.
//         params: ssrc
//   - ~RTPCISenderStatus:
//         Elküldi az App részére a modul állapotát. (PLAYING, FINISHED, STOPPED, PAUSED)
//         params: ssrc, status (see ~RTPSenderStatus), timeStamp
//   - ~RTPCISessionEntered:
//         Értesíti az App-ot, hogy létrehozott egy új session-t (~RTPProfile-t).
//         params: ssrc
//   - ~RTPCISessionLeft:
//         Értesíti az App-ot, hogy véget lett a session-nek és törölte az ~RTPProfile-t.
//         params: ---

simple RTP
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string routingTableModule;
        @display("i=block/buffer");
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);

    gates:
        input appIn @labels(RTPInterfacePacket/down);
        input profileIn @labels(RTPInnerPacket);
        input rtcpIn @labels(RTPInnerPacket);
        input udpIn @labels(UDPControlInfo/up);
        output appOut @labels(RTPInterfacePacket/up);
        output profileOut @labels(RTPInnerPacket);
        output rtcpOut @labels(RTPInnerPacket);
        output udpOut @labels(UDPControlInfo/down);
}
