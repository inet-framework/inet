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

#ifndef __INET_MPSKMODULATION_H
#define __INET_MPSKMODULATION_H

#include "inet/physicallayer/base/packetlevel/ApskModulationBase.h"

namespace inet {

namespace physicallayer {

/**
 * This modulation implements parameterized phase-shift keying that arranges
 * symbols evenly on a circle.
 *
 * http://en.wikipedia.org/wiki/Phase-shift_keying#Higher-order_PSK
 */
class INET_API MpskModulation : public ApskModulationBase
{
  public:
    MpskModulation(unsigned int codeWordSize);
    virtual ~MpskModulation();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override;
    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MPSKMODULATION_H

