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

#include "Ieee80211ScalarRadioSignalTransmitter.h"
#include "WifiMode.h"
#include "ModulationType.h"
#include "RadioControlInfo_m.h"
#include "Ieee80211Consts.h"

Define_Module(Ieee80211ScalarRadioSignalTransmitter);

void Ieee80211ScalarRadioSignalTransmitter::initialize(int stage)
{
    ScalarRadioSignalTransmitter::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
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

const IRadioSignalTransmission *Ieee80211ScalarRadioSignalTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, simtime_t startTime) const
{
    RadioTransmissionRequest *controlInfo = dynamic_cast<RadioTransmissionRequest *>(macFrame->getControlInfo());
    W transmissionPower = controlInfo && !isNaN(controlInfo->getPower().get()) ? controlInfo->getPower() : power;
    bps transmissionBitrate = controlInfo && !isNaN(controlInfo->getBitrate().get()) ? controlInfo->getBitrate() : bitrate;
    ModulationType modulationType = WifiModulationType::getModulationType(opMode, transmissionBitrate.get());
    simtime_t duration = SIMTIME_DBL(WifiModulationType::calculateTxDuration(macFrame->getBitLength(), modulationType, preambleMode));
    simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    Coord startPosition = mobility->getPosition(startTime);
    Coord endPosition = mobility->getPosition(endTime);
    return new ScalarRadioSignalTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, modulation, headerBitLength, macFrame->getBitLength(), transmissionBitrate, transmissionPower, carrierFrequency, bandwidth);
}
