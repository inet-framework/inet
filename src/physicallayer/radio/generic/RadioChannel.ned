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

package inet.physicallayer.radio.generic;

import inet.physicallayer.radio.contract.IRadioBackgroundNoise;
import inet.physicallayer.radio.contract.IRadioChannel;
import inet.physicallayer.radio.contract.IRadioSignalAttenuation;
import inet.physicallayer.radio.contract.IRadioSignalPropagation;

module RadioChannel like IRadioChannel
{
    parameters:
        // submodule type parameters
        string propagationType;
        string attenuationType;
        string backgroundNoiseType;

        // channel limit parameters
        double alpha = default(2); // TODO: rename to maxRangeAlpha
        double carrierFrequency @unit(Hz) = default(0Hz/0); // TODO: rename to maxRangeCarrierFrequency
        double maxTransmissionPower @unit(W) = default(0W/0);
        double minInterferencePower @unit(dBm) = default(0dBm/0); // TODO: @unit(W) + dBm/dBW <--> W
        double minReceptionPower @unit(dBm) = default(0dBm/0); // TODO: @unit(W) + dBm/dBW <--> W
        double maxAntennaGain @unit(dB) = default(0dB);
        double minInterferenceTime @unit(s) = default(1ps);
        double maxTransmissionDuration @unit(s) = default(10ms);
        double maxCommunicationRange @unit(m) = default(0m/0);
        double maxInterferenceRange @unit(m) = default(0m/0);

        // filter parameters that control when radio frames are sent to receiver radios
        string rangeFilter = default(""); // when either set to 'interferenceRange' or 'communicationRange' the radio channel doesn't send radio frames to a radio if it's outside the provided range
        bool radioModeFilter = default(false); // when enabled the radio channel doesn't send radio frames to a radio if it's neither in receiver nor in transceiver mode 
        bool listeningFilter = default(false); // when enabled the radio channel doesn't send radio frames to a radio if it listens on the channel in incompatible mode (e.g. different carrier frequency and bandwidth, different modulation)
        bool macAddressFilter = default(false); // when enabled the radio channel doesn't send radio frames to a radio if it the destination mac address differs 

        // logging parameters
        bool recordCommunication = default(false);

        @class(RadioChannel);

    submodules:
        propagation: <propagationType> like IRadioSignalPropagation if propagationType != "";
        attenuation: <attenuationType> like IRadioSignalAttenuation if attenuationType != "";
        backgroundNoise: <backgroundNoiseType> like IRadioBackgroundNoise if backgroundNoiseType != "";
}
