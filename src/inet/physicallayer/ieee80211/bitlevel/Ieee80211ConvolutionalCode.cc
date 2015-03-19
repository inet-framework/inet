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

#include "inet/physicallayer/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"

namespace inet {

namespace physicallayer {

Ieee80211ConvolutionalCode::Ieee80211ConvolutionalCode(int codeRateK, int codeRateN) :
    ConvolutionalCode("133 171", nullptr, "7", -1, -1, "truncated")
{
    if (codeRateK == 1 && codeRateN == 2) {
        codeRatePuncturingK = 1;
        codeRatePuncturingN = 2;
        puncturingMatrix = "1; 1";
    }
    else if (codeRateK == 2 && codeRateN == 3) {
        codeRatePuncturingK = 2;
        codeRatePuncturingN = 3;
        puncturingMatrix = "1 1; 1 0";
    }
    else if (codeRateK == 3 && codeRateN == 4) {
        codeRatePuncturingK = 3;
        codeRatePuncturingN = 4;
        puncturingMatrix = "1 1 0; 1 0 1";
    }
    else if (codeRateK == 5 && codeRateN == 6)
    {
        codeRatePuncturingK = 5;
        codeRatePuncturingN = 6;
        puncturingMatrix = "1 1 0 1 0; 1 0 1 0 1";
    }
    else
        throw cRuntimeError("Unsupported code rate %d/%d", codeRateK, codeRateN);
}
}
}

