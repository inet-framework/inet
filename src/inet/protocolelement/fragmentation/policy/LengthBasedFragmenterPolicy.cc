//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/fragmentation/policy/LengthBasedFragmenterPolicy.h"

namespace inet {

Define_Module(LengthBasedFragmenterPolicy);

void LengthBasedFragmenterPolicy::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        minFragmentLength = b(par("minFragmentLength"));
        maxFragmentLength = b(par("maxFragmentLength"));
        roundingLength = b(par("roundingLength"));
        fragmentHeaderLength = b(par("fragmentHeaderLength"));
    }
}

std::vector<b> LengthBasedFragmenterPolicy::computeFragmentLengths(Packet *packet) const
{
    Enter_Method("computeFragmentLengths");
    std::vector<b> fragmentLengths;
    if (maxFragmentLength >= packet->getTotalLength())
        fragmentLengths.push_back(packet->getTotalLength());
    else {
        b remainingLength = packet->getTotalLength();
        while (fragmentHeaderLength + remainingLength > maxFragmentLength) {
            auto fragmentLength = maxFragmentLength - fragmentHeaderLength;
            fragmentLengths.push_back(fragmentLength);
            remainingLength -= fragmentLength;
        }
        if (remainingLength != b(0))
            fragmentLengths.push_back(remainingLength);
    }
    return fragmentLengths;
}

} // namespace inet

