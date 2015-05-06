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

package inet.linklayer.ieee80211.thenewmac.mlme;

//
// Includes scan, join, beacon/dwell and awake/doze timing,
// (re/dis)associate, (de)authenticate, start IBSS, and
// monitor of station & power save state.
//
module Ieee80211MacMlme
{
    gates:
        input psFromRx;
        output psToRx;
        inout mmTxToMtx;
        inout mmTxPsm;
        inout mmgtReq; // TODO
        output mmgtInd; // TODO
        inout mctlToMct;
        input mctlToMctRxCoordination;
        inout mctlSst;

    submodules:
        mlmeStaService: Ieee80211MacMlmeStaServices;
        powerSaveMonitor: Ieee80211MacPowerSaveMonitor;

	connections:
	    // Outer links
	    mlmeStaService.toRx --> psToRx;
	    powerSaveMonitor.fromRx <-- psFromRx;
        mlmeStaService.mopReq <--> mmgtReq;
        mlmeStaService.mopInd --> mmgtInd;
        mlmeStaService.toMct <--> mctlToMct;
        mlmeStaService.toMctRxCoordination <-- mctlToMctRxCoordination;
        powerSaveMonitor.sst <--> mctlSst;

	    // Inner links
	    mlmeStaService.toMtx <--> mmTxToMtx;
	    powerSaveMonitor.psm <--> mmTxPsm;
}
