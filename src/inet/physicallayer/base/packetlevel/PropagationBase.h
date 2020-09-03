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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __INET_PROPAGATIONBASE_H
#define __INET_PROPAGATIONBASE_H

#include "inet/physicallayer/contract/packetlevel/IPropagation.h"

namespace inet {

namespace physicallayer {

class INET_API PropagationBase : public cModule, public IPropagation
{
  protected:
    mps propagationSpeed;
    mutable long arrivalComputationCount;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;

  public:
    PropagationBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual mps getPropagationSpeed() const override { return propagationSpeed; }
};

} // namespace physicallayer

} // namespace inet

#endif

