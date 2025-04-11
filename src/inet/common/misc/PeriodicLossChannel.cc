//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/common/misc/PeriodicLossChannel.h"

namespace inet {

Register_Class(PeriodicLossChannel);

PeriodicLossChannel::PeriodicLossChannel(const char *name) : cDatarateChannel(name)
{
}

PeriodicLossChannel::~PeriodicLossChannel()
{
}

void PeriodicLossChannel::processPacket(cPacket *pkt, const SendOptions& options, simtime_t t, Result& inoutResult)
{
    cDatarateChannel::processPacket(pkt, options, t, inoutResult);
    pkt->setBitError(false);
    double per = getPacketErrorRate();
    if (per > 0.0) {
        packetCounter++;
        if (packetCounter > 1/per) {
            // drop packet
            pkt->setBitError(true);
            packetCounter = 0;
        }
    }
}

} /* namespace inet */
