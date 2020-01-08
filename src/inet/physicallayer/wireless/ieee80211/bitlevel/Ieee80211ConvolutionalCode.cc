//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

