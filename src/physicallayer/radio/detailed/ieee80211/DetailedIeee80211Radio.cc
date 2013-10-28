//
// Copyright (C) 2006 Andras Varga, Levente Meszaros
// Based on the Mobility Framework's SnrEval by Marc Loebbers
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

#include "PhyControlInfo_m.h"
#include "DetailedIeee80211Radio.h"
#include "Ieee80211Frame_m.h"

Define_Module(DetailedIeee80211Radio);

DetailedRadioSignal *DetailedIeee80211Radio::createSignal(cPacket *macFrame)
{
    if (dynamic_cast<Ieee80211Frame*>(macFrame)) {
        PhyControlInfo *controlInfo = dynamic_cast<PhyControlInfo*>(macFrame->getControlInfo());
        // TODO: KLUDGE:
        double bitrate = controlInfo ? controlInfo->getBitrate() : 2E+6;
        return createSignal(simTime(), packetDuration(macFrame, bitrate), txPower, bitrate);
    }
    else
        throw cRuntimeError("Unknown MAC packet");
}

DetailedRadioSignal *DetailedIeee80211Radio::createSignal(simtime_t_cref start, simtime_t_cref length, double power, double bitrate)
{
    simtime_t end = start + length;
    //create signal with start at current simtime and passed length
    DetailedRadioSignal *s = new DetailedRadioSignal(start, length);

    //create and set tx power mapping
    // TODO: channel
    double centerFreq = CENTER_FREQUENCIES[getRadioChannel()];
    ConstMapping *txPowerMapping
            = createSingleFrequencyMapping( start, end,
                                            centerFreq, 11.0e6,
                                            power);
    s->setTransmissionPower(txPowerMapping);

    //create and set bitrate mapping

    //create mapping over time
    Mapping *bitrateMapping
            = MappingUtils::createMapping(DimensionSet::timeDomain,
                                          Mapping::STEPS);

    Argument pos(start);
    bitrateMapping->setValue(pos, BITRATE_HEADER);

    pos.setTime(PHY_HEADER_LENGTH / BITRATE_HEADER);
    bitrateMapping->setValue(pos, bitrate);

    s->setBitrate(bitrateMapping);

    return s;
}

simtime_t DetailedIeee80211Radio::packetDuration(cPacket *macFrame, double br)
{
    return macFrame->getBitLength() / br + PHY_HEADER_LENGTH / BITRATE_HEADER;
}
