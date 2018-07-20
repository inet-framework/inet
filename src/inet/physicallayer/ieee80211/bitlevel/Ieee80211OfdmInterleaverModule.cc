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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211OfdmInterleaverModule.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211OfdmInterleaverModule);

void Ieee80211OfdmInterleaverModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        int numberOfCodedBitsPerSymbol = par("numberOfCodedBitsPerSymbol");
        int numberOfCodedBitsPerSubcarrier = par("numberOfCodedBitsPerSubcarrier");
        const Ieee80211OfdmInterleaving *interleaving = new Ieee80211OfdmInterleaving(numberOfCodedBitsPerSymbol, numberOfCodedBitsPerSubcarrier);
        interleaver = new Ieee80211OfdmInterleaver(interleaving);
    }
}

std::ostream& Ieee80211OfdmInterleaverModule::printToStream(std::ostream& stream, int level) const
{
    return interleaver->printToStream(stream, level);
}

Ieee80211OfdmInterleaverModule::~Ieee80211OfdmInterleaverModule()
{
    delete interleaver->getInterleaving();
    delete interleaver;
}
} /* namespace physicallayer */
} /* namespace inet */

