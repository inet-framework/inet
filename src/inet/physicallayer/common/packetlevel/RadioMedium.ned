//
// Copyright (C) 2013 OpenSim Ltd.
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

package inet.physicallayer.common.packetlevel;

import inet.physicallayer.contract.packetlevel.IAnalogModel;
import inet.physicallayer.contract.packetlevel.ICommunicationCache;
import inet.physicallayer.contract.packetlevel.IMediumLimitCache;
import inet.physicallayer.contract.packetlevel.INeighborCache;
import inet.physicallayer.contract.packetlevel.IObstacleLoss;
import inet.physicallayer.contract.packetlevel.IPathLoss;
import inet.physicallayer.contract.packetlevel.IPropagation;
import inet.physicallayer.contract.packetlevel.IBackgroundNoise;
import inet.physicallayer.contract.packetlevel.IRadioMedium;

//
// The medium model describes the shared physical medium where communication
// takes place. It keeps track of radios, noise sources, ongoing transmissions,
// background noise, and other ongoing noises. The medium computes when, where
// and how transmissions and noises arrive at receivers. It also efficiently
// provides the set of interfering transmissions and noises for the receivers.
//
module RadioMedium like IRadioMedium
{
    parameters:
        string physicalEnvironmentModule = default("physicalEnvironment"); // module path of the physical environment model
        string signalAnalogRepresentation @enum("unitDisk", "scalar", "dimensional") = default("scalar");

        // filter parameters that control when signals are sent to receiver radios
        string rangeFilter = default("");       // when either set to 'interferenceRange' or 'communicationRange' the radio medium doesn't send signals to a radio if it's outside the provided range
        bool radioModeFilter = default(false);  // when enabled the radio medium doesn't send signals to a radio if it's neither in receiver nor in transceiver mode
        bool listeningFilter = default(false);  // when enabled the radio medium doesn't send signals to a radio if it listens on the channel in incompatible mode (e.g. different carrier frequency and bandwidth, different modulation)
        bool macAddressFilter = default(false); // when enabled the radio medium doesn't send signals to a radio if it the destination mac address differs

        // logging parameters
        bool recordCommunicationLog = default(false); // when enabled the medium writes a communication log file that contains one line per transmission and per reception

        @class(RadioMedium);
        @display("i=misc/sun");

        @signal[radioAdded];
        @signal[radioRemoved];
        @signal[signalAdded];
        @signal[signalRemoved];
        @signal[signalDepartureStarted];
        @signal[signalDepartureEnded];
        @signal[signalArrivalStarted];
        @signal[signalArrivalEnded];

    submodules:
        propagation: <default("ConstantSpeedPropagation")> like IPropagation {
            parameters:
                @display("p=100,100");
        }
        analogModel: <default(signalAnalogRepresentation == "unitDisk" ? "UnitDiskAnalogModel" : signalAnalogRepresentation == "scalar" ? "ScalarAnalogModel" : signalAnalogRepresentation == "dimensional" ? "DimensionalAnalogModel" : "")> like IAnalogModel {
            parameters:
                @display("p=100,200");
        }
        backgroundNoise: <default(signalAnalogRepresentation == "scalar" ? "IsotropicScalarBackgroundNoise" : signalAnalogRepresentation == "dimensional" ? "IsotropicDimensionalBackgroundNoise" : "")> like IBackgroundNoise if typename != "" {
            parameters:
                @display("p=100,300");
        }
        pathLoss: <default("FreeSpacePathLoss")> like IPathLoss {
            parameters:
                @display("p=300,100");
        }
        obstacleLoss: <default("")> like IObstacleLoss if typename != "" {
            parameters:
                @display("p=300,200");
        }
        mediumLimitCache: <default("MediumLimitCache")> like IMediumLimitCache {
            parameters:
                @display("p=500,100");
        }
        communicationCache: <default("VectorCommunicationCache")> like ICommunicationCache {
            parameters:
                @display("p=500,300");
        }
        neighborCache: <default("")> like INeighborCache if typename != "" {
            parameters:
                @display("p=500,200");
        }
}

