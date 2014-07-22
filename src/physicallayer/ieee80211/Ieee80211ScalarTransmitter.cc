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

#include "Ieee80211ScalarTransmitter.h"
#include "Ieee80211ScalarTransmission.h"
#include "IMobility.h"
#include "IRadio.h"
#include "WifiMode.h"
#include "ModulationType.h"
#include "RadioControlInfo_m.h"
#include "Ieee80211Consts.h"

namespace inet {

using namespace ieee80211;

namespace physicallayer {

Define_Module(Ieee80211ScalarTransmitter);

void Ieee80211ScalarTransmitter::initialize(int stage)
{
    ScalarTransmitter::initialize(stage);
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
        carrierFrequency = Hz(CENTER_FREQUENCIES[par("channelNumber")]);
    }
}

const ITransmission *Ieee80211ScalarTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, simtime_t startTime) const
{
    RadioTransmissionRequest *controlInfo = dynamic_cast<RadioTransmissionRequest *>(macFrame->getControlInfo());
    W transmissionPower = controlInfo && !isNaN(controlInfo->getPower().get()) ? controlInfo->getPower() : power;
    bps transmissionBitrate = controlInfo && !isNaN(controlInfo->getBitrate().get()) ? controlInfo->getBitrate() : bitrate;
    ModulationType modulationType = WifiModulationType::getModulationType(opMode, transmissionBitrate.get());
    const simtime_t duration = SIMTIME_DBL(WifiModulationType::calculateTxDuration(macFrame->getBitLength(), modulationType, preambleMode));
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new Ieee80211ScalarTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, headerBitLength, macFrame->getBitLength(), carrierFrequency, bandwidth, transmissionBitrate, transmissionPower, opMode, preambleMode);
}

} // namespace physicallayer

} // namespace inet

