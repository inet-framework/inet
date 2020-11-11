//
// Copyright (C) 2014 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/wireless/apsk/bitlevel/ApskCode.h"

namespace inet {

namespace physicallayer {

#define SYMBOL_SIZE    48

ApskCode::ApskCode(const ConvolutionalCode *convCode, const IInterleaving *interleaving, const IScrambling *scrambling) :
    convolutionalCode(convCode),
    interleaving(interleaving),
    scrambling(scrambling)
{
}

ApskCode::~ApskCode()
{
    delete convolutionalCode;
    delete interleaving;
    delete scrambling;
}

std::ostream& ApskCode::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "ApskCode";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(convolutionalCode, printFieldToString(convolutionalCode, level + 1, evFlags))
               << EV_FIELD(interleaving, printFieldToString(interleaving, level + 1, evFlags))
               << EV_FIELD(scrambling, printFieldToString(scrambling, level + 1, evFlags));
    return stream;
}

} /* namespace physicallayer */

} /* namespace inet */

