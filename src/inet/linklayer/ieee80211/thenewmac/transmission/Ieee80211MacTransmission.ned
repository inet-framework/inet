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
// Block Transmission
// IEEE Std 802.11-2012 (p. 2418)
//
// This block does octet-level transfers of MPDUs
// from the MAC to the PHY transmitter, generating
// FCS fields and inserting timestamp values where
// necessary. Process Data_Pump begins transmitting
// when TxRequest arrives. The sender of TxRequest
// is assumed to have done the appropriate actions
// prior to transimtting onto the WM. If these actions
// include performing random backoff or invoking the
// "backoff procedure" (see 9.2.5.2), a Backoff signal
// is sent to process Backoff.
// At the completion of each backoff, a BkDone signal
// is returned to the sender of the Backoff signal at
// the correct time to send a TxRequest. The medium
// state updates (busy, idle, slot) from Channel_State
// are forwarded to Backoff_Procedure via Data_Pump
// to prevent decrementing the backoff count while
// transmitting Cts or Ack frames. This block is used
// in both station and AP.
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
