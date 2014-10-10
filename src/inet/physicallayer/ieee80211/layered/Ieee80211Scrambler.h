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

#ifndef __INET_IEEE80211SCRAMBLER_H_
#define __INET_IEEE80211SCRAMBLER_H_

#include "inet/common/INETDefs.h"
#include "inet/physicallayer/contract/IScrambler.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211Scrambling.h"
#include "inet/common/BitVector.h"
#include "inet/common/ShortBitVector.h"

namespace inet {
namespace physicallayer {

/* TODO: It is not a completely generic scrambler, it implements a 802.11a scrambler, however it can be parameterized
 * with a seed and a generator polynomial. Make it more generic and implement other scramblers. For example: there is
 * difference between a multiplicative scrambler and an additive scrambler.
 *
 * It is a IEEE 802.11 data scrambler/descrambler implementation.
 * The details can be found in: Part 11: Wireless LAN Medium Access Control (MAC) and Physical Layer (PHY) Specifications,
 * 18.3.5.5 PLCP DATA scrambler and descrambler
 */
class Ieee80211Scrambler : public ScramblerBase
{
    protected:
        BitVector scramblingSequence;
        const Ieee80211Scrambling *scrambling;

    protected:
        inline bool eXOR(bool alpha, bool beta) const { return alpha != beta; }
        BitVector generateScramblingSequence(const ShortBitVector& generatorPolynomial, const ShortBitVector& seed) const;

    public:
        Ieee80211Scrambler(const Ieee80211Scrambling *scrambling);
        BitVector scramble(const BitVector& bits) const;
        BitVector descramble(const BitVector& bits) const { return scramble(bits); }
        const Ieee80211Scrambling *getScrambling() const { return scrambling; }
        const BitVector& getScramblingSequcene() const { return scramblingSequence; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211SCRAMBLER_H_ */
