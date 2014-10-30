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

#include "inet/physicallayer/base/FlatTransmissionBase.h"
#include "inet/physicallayer/common/ModulationType.h"
#include "inet/physicallayer/ieee80211/Ieee80211DimensionalReceiver.h"
#include "inet/physicallayer/ieee80211/Ieee80211DimensionalTransmission.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211DimensionalReceiver);

Ieee80211DimensionalReceiver::Ieee80211DimensionalReceiver() :
    DimensionalReceiver(),
    opMode('\0'),
    preambleMode((WifiPreamble) - 1)
{
}

void Ieee80211DimensionalReceiver::initialize(int stage)
{
    DimensionalReceiver::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *opModeString = par("opMode");
        if (!strcmp("b", opModeString))
            opMode = 'b';
        else if (!strcmp("g", opModeString))
            opMode = 'g';
        else if (!strcmp("a", opModeString))
            opMode = 'a';
        else if (!strcmp("p", opModeString))
            opMode = 'p';
        else
            opMode = 'g';
        const char *preambleModeString = par("preambleMode");
        if (!strcmp("short", preambleModeString))
            preambleMode = WIFI_PREAMBLE_SHORT;
        else if (!strcmp("long", preambleModeString))
            preambleMode = WIFI_PREAMBLE_LONG;
        else
            throw cRuntimeError("Unknown preamble mode");
    }
}

bool Ieee80211DimensionalReceiver::computeIsReceptionPossible(const ITransmission *transmission) const
{
    const Ieee80211DimensionalTransmission *ieee80211Transmission = check_and_cast<const Ieee80211DimensionalTransmission *>(transmission);
    return DimensionalReceiver::computeIsReceptionPossible(transmission) && ieee80211Transmission->getOpMode() == opMode && ieee80211Transmission->getPreambleMode() == preambleMode;
}

} // namespace physicallayer

} // namespace inet

