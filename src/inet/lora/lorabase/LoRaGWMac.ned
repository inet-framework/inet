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

import inet.linklayer.base.MacProtocolBase;
import inet.linklayer.contract.IMacProtocol;

//
// TODO auto-generated module
//
simple LoRaGWMac extends MacProtocolBase like IMacProtocol
{
    parameters:
        string radioModule = default("^.radio"); // The path to the Radio module  //FIXME remove default value
        string address = default("auto"); 
        string queueModule = default(""); // name of optional external queue module
        int maxQueueSize = default(-1); // maximum queue length in frames; only used if queueModule==""
        bool prioritizeByUP = default(false); // use priority queueing, based on IEEE 802.1d User Priority (UP)
        bool useAck = default(true);
        int headerLength @unit(B) = default(8B);
        int ackLength @unit(B) = default(headerLength);
        double sifsTime @unit(s) = default(10us);
        double slotTime @unit(s) = default(20us);
        double difsTime @unit(s) = default(sifsTime + 2 * slotTime);
        int mtu = default(1500);
        int cwMin = default(31); // minimum contention window
        int cwMax = default(1023); // maximum contention window
        int cwMulticast = default(cwMin); // multicast contention window
        int retryLimit = default(7); // maximum number of retries
        @class(LoRaGWMac);
        
    gates:
    	input upperMgmtIn;
    	output upperMgmtOut;
}
