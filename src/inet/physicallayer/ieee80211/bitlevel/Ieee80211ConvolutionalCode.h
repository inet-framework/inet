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

#ifndef __INET_IEEE80211CONVOLUTIONALCODE_H
#define __INET_IEEE80211CONVOLUTIONALCODE_H

#include "inet/physicallayer/common/bitlevel/ConvolutionalCoder.h"
#include "inet/physicallayer/common/bitlevel/ConvolutionalCode.h"

namespace inet {

namespace physicallayer {

/* By default, it supports the following code rates often used by IEEE802.11 PHY:
 *
 *  - k = 1, n = 2 with constraint length 7, generator polynomials: (133)_8 = (1011011)_2,
 *                                                                  (171)_8 = (1111001)_2
 * Higher code rates are achieved by puncturing:
 *
 *  - k = 2, n = 3 with puncturing matrix: |1 1|
 *                                         |1 0|
 *
 *  - k = 3, n = 4 with puncturing matrix: |1 1 0|
 *                                         |1 0 1|
 *
 *  - k = 5, n = 6 with puncturing matrix: |1 1 0 1 0|
 *                                         |1 0 1 0 1|
 *
 *
 * We use industry-standard generator polynomials : (133)_8, (171)_8.
 */
class INET_API Ieee80211ConvolutionalCode : public ConvolutionalCode
{
  public:
    Ieee80211ConvolutionalCode(int codeRateK, int codeRateN);
};
} /* namespace physicallayer */
} /* namespace inet */

#endif // ifndef __INET_IEEE80211CONVOLUTIONALCODE_H

