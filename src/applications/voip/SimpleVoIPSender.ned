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
// VoIPSender
// A destAddress cimre VoipPacket-eket kuld UDP protokollon keresztul.
// (VoipPacket and VoIPPacket are differents.)
// A talk/silence hossza es aranya a shapeTalk, scaleTalk es shapeSil, scaleSil
// parameterektol fugg.
// Csak 'talk' allapotban kuld csomagokat, a csomagokba beleteszi, hogy
// osszesen hany frame-bol all a talk, igy a VoIPReceiver tudja detektalni az utolso
// csomagok elveszeset is (csak egy teljes talk tud eszrevetlenul elveszni).
// 
//
simple SimpleVoIPSender like IUDPApp
{
    parameters:
        int localPort = default(-1);
        int destPort;
        string destAddress;
        int talkPacketSize @unit(B) = default(40B);     // byte size of talk frames
        // FIXME: delete shape and scale parameters and use volatile random variables for talk and silence duration
        // FIXME: for the god's sake is it worth abbreviating silence as sil?
        volatile double talkDuration @unit(s) = default(weibull(1.423s, 0.824s));
        volatile double silenceDuration @unit(s) = default(weibull(0.899s, 1.089s));
        // FIXME: the standard terminology is packetization interval, it would be better
        double packetizationInterval @unit(s) = default(20ms);   // timelength of samplepackets
        double startTime @unit(s) = default(0);
        double stopTime @unit(s) = default(0);  // time of finish sending, 0 means forever

        @display("i=block/source");
    gates:
        output udpOut;
        input udpIn;
}
