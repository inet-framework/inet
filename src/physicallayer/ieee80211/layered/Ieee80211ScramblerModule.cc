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

#include "Ieee80211ScramblerModule.h"

namespace inet {
namespace physicallayer {

void Ieee80211ScramblerModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        ShortBitVector seed(par("seed").stringValue());
        ShortBitVector generatorPolynomial(par("generatorPolynomial").stringValue());
        Ieee80211Scrambling *scrambling = new Ieee80211Scrambling(seed, generatorPolynomial);
        scrambler = new Ieee80211Scrambler(scrambling);
    }
}

Ieee80211ScramblerModule::~Ieee80211ScramblerModule()
{
    delete scrambler;
}

} /* namespace physicallayer */
} /* namespace inet */
