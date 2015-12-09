//
// Copyright (C) 2015 Andras Varga
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//
// Author: Andras Varga
//

package inet.linklayer.ieee80211.mac;

//
// UpperMac for DCF mode.
//
simple DcfUpperMac like IUpperMac
{
    parameters:
        string macModule;
        string rxModule;
        string txModule;
        string firstContentionModule;
        string rateSelectionModule;
        string rateControlModule;
        string statisticsModule;

        int rtsThreshold @unit("B") = default(2346B); // longer frames will be sent using RTS/CTS
        int shortRetryLimit = default(-1); // maximum number of retries per frame, -1 means default
        int longRetryLimit = default(-1); // maximum number of retries for frames sent via RTS/CTS, -1 means default
        bool prioritizeMulticast = default(false); // if true, prioritize multicast frames (9.3.2.1 Fundamental access)
        int maxQueueSize = default(14); // max queue length in frames
        bool useFullAckTimeout = default(false); // when true wait for the full ack frame to arrive instead of waiting for RX start
        double sifsTime @unit("s") = default(-1s); // SIFS time, -1 means default (mode dependent)
        double difsTime @unit("s") = default(-1s); // DIFS time, -1 means default (mode dependent)
        double slotTime @unit("s") = default(-1s); // slot time, -1 means default (mode dependent)
        int cwMin = default(-1); // minimum contention window, -1 means default
        int cwMax = default(-1); // maximum contention window, -1 means default
        int cwMulticast = default(-1); // contention window for multicast frames, -1 means default
        string fragmenterClass = default("BasicFragmentation");
        string reassemblyClass = default("BasicReassembly");
        @display("i=block/layer");
}
