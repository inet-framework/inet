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

package inet.lora.lorabase;
import inet.applications.contract.IApp;

simple NetworkServerApp like IApp
{
    @class(inet::NetworkServerApp);
    @signal[LoRa_ServerPacketReceived](type=bool); // optional
    @statistic[LoRa_ServerPacketReceived](source=LoRa_ServerPacketReceived; record=count);
    int localPort = default(-1);  // local port (-1: use ephemeral port)
    string localAddress = default("");
    int destPort = default(-1);
    bool evaluateADRinServer = default(false);
    int headerLength @unit(B) = default(8B);
	
	string adrMethod = default("max");
	double adrDeviceMargin = default(15);
	
    gates:
    output udpOut;
    input udpIn;

}
