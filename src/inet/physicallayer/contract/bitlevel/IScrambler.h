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

#ifndef __INET_ISCRAMBLER_H
#define __INET_ISCRAMBLER_H

#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"
#include "inet/common/BitVector.h"
#include "inet/common/ShortBitVector.h"

namespace inet {

namespace physicallayer {

class INET_API IScrambling : public IPrintableObject
{
  public:
    const ShortBitVector& getGeneratorPolynomial() const;
    const ShortBitVector& getSeed() const;
};

class INET_API IScrambler : public IPrintableObject
{
  public:
    virtual BitVector scramble(const BitVector& bits) const = 0;
    virtual BitVector descramble(const BitVector& bits) const = 0;
    virtual const IScrambling *getScrambling() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ISCRAMBLER_H

