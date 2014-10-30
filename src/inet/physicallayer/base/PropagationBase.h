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

#ifndef __INET_PROPAGATIONBASE_H
#define __INET_PROPAGATIONBASE_H

#include "inet/physicallayer/contract/IPropagation.h"

namespace inet {

namespace physicallayer {

class INET_API PropagationBase : public cModule, public IPropagation
{
  protected:
    mps propagationSpeed;
    mutable long arrivalComputationCount;

  protected:
    virtual void initialize(int stage);
    virtual void finish();

  public:
    PropagationBase();

    virtual void printToStream(std::ostream& stream) const;
    virtual mps getPropagationSpeed() const { return propagationSpeed; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_PROPAGATIONBASE_H

