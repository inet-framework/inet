//
// Copyright (C) 2016 OpenSim Ltd.
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

package inet.linklayer.csmaca;

import inet.linklayer.base.MACProtocolBase;
import inet.linklayer.contract.IMACProtocol;

simple CsmaCaMac extends MACProtocolBase like IMACProtocol
{
    parameters:
        string radioModule = default("^.radio"); // The path to the Radio module  //FIXME remove default value
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
        string queueModule = default(""); // name of optional external queue module
        int maxQueueSize = default(10); // maximum queue length in frames; only used if queueModule==""
        bool useAck = default(true);
        double bitrate @unit(bps);
        int headerLength @unit(B) = default(8B);
        int ackLength @unit(B) = default(headerLength);
        double sifsTime @unit(s) = default(10us);
        double slotTime @unit(s) = default(20us);
        double difsTime @unit(s) = default(sifsTime + 2 * slotTime);
        double ackTimeout @unit(s) = default(dropUnit(ackLength) / dropUnit(bitrate) * 1s + sifsTime + slotTime); 
        int mtu = default(1500);
        int cwMin = default(31); // minimum contention window
        int cwMax = default(1023); // maximum contention window
        int cwMulticast = default(cwMin); // multicast contention window
        int retryLimit = default(7); // maximum number of retries
        @class(CsmaCaMac);
        @signal[NF_LINK_BREAK](type=CsmaCaMacFrame);
}
