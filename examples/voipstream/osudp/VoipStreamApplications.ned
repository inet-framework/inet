//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
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
