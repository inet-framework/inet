//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include "inet/protocol/fragmentation/policy/LengthBasedFragmenterPolicy.h"

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
    Enter_Method_Silent("computeFragmentLengths");
    std::vector<b> fragmentLengths;
    if (maxFragmentLength >= packet->getTotalLength())
        fragmentLengths.push_back(packet->getTotalLength());
    else {
        b remainingLength = packet->getTotalLength();
        for (int i = 0; fragmentHeaderLength + remainingLength > maxFragmentLength; i++) {
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

