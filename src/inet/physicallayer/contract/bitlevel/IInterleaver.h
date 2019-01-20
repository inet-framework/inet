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

#ifndef __INET_IINTERLEAVER_H
#define __INET_IINTERLEAVER_H

#include "inet/common/BitVector.h"
#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"

namespace inet {
namespace physicallayer {

class INET_API IInterleaving : public IPrintableObject
{
};

class INET_API IInterleaver : public IPrintableObject
{
  public:
    virtual BitVector interleave(const BitVector& bits) const = 0;
    virtual BitVector deinterleave(const BitVector& bits) const = 0;
    virtual const IInterleaving *getInterleaving() const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_IINTERLEAVER_H

