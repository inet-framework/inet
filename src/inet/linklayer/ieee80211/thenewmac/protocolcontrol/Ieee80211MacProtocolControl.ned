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

//
// This block performs the DCF functions, as well as
// CF-responder functions if the station is CF-pollable.
// Tx_Coordination includes RTS and ATIM generation.
// Rx_Coordination generates acknowledgments, routes
// data frames to MAC data service and management
// frames to MLME, and indicates receipt of Ack,
// Cts, and CF-Poll frames to Tx_Coordination.
//
module Ieee80211MacProtocolControl
{
    gates:
        inout txTxO[2];
        inout txTrsp;
        inout tpdu;
        output rsdu;
        output rxRctl;
        input rxRxI[2];
        inout mctlTmgt;
        inout mctlBcMgt;
        output mctlBcmgtMlme;

    submodules:
        txCoordinationSta: Ieee80211MacTxCoordinationSta {
            @display("p=195,59");
        }
        rxCoordination: Ieee80211MacRxCoordination {
            @display("p=63,59");
        }

    connections:
        // Outer links
        txCoordinationSta.txODataPump <--> txTxO[0];
        txCoordinationSta.txOBackoff <--> txTxO[1];
        rxCoordination.trsp <--> txTrsp;
        txCoordinationSta.tdat <--> tpdu;
        rxCoordination.rdat --> rsdu;
        txCoordinationSta.rctl --> rxRctl;
        rxCoordination.rxI[0] <-- rxRxI[0];
        rxCoordination.rxI[1] <-- rxRxI[1];
        rxCoordination.bcmgt <--> mctlBcMgt;
        rxCoordination.bcmgtMlme --> mctlBcmgtMlme;
        txCoordinationSta.tmgt <--> mctlTmgt;

        // Inner links
        txCoordinationSta.txRx <-- rxCoordination.txRx;
}
