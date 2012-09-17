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
package inet.applications.voip;

import inet.applications.IUDPApp;

//
//FIXME Translate it!!!
//
// SimpleVoIPSender
// A destAddress cimre SimpleVoipPacket-eket kuld UDP protokollon keresztul.
// A talk/silence hossza es aranya a shapeTalk, scaleTalk es shapeSil, scaleSil
// parameterektol fugg.
// Csak 'talk' allapotban kuld csomagokat, a csomagokba beleteszi, hogy
// osszesen hany frame-bol all a talk, igy a SimpleVoIPReceiver tudja detektalni az utolso
// csomagok elveszeset is (csak egy teljes talk tud eszrevetlenul elveszni).
//
simple SimpleVoIPSender like IUDPApp
{
    parameters:
        int localPort = default(-1);
        int destPort;
        string destAddress;
        int talkPacketSize @unit(B) = default(40B);     // size of talk packets in bytes
        volatile double talkspurtDuration @unit(s) = default(weibull(1.423s, 0.824s));
        volatile double silenceDuration @unit(s) = default(weibull(0.899s, 1.089s));
        double packetizationInterval @unit(s) = default(20ms);   // timelength of samplepackets
        double startTime @unit(s) = default(0);   // time of start sending
        double stopTime @unit(s) = default(0);  // time of finish sending, 0 means forever

        @display("i=block/source");
    gates:
        output udpOut;
        input udpIn;
}
