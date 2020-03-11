//
// Copyright (C) 2018 OpenSim Ltd.
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
// Author: Zoltan Bojthe
//

package inet.physicallayer.ethernet;

simple EtherPhy
{
    parameters:
        @display("i=block/rxtx");
        //double bitrate @unit(bps);    // get from NIC module
        //bool   duplexMode;    // get from NIC module
        bool   sendRawBytes = default(false); // when true packets are serialized into a sequence of bytes before sending out
        string displayStringTextFormat = default("rate: %b\nsent: %s, rcvd: %r\nqueue: %q, drop: %d");
        @signal[txStateChanged](type=long); // enum=MacTxState
        @signal[rxStateChanged](type=long); // enum=MacTransmitState
        @signal[txFinished](type=long);
        @signal[txAborted](type=long);
        @statistic[rxState](title="RX state"; source=rxStateChanged; record=vector; interpolationmode=sample-hold);
        @statistic[txState](title="TX state"; source=txStateChanged; record=vector; interpolationmode=sample-hold);
    gates:
        input upperLayerIn @labels(EtherFrame);
        output upperLayerOut @labels(EtherFrame);
        inout phys @labels(EthernetSignal);
}

