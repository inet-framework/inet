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
import inet.physicallayer.contract.packetlevel.IMediumVisualizer;
import inet.physicallayer.contract.packetlevel.INeighborCache;
import inet.physicallayer.contract.packetlevel.IObstacleLoss;
import inet.physicallayer.contract.packetlevel.IPathLoss;
import inet.physicallayer.contract.packetlevel.IPropagation;
import inet.physicallayer.contract.packetlevel.IRadioBackgroundNoise;
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
        // submodule type parameters
        string propagationType;                   // NED type of the propagation model
        string pathLossType;                      // NED type of the path loss model
        string obstacleLossType = default("");    // NED type of the obstacle loss model
        string analogModelType;                   // NED type of the analog model
        string backgroundNoiseType = default(""); // NED type of the background noise model
        string mediumLimitCacheType = default("MediumLimitCache"); // NED type of the medium limit cache model
        string neighborCacheType = default("");   // NED type of the neighbor cache model
        string communicationCacheType = default("VectorCommunicationCache"); // NED type of the communication cache model
        string mediumVisualizerType = default("MediumVisualizer"); // NED type of the medium visualizer

        // filter parameters that control when radio frames are sent to receiver radios
        string rangeFilter = default("");       // when either set to 'interferenceRange' or 'communicationRange' the radio medium doesn't send radio frames to a radio if it's outside the provided range
        bool radioModeFilter = default(false);  // when enabled the radio medium doesn't send radio frames to a radio if it's neither in receiver nor in transceiver mode
        bool listeningFilter = default(false);  // when enabled the radio medium doesn't send radio frames to a radio if it listens on the channel in incompatible mode (e.g. different carrier frequency and bandwidth, different modulation)
        bool macAddressFilter = default(false); // when enabled the radio medium doesn't send radio frames to a radio if it the destination mac address differs

        // logging parameters
        bool recordCommunicationLog = default(false); // when enabled the medium writes a communication log file that contains one line per transmission and per reception

        @class(RadioMedium);
        @display("i=misc/sun");

    submodules:
        propagation: <propagationType> like IPropagation {
            parameters:
                @display("p=100,50");
        }
        analogModel: <analogModelType> like IAnalogModel {
            parameters:
                @display("p=100,150");
        }
        backgroundNoise: <backgroundNoiseType> like IRadioBackgroundNoise if backgroundNoiseType != "" {
            parameters:
                @display("p=100,250");
        }
        pathLoss: <pathLossType> like IPathLoss {
            parameters:
                @display("p=300,50");
        }
        obstacleLoss: <obstacleLossType> like IObstacleLoss if obstacleLossType != "" {
            parameters:
                @display("p=300,150");
        }
        mediumVisualizer: <mediumVisualizerType> like IMediumVisualizer if mediumVisualizerType != "" {
            parameters:
                @display("p=300,250");
        }
        mediumLimitCache: <mediumLimitCacheType> like IMediumLimitCache {
            parameters:
                @display("p=500,50");
        }
        communicationCache: <communicationCacheType> like ICommunicationCache {
            parameters:
                @display("p=500,250");
        }
        neighborCache: <neighborCacheType> like INeighborCache if neighborCacheType != "" {
            parameters:
                @display("p=500,150");
        }
}
