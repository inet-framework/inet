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

#include "inet/physicallayer/common/ReceptionDecision.h"

namespace inet {

namespace physicallayer {

ReceptionDecision::ReceptionDecision(const IReception *reception, const RadioReceptionIndication *indication, bool isReceptionPossible, bool isReceptionAttempted, bool isReceptionSuccessful) :
    reception(reception),
    indication(indication),
    isSynchronizationPossible_(false),
    isSynchronizationAttempted_(false),
    isSynchronizationSuccessful_(false),
    isReceptionPossible_(isReceptionPossible),
    isReceptionAttempted_(isReceptionAttempted),
    isReceptionSuccessful_(isReceptionSuccessful)
{
}

void ReceptionDecision::printToStream(std::ostream& stream) const
{
    stream << "ReceptionDecision, "
           << (isReceptionPossible_ ? "possible" : "impossible") << ", "
           << (isReceptionAttempted_ ? "attempted" : "ignored") << ", "
           << (isReceptionSuccessful_ ? "successful" : "unsuccessful") << ", "
           << "indication = { " << indication << " }";
}

} // namespace physicallayer

} // namespace inet

