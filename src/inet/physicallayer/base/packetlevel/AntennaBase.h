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

#ifndef __INET_ANTENNABASE_H
#define __INET_ANTENNABASE_H

#include "inet/physicallayer/contract/packetlevel/IAntenna.h"

namespace inet {

namespace physicallayer {

class INET_API AntennaBase : public IAntenna, public cModule
{
  protected:
    IMobility *mobility;
    int numAntennas;

  protected:
    virtual void initialize(int stage) override;

  public:
    AntennaBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual IMobility *getMobility() const override { return mobility; }
    virtual int getNumAntennas() const override { return numAntennas; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ANTENNABASE_H

