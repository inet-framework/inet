//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211OfdmInterleaverModule.h"

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

std::ostream& Ieee80211OfdmInterleaverModule::printToStream(std::ostream& stream, int level, int evFlags) const
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

