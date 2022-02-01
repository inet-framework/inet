//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.medium;

import inet.physicallayer.wireless.common.contract.packetlevel.IAnalogModel;
import inet.physicallayer.wireless.common.contract.packetlevel.ICommunicationCache;
import inet.physicallayer.wireless.common.contract.packetlevel.IMediumLimitCache;
import inet.physicallayer.wireless.common.contract.packetlevel.INeighborCache;
import inet.physicallayer.wireless.common.contract.packetlevel.IObstacleLoss;
import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;
import inet.physicallayer.wireless.common.contract.packetlevel.IPropagation;
import inet.physicallayer.wireless.common.contract.packetlevel.IBackgroundNoise;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;

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
        string signalAnalogRepresentation @enum("unitDisk","scalar","dimensional") = default("scalar");

        // filter parameters that control when signals are sent to receiver radios
        string rangeFilter = default("");       // when either set to 'interferenceRange' or 'communicationRange' the radio medium doesn't send signals to a radio if it's outside the provided range
        bool radioModeFilter = default(false);  // when enabled the radio medium doesn't send signals to a radio if it's neither in receiver nor in transceiver mode
        bool listeningFilter = default(false);  // when enabled the radio medium doesn't send signals to a radio if it listens on the channel in incompatible mode (e.g. different carrier frequency and bandwidth, different modulation)
        bool macAddressFilter = default(false); // when enabled the radio medium doesn't send signals to a radio if it the destination mac address differs

        // logging parameters
        bool recordTransmissionLog = default(false); // when enabled the medium writes one line per transmission into the communication log file
        bool recordReceptionLog = default(false);    // when enabled the medium writes one line per reception into the communication log file

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

