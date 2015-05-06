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

package inet.linklayer.ieee80211.thenewmac.protocolcontrol;

simple Ieee80211MacRxCoordination
{
    gates:
        inout trsp; // From DataPump: [TxConfirm], to DataPump: [TxRequest]
        output txRx; // From Tx_Coordination_sta: [Ack, Cts, Cfend, Cfpoll, TxCfAck]
        output rdat;
        input rxI[2];
        inout bcmgt;
        output bcmgtMlme;
}
