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

#ifndef __INET_IPATHLOSS_H_
#define __INET_IPATHLOSS_H_

#include "IPrintableObject.h"

namespace physicallayer
{

/**
 * This interface models path loss (or path attenuation) that is the reduction
 * in power density of a radio signal as it propagates through space.
 */
class INET_API IPathLoss : public IPrintableObject
{
    public:
        /**
         * Returns the loss factor as a function of propagation speed, carrier
         * frequency and distance. The value is in the range [0, 1] where 1 means
         * no loss at all and 0 means all power is lost.
         */
        virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const = 0;
};

}

#endif
