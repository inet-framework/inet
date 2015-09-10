//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_IFECCODER_H
#define __INET_IFECCODER_H

#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"
#include "inet/common/BitVector.h"

namespace inet {

namespace physicallayer {

class INET_API IForwardErrorCorrection : public IPrintableObject
{
  public:
    virtual double getCodeRate() const = 0;
    virtual int getEncodedLength(int decodedLength) const = 0;
    virtual int getDecodedLength(int encodedLength) const = 0;
    virtual double computeNetBitErrorRate(double grossBitErrorRate) const = 0;
};

class INET_API IFECCoder : public IPrintableObject
{
  public:
    virtual BitVector encode(const BitVector& informationBits) const = 0;
    virtual std::pair<BitVector, bool> decode(const BitVector& encodedBits) const = 0;
    virtual const IForwardErrorCorrection *getForwardErrorCorrection() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IFECCODER_H

