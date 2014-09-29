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

#include "Ieee80211InterleaverModule.h"

namespace inet {
namespace physicallayer {

Define_Module(Ieee80211InterleaverModule);

void Ieee80211InterleaverModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        int numberOfCodedBitsPerSymbol = par("numberOfCodedBitsPerSymbol");
        int numberOfCodedBitsPerSubcarrier = par("numberOfCodedBitsPerSubcarrier");
        Ieee80211Interleaving *interleaving = new Ieee80211Interleaving(numberOfCodedBitsPerSymbol, numberOfCodedBitsPerSubcarrier);
        interleaver = new Ieee80211Interleaver(interleaving);
    }
}

Ieee80211InterleaverModule::~Ieee80211InterleaverModule()
{
    delete interleaver;
}

} /* namespace physicallayer */
} /* namespace inet */
