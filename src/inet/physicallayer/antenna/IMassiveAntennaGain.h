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

#ifndef __INET_IMASSIVEANTENNAGAIN_H
#define __INET_IMASSIVEANTENNAGAIN_H

#include "inet/physicallayer/contract/packetlevel/IAntennaGain.h"

namespace inet {
namespace physicallayer {

/**
 * This interface represents the directional selectivity of an antenna.
 */
class INET_API IMassiveAntennaGain : public IAntennaGain
{
  public:
    virtual double getAngolo(Coord p1, Coord p2)const = 0;
    virtual double getPhizero() = 0;
    static int getM() = 0;
    virtual double computeRecGain(const Quaternion &direction) const = 0;
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_IANTENNAGAIN_H

