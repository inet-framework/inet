//
// Copyright (C) 2015 OpenSim Ltd.
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

package inet.linklayer.ieee80211.thenewmac.transmission;

//
// Block Transmission (p. 2418)
//
// Includes backoff FCS generate, and timestamp insert.
//
module Ieee80211MacTransmission
{
    gates:
        input csFromCs;
        inout txTxrq[2];
        inout txBkof;
    submodules:
        dataPump: Ieee80211MacDataPump {
            @display("p=52,50");
        }
        backoff: Ieee80211MacBackoff {
            @display("p=199,50");
        }

    connections:
        csFromCs --> dataPump.fromCs;
        txTxrq[0] <--> dataPump.txRq[0];
        txTxrq[1] <--> dataPump.txRq[1];
        txBkof <--> backoff.bkof;
        dataPump.fwdCs --> backoff.fwdCs;
}
