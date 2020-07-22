//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/protocol/ethernet/ChannelDatarateReader.h"

namespace inet {

Define_Module(ChannelDatarateReader);

void ChannelDatarateReader::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        auto networkInterface = getContainingNicModule(this);
        auto txGate = networkInterface->gate("phys$o");
        auto txTransmissionChannel = txGate->findTransmissionChannel();
        if (txTransmissionChannel) {
            if (txTransmissionChannel->hasPar("datarate")) {
                double bitrate = txTransmissionChannel->par("datarate");

                //TODO which is the good solution from these?
                networkInterface->par("bitrate").setDoubleValue(bitrate);
                networkInterface->setDatarate(bitrate);
            }
        }
        else {
            networkInterface->setCarrier(false);      //TODO move to NetworkInterface?
        }
    }
}

} // namespace inet

