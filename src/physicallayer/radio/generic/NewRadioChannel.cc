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

#include "NewRadioChannel.h"
#include "NewRadio.h"
#include "ScalarImplementation.h"

Define_Module(NewRadioChannel);

void NewRadioChannel::initialize(int stage)
{
    RadioChannel::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        minInterferenceTime = computeMinInterferenceTime();
        maxTransmissionDuration = computeMaxTransmissionDuration();
        // TODO: use computeMaxCommunicationRange();
        maxCommunicationRange = computeMaxInterferenceRange();
        maxInterferenceRange = computeMaxInterferenceRange();
        propagation = check_and_cast<IRadioSignalPropagation *>(getSubmodule("propagation"));
        attenuation = check_and_cast<IRadioSignalAttenuation *>(getSubmodule("attenuation"));
        backgroundNoise = dynamic_cast<IRadioBackgroundNoise *>(getSubmodule("backgroundNoise"));
    }
    else if (stage == INITSTAGE_LAST)
    {
        EV_DEBUG << "Radio channel initialized with"
                 << " minimum interference time = " << minInterferenceTime << " s"
                 << ", maximum transmission duration = " << maxTransmissionDuration << " s"
                 << ", maximum communication range = " << maxCommunicationRange
                 << ", maximum interference range = " << maxInterferenceRange
                 << ", " << propagation << ", " << attenuation << ", " << backgroundNoise << endl;
    }
}
