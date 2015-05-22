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

package inet.linklayer.ieee80211.thenewmac;

import inet.linklayer.ieee80211.thenewmac.dataservice.Ieee80211MacDataService;
import inet.linklayer.ieee80211.thenewmac.macmib.Ieee80211MacMacmib;
import inet.linklayer.ieee80211.thenewmac.macsorts.Ieee80211MacMacsorts;
import inet.linklayer.ieee80211.thenewmac.managementservice.Ieee80211MacManagementService;
import inet.linklayer.ieee80211.thenewmac.mlme.Ieee80211MacMlme;
import inet.linklayer.ieee80211.thenewmac.mpdugeneration.Ieee80211MacMpduGeneration;
import inet.linklayer.ieee80211.thenewmac.protocolcontrol.Ieee80211MacProtocolControl;
import inet.linklayer.ieee80211.thenewmac.reception.Ieee80211MacReception;
import inet.linklayer.ieee80211.thenewmac.transmission.Ieee80211MacTransmission;


module Ieee80211NewMac
{
    @display("bgb=450,417");
    submodules:
        dataService: Ieee80211MacDataService {
            @display("p=64,58");
        }
        managementService: Ieee80211MacManagementService {
            @display("p=277,58");
        }
        transmission: Ieee80211MacTransmission {
            @display("p=64,351");
        }
        protocolControl: Ieee80211MacProtocolControl {
            @display("p=64,238");
        }
        mlme: Ieee80211MacMlme {
            @display("p=277,238");
        }
        mpduGeneration: Ieee80211MacMpduGeneration {
            @display("p=171,127");
        }
        reception: Ieee80211MacReception {
            @display("p=277,351");
        }
        macsorts: Ieee80211MacMacsorts {
            @display("p=410,58");
        }
        macmib: Ieee80211MacMacmib {
            @display("p=410,140");
        }
    connections:
        transmission.csFromCs <-- reception.csToTx;
        transmission.txTxrq[0] <--> protocolControl.txTxO[0]; // DataPump <-->  Tx_Coordination_sta
        transmission.txBkof <--> protocolControl.txTxO[1]; // Backoff_Procedur <--> Tx_Coordination_sta
        transmission.txTxrq[1] <--> protocolControl.txTrsp; // DataPump <--> Rx_Coordination
        reception.psFromSync <-- mlme.psToRx;
        reception.psToPs --> mlme.psFromRx;
        mlme.mmTxPsm <--> mpduGeneration.mmTxPwrMgt;
        mlme.mmTxToMtx <--> mpduGeneration.mmTxMmpdu;
        mpduGeneration.tpdu <--> protocolControl.tpdu;
        mpduGeneration.tsdu <--> dataService.tsdu;
        dataService.rsdu <-- protocolControl.rsdu;
        mlme.mmgtReq <--> managementService.mmgtToMgt;
        managementService.mmgtFromMgt <-- mlme.mmgtInd;
        protocolControl.rxRctl --> reception.rxFromCtl;
        protocolControl.rxRxI[0] <-- reception.rxIndAck;
        protocolControl.rxRxI[1] <-- reception.rxToRx;
        mlme.mctlToMct <--> protocolControl.mctlTmgt;
        mlme.mctlSst <--> protocolControl.mctlBcMgt;
        mlme.mctlToMctRxCoordination <-- protocolControl.mctlBcmgtMlme;
}
