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

#ifndef __INET_INTERFERENCE_H
#define __INET_INTERFERENCE_H

#include "inet/physicallayer/contract/packetlevel/IInterference.h"

namespace inet {

namespace physicallayer {

class INET_API Interference : public virtual IInterference
{
  protected:
    const INoise *backgroundNoise;
    const std::vector<const IReception *> *interferingReceptions;

  public:
    Interference(const INoise *noise, const std::vector<const IReception *> *receptions);
    virtual ~Interference();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual const INoise *getBackgroundNoise() const override { return backgroundNoise; }
    virtual const std::vector<const IReception *> *getInterferingReceptions() const override { return interferingReceptions; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_INTERFERENCE_H

