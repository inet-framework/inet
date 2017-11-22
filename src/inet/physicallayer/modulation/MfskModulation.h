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

#ifndef __INET_MFSKMODULATION_H
#define __INET_MFSKMODULATION_H

#include "inet/physicallayer/contract/packetlevel/IModulation.h"

namespace inet {

namespace physicallayer {

/**
 * This modulation implements parameterized frequency-shift keying.
 *
 * http://en.wikipedia.org/wiki/Multiple_frequency-shift_keying
 */
class INET_API MfskModulation : public IModulation
{
  protected:
    unsigned int codeWordSize;

  public:
    MfskModulation(unsigned int codeWordSize);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override;
    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_MFSKMODULATION_H

