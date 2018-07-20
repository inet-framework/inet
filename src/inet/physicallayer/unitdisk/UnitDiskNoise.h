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

#ifndef __INET_IDEALNOISE_H
#define __INET_IDEALNOISE_H

#include "inet/physicallayer/base/packetlevel/NoiseBase.h"

namespace inet {

namespace physicallayer {

class INET_API UnitDiskNoise : public NoiseBase
{
  protected:
    bool isInterfering_;

  public:
    UnitDiskNoise(simtime_t startTime, simtime_t endTime, bool isInterfering);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual bool isInterfering() const { return isInterfering_; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IDEALNOISE_H

