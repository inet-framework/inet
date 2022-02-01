//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.examples.voipstream.osudp;

import inet.applications.voipstream.VoipStreamReceiver;
import inet.applications.voipstream.VoipStreamSender;
import inet.emulation.transportlayer.udp.ExtLowerUdp;

network VoipStreamSenderApplication
{
    parameters:
        @networkNode;
    submodules:
        app: VoipStreamSender;
        udp: ExtLowerUdp;
    connections:
        app.socketOut --> udp.appIn;
        app.socketIn <-- udp.appOut;
}

network VoipStreamReceiverApplication
{
    parameters:
        @networkNode;
    submodules:
        app: VoipStreamReceiver;
        udp: ExtLowerUdp;
    connections:
        app.socketOut --> udp.appIn;
        app.socketIn <-- udp.appOut;
}
