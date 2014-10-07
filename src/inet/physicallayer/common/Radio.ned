//
// Copyright (C) 2013 OpenSim Ltd
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

package inet.physicallayer.common;

import inet.physicallayer.contract.IRadio;
import inet.physicallayer.contract.IAntenna;
import inet.physicallayer.contract.IReceiver;
import inet.physicallayer.contract.ITransmitter;
import inet.power.contract.IEnergyConsumer;

// switchingTimes parameter should be regarded as a 5x5 matrix:
//
//               OFF SLEEP RECEIVER TRANSMITTER TRANSCEIVER
// OFF           0 0 0 0 0
// SLEEP         0 0 0 0 0
// RECEIVER      0 0 0 0 0
// TRANSMITTER   0 0 0 0 0
// TRANSCEIVER   0 0 0 0 0
//
// The corresponding RadioMode pairs contain the time needed to
// switch from one state to another.
// For example: the 6th number identifies the time needed to switch
// from SLEEP to OFF.
//
// The first entry of the switchingTimes string indicates the
// time-related metric prefix and it MUST be s, ms or ns.
//
module Radio like IRadio
{
    parameters:
        string antennaType;
        string transmitterType;
        string receiverType;
        string energyConsumerType = default("");

        string radioMediumModule = default("radioMedium");
        string energySourceModule = default("");
        string switchingTimes = default("ms 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
        bool displayCommunicationRange = default(true);
        bool displayInterferenceRange = default(true);
        
        *.energySourceModule = default(absPath(energySourceModule));

        @class(Radio);
        @display("i=block/wrxtx");

        @signal[radioModeChanged](type=long);
        @signal[listeningChanged];
        @signal[receptionStateChanged](type=long);
        @signal[transmissionStateChanged](type=long);

        @statistic[radioMode](title="Radio mode"; source=radioModeChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[receptionState](title="Radio reception state"; source=receptionStateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[transmissionState](title="Radio transmission state"; source=receptionStateChanged; record=count,vector; interpolationmode=sample-hold);

    gates:
        input upperLayerIn @labels(ILinkLayerFrame/down);
        output upperLayerOut @labels(ILinkLayerFrame/up);
        input radioIn @labels(IRadioFrame);

    submodules:
        antenna: <antennaType> like IAntenna {
            parameters:
                @display("p=100,50");
        }
        transmitter: <transmitterType> like ITransmitter {
            parameters:
                @display("p=100,150");
        }
        receiver: <receiverType> like IReceiver {
            parameters:
                @display("p=100,250");
        }
        energyConsumer: <energyConsumerType> like IEnergyConsumer if energyConsumerType != "" {
            parameters:
                @display("p=100,350");
        }
}
