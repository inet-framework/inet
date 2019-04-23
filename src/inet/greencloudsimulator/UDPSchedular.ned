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

package inet.greencloudsimulator;
import inet.applications.udpapp.UdpBasicApp;

//
// TODO auto-generated type
//
simple UDPSchedular extends UdpBasicApp
{
    parameters:
        double scheduleDelay @unit(s) = default(5s); // PacketConversion/DecreasingTimeAlgo/ShortestJobFirst
        double lastInFirstOutDelay @unit(s) = default(1s); // used in FirstComeFirstServe/LastInFirstOut
   // @signal[emed](type="int");
    //@statistic[EConsumed](title="EnergyConsumed"; source="emed"; record=vector,stats; interpolationmode=none);
    @signal[sentPk](type=cPacket);
    @signal[rcvdPk](type=cPacket);
    @signal[arrival](type="int");    
    @class(UDPSchedular);
}
