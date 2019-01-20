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

#ifndef __INET_SNIRBASE_H
#define __INET_SNIRBASE_H

#include "inet/physicallayer/contract/packetlevel/INoise.h"
#include "inet/physicallayer/contract/packetlevel/IReception.h"
#include "inet/physicallayer/contract/packetlevel/ISnir.h"

namespace inet {

namespace physicallayer {

class INET_API SnirBase : public virtual ISnir
{
  protected:
    const IReception *reception;
    const INoise *noise;

  public:
    SnirBase(const IReception *reception, const INoise *noise);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual const IReception *getReception() const override { return reception; }
    virtual const INoise *getNoise() const override { return noise; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_SNIRBASE_H

