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

#include "inet/physicallayer/common/packetlevel/ReceptionDecision.h"

namespace inet {

namespace physicallayer {

ReceptionDecision::ReceptionDecision(const IReception *reception, const ReceptionIndication *indication, bool isReceptionPossible, bool isReceptionAttempted, bool isReceptionSuccessful) :
    reception(reception),
    indication(indication),
    isSynchronizationPossible_(false),
    isSynchronizationAttempted_(false),
    isSynchronizationSuccessful_(false),
    isReceptionPossible_(isReceptionPossible),
    isReceptionAttempted_(isReceptionAttempted),
    isReceptionSuccessful_(isReceptionSuccessful),
    macFrame(nullptr)
{
    macFrame = reception->getTransmission()->getMacFrame()->dup();
    const_cast<cPacket *>(macFrame)->setBitError(!isReceptionSuccessful);
}

ReceptionDecision::~ReceptionDecision()
{
    delete macFrame;
}

std::ostream& ReceptionDecision::printToStream(std::ostream& stream, int level) const
{
    stream << "ReceptionDecision";
    if (level >= PRINT_LEVEL_TRACE)
        stream << (isReceptionPossible_ ? ", possible" : ", impossible")
               << (isReceptionAttempted_ ? ", attempted" : ", ignored")
               << (isReceptionSuccessful_ ? ", successful" : ", unsuccessful")
               << ", indication = " << indication ;
    return stream;
}

const cPacket* inet::physicallayer::ReceptionDecision::getPhyFrame() const
{
    return nullptr;
}

const cPacket* inet::physicallayer::ReceptionDecision::getMacFrame() const
{
    return macFrame;
}

} // namespace physicallayer

} // namespace inet

