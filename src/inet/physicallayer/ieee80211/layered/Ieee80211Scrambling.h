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

#ifndef __INET_IEEE80211SCRAMBLING_H_
#define __INET_IEEE80211SCRAMBLING_H_

#include "inet/physicallayer/contract/IScrambler.h"
#include "inet/common/ShortBitVector.h"

namespace inet {
namespace physicallayer {

class INET_API Ieee80211Scrambling : public IScrambling
{
    protected:
        ShortBitVector seed;
        ShortBitVector generatorPolynomial;

    public:
        Ieee80211Scrambling(const ShortBitVector& seed, const ShortBitVector& generatorPolynomial) :
            seed(seed), generatorPolynomial(generatorPolynomial) {}
        const ShortBitVector& getGeneratorPolynomial() const { return generatorPolynomial; }
        const ShortBitVector& getSeed() const { return seed; }
        void printToStream(std::ostream& stream) const { stream << "Ieee80211Scrambler with seed : " << seed << " and with generator polynomial: " << generatorPolynomial; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211SCRAMBLING_H_ */
