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

#include "inet/physicallayer/common/packetlevel/ReceptionResult.h"

namespace inet {

namespace physicallayer {

ReceptionResult::ReceptionResult(const IReception *reception, const std::vector<const IReceptionDecision *> *decisions, const ReceptionIndication *indication) :
    reception(reception),
    decisions(decisions),
    indication(indication),
    macFrame(nullptr)
{
    macFrame = reception->getTransmission()->getMacFrame()->dup();
    bool isReceptionSuccessful = true;
    for (auto decision : *decisions)
        isReceptionSuccessful &= decision->isReceptionSuccessful();
    const_cast<cPacket *>(macFrame)->setBitError(!isReceptionSuccessful);
}

ReceptionResult::~ReceptionResult()
{
    delete macFrame;
    delete decisions;
}

std::ostream& ReceptionResult::printToStream(std::ostream& stream, int level) const
{
    stream << "ReceptionResult";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", indication = " << indication;
    return stream;
}

const cPacket* inet::physicallayer::ReceptionResult::getPhyFrame() const
{
    return nullptr;
}

const cPacket* inet::physicallayer::ReceptionResult::getMacFrame() const
{
    return macFrame;
}

} // namespace physicallayer

} // namespace inet

