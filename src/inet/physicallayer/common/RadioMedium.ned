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

package inet.physicallayer.common;

import inet.physicallayer.contract.IAnalogModel;
import inet.physicallayer.contract.INeighborCache;
import inet.physicallayer.contract.IObstacleLoss;
import inet.physicallayer.contract.IParallelStrategy;
import inet.physicallayer.contract.IPathLoss;
import inet.physicallayer.contract.IPropagation;
import inet.physicallayer.contract.IRadioBackgroundNoise;
import inet.physicallayer.contract.IRadioMedium;

module Dummy {}

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
        string neighborCacheType = default("");   // NED type of the neighbor cache model
        string parallelStrategyType = default("");// NED type of the parallel computation strategy

        // channel limit parameters
        double carrierFrequency @unit(Hz) = default(0Hz/0);       // TODO: rename to maxRangeCarrierFrequency
        double maxSpeed @unit(mps) = default(0mps/0);             // maxium speed on the medium, NaN means medium computes using mobility models
        double maxTransmissionPower @unit(W) = default(0W/0);
        double minInterferencePower @unit(dBm) = default(0dBm/0); // TODO: @unit(W) + dBm/dBW <--> W
        double minReceptionPower @unit(dBm) = default(0dBm/0);    // TODO: @unit(W) + dBm/dBW <--> W
        double maxAntennaGain @unit(dB) = default(0dB);           // maximum antenna gain on the medium, NaN means medium computes using antenna models
        double minInterferenceTime @unit(s) = default(1ps);       // minimum time interval to consider two overlapping signals interfering
        double maxTransmissionDuration @unit(s) = default(10ms);  // maximum duration of a transmission on the medium
        double maxCommunicationRange @unit(m) = default(0m/0);    // maximum communication range on the medium, NaN means medium computes using transmitter and receiver models
        double maxInterferenceRange @unit(m) = default(0m/0);     // maximum interference range on the medium, NaN means medium computes using transmitter and receiver models

        // filter parameters that control when radio frames are sent to receiver radios
        string rangeFilter = default("");       // when either set to 'interferenceRange' or 'communicationRange' the radio medium doesn't send radio frames to a radio if it's outside the provided range
        bool radioModeFilter = default(false);  // when enabled the radio medium doesn't send radio frames to a radio if it's neither in receiver nor in transceiver mode 
        bool listeningFilter = default(false);  // when enabled the radio medium doesn't send radio frames to a radio if it listens on the channel in incompatible mode (e.g. different carrier frequency and bandwidth, different modulation)
        bool macAddressFilter = default(false); // when enabled the radio medium doesn't send radio frames to a radio if it the destination mac address differs 

        // logging parameters
        bool recordCommunicationLog = default(false); // when enabled the medium writes a communication log file that contains one line per transmission and per reception

        // graphics parameters
        bool displayCommunication = default(false);         // when enabled the radio medium displays the ongoing transmissions
        bool drawCommunication2D = default(false);          // when enabled the raido medium displays the ongoing transmissions as 2 dimensional rings instead of 3 dimensional spheres 
        bool leaveCommunicationTrail = default(false);      // when enabled the radio medium displays recent successful receptions as straight blue lines from the transmission to the reception
        double updateCanvasInterval @unit(s) = default(0s); // the time interval between canvas updates when there are ongoing transmissions 

        @class(RadioMedium);
        @display("i=misc/sun");

    submodules:
        propagation: <propagationType> like IPropagation {
            parameters:
                @display("p=100,50");
        }
        pathLoss: <pathLossType> like IPathLoss {
            parameters:
                @display("p=100,150");
        }
        obstacleLoss: <obstacleLossType> like IObstacleLoss if obstacleLossType != "" {
            parameters:
                @display("p=100,250");
        }
        analogModel: <analogModelType> like IAnalogModel {
            parameters:
                @display("p=100,350");
        }
        backgroundNoise: <backgroundNoiseType> like IRadioBackgroundNoise if backgroundNoiseType != "" {
            parameters:
                @display("p=100,450");
        }
        neighborCache: <neighborCacheType> like INeighborCache if neighborCacheType != "" {
            parameters:
                @display("p=100,550");
        }
        parallelStrategy: <parallelStrategyType> like IParallelStrategy if parallelStrategyType != "" {
            parameters:
                @display("p=100,650");
        }
        dummy: Dummy if parallelStrategyType == "";
}
