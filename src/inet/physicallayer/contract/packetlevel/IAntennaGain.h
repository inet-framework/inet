//
// Copyright (C) 2017 Raphael Riebl, TH Ingolstadt
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

#ifndef __INET_IANTENNAGAIN_H
#define __INET_IANTENNAGAIN_H

#include "inet/common/Ptr.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"

namespace inet {
namespace physicallayer {

/**
 * This interface represents the directional selectivity of an antenna.
 */
class INET_API IAntennaGain : public IPrintableObject
#if INET_PTR_IMPLEMENTATION == INET_INTRUSIVE_PTR
    , public IntrusivePtrCounter<IAntennaGain>
#endif
{
  public:
    /**
     * Returns the minimum possible antenna gain independent of any direction.
     */
    virtual double getMinGain() const = 0;

    /**
     * Returns the maximum possible antenna gain independent of any direction.
     */
    virtual double getMaxGain() const = 0;

    /**
     * Returns the antenna gain in the provided direction. The direction is
     * determined by rotating the X axis using the given quaternion. The direction
     * is to be interpreted in the local coordinate system of the radiation pattern.
     * This way the gain depends only on the antenna radion pattern characteristics,
     * and not on the antenna orientation determined by the antenna's mobility model.
     *
     * For transmissions, it determines how well the antenna converts input
     * power into radio waves headed in the specified direction. For receptions,
     * it determines how well the antenna converts radio waves arriving from
     * the specified direction.
     */
    virtual double computeGain(const Quaternion direction) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_IANTENNAGAIN_H

