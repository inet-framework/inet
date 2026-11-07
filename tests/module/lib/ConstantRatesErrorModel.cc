//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "ConstantRatesErrorModel.h"

namespace inet {

Define_Module(ConstantRatesErrorModel);

void ConstantRatesErrorModel::initialize(int stage)
{
    // a model that forgets to initialize the base, like the one that #1429 is about
    if (par("initializeBase"))
        ErrorModelBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetErrorRate = par("packetErrorRate");
        bitErrorRate = par("bitErrorRate");
    }
}

std::ostream& ConstantRatesErrorModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "ConstantRatesErrorModel, packetErrorRate = " << packetErrorRate << ", bitErrorRate = " << bitErrorRate;
}

} // namespace inet
