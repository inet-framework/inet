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
#include "inet/common/BitVector.h"
#include "inet/common/ShortBitVector.h"

namespace inet {
namespace physicallayer {

/*
 * It is a IEEE 802.11 data scrambler/descrambler implementation.
 * The default generator polynomial is S(x) = x^7 + x^4 + 1.
 * The details can be found in: Part 11: Wireless LAN Medium Access Control (MAC) and Physical Layer (PHY) Specifications,
 * 18.3.5.5 PLCP DATA scrambler and descrambler
 */
class Ieee80211Scrambler : public cSimpleModule
{
    protected:
        BitVector scramblingSequence;

    protected:
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg) { throw cRuntimeError("This module doesn't handle self messages"); }
        inline bool eXOR(bool alpha, bool beta) const
        {
            return (alpha || beta) && !(alpha && beta);
        }
        BitVector generateScramblingSequence(const ShortBitVector& generatorPolynomial, const ShortBitVector& seed) const;

    public:
        BitVector scrambling(const BitVector& bits) const;
        const BitVector& getScramblingSequcene() const { return scramblingSequence; }
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* __INET_IEEE80211SCRAMBLER_H_ */
