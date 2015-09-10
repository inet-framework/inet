//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "APSKCode.h"

namespace inet {

namespace physicallayer {

#define SYMBOL_SIZE    48

APSKCode::APSKCode(const ConvolutionalCode *convCode, const IInterleaving *interleaving, const IScrambling *scrambling) :
    convolutionalCode(convCode),
    interleaving(interleaving),
    scrambling(scrambling)
{
}

APSKCode::~APSKCode()
{
    delete convolutionalCode;
    delete interleaving;
    delete scrambling;
}

std::ostream& APSKCode::printToStream(std::ostream& stream, int level) const
{
    stream << "APSKCode";
    if (level >= PRINT_LEVEL_DETAIL)
        stream << ", convolutionalCode = " << printObjectToString(convolutionalCode, level - 1)
               << ", interleaving = " << printObjectToString(interleaving, level - 1)
               << ", scrambling = " << printObjectToString(scrambling, level - 1);
    return stream;
}

} /* namespace physicallayer */

} /* namespace inet */

