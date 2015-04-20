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

#include "inet/physicallayer/common/bitlevel/AdditiveScramblerModule.h"

namespace inet {

namespace physicallayer {

Define_Module(AdditiveScramblerModule);

void AdditiveScramblerModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        ShortBitVector seed(par("seed").stringValue());
        ShortBitVector generatorPolynomial(par("generatorPolynomial").stringValue());
        const AdditiveScrambling *scrambling = new AdditiveScrambling(seed, generatorPolynomial);
        scrambler = new AdditiveScrambler(scrambling);
    }
}

std::ostream& AdditiveScramblerModule::printToStream(std::ostream& stream, int level) const
{
    return scrambler->printToStream(stream, level);
}

AdditiveScramblerModule::~AdditiveScramblerModule()
{
    delete scrambler;
}
} /* namespace physicallayer */
} /* namespace inet */

