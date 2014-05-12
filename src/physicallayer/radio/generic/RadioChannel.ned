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

import inet.physicallayer.radio.common.IRadioSignalPropagation;
import inet.physicallayer.radio.contract.IRadioBackgroundNoise;
import inet.physicallayer.radio.contract.IRadioChannel;
import inet.physicallayer.radio.contract.IRadioSignalAttenuation;

simple RadioChannel like IRadioChannel
{
    parameters:
        string propagationType;
        string attenuationType;
        string backgroundNoiseType;
        double minInterferenceTime @unit(s) = default(1ps);
        double maxTransmissionDuration @unit(s) = default(10ms);
        double alpha = default(2);
        double carrierFrequency @unit(Hz) = default(1Hz/0);
        double maxTransmissionPower @unit(W) = default(1W/0);
        double minReceptionPower @unit(dBm) = default(1dBm/0); // TODO: @unit(W) + dBm/dBW <--> W
        double minInterferencePower @unit(dBm) = default(1dBm/0); // TODO: @unit(W) + dBm/dBW <--> W
        double maxCommunicationRange @unit(m) = default(1m/0);
        double maxInterferenceRange @unit(m) = default(1m/0);
        double maxAntennaGain @unit(dB) = default(0dB);
        bool recordTransmissions = default(false);
        @class(RadioChannel);

    submodules:
        propagation: <propagationType> like IRadioSignalPropagation if propagationType != "";
        attenuation: <attenuationType> like IRadioSignalAttenuation if attenuationType != "";
        backgroundNoise: <backgroundNoiseType> like IRadioBackgroundNoise if backgroundNoiseType != "";
}
