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

#include "inet/physicallayer/wireless/ieee80211/bitlevel/Ieee80211ConvolutionalCode.h"

namespace inet {

namespace physicallayer {

Ieee80211ConvolutionalCode::Ieee80211ConvolutionalCode(int codeRateK, int codeRateN) :
    ConvolutionalCode(std::vector<std::vector<intval_t>>{{133, 171}},
            (codeRateK == 1 && codeRateN == 2) ? std::vector<std::vector<intval_t>>{{1}, {1}} :
            (codeRateK == 2 && codeRateN == 3) ? std::vector<std::vector<intval_t>>{{1, 1}, {1, 0}} :
            (codeRateK == 3 && codeRateN == 4) ? std::vector<std::vector<intval_t>>{{1, 1, 0}, {1, 0, 1}} :
            (codeRateK == 5 && codeRateN == 6) ? std::vector<std::vector<intval_t>>{{1, 1, 0, 1, 0}, {1, 0, 1, 0, 1}} : std::vector<std::vector<intval_t>>{{}},
            {7}, codeRateK, codeRateN, "truncated")
{
    if (puncturingMatrix.size() == 0)
        throw cRuntimeError("Unsupported code rate %d/%d", codeRateK, codeRateN);
}

} // namespace physicallayer
} // namespace inet

