//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_GRIDOBJECTCACHE_H
#define __INET_GRIDOBJECTCACHE_H

#include "inet/common/IVisitor.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/container/SpatialGrid.h"
#include "inet/environment/common/PhysicalEnvironment.h"
#include "inet/environment/contract/IObjectCache.h"
#include "inet/environment/contract/IPhysicalObject.h"

namespace inet {

namespace physicalenvironment {

class INET_API GridObjectCache : public IObjectCache, public cModule
{
  protected:
    /** @name Parameters */
    //@{
    ModuleRefByPar<PhysicalEnvironment> physicalEnvironment;
    //@}

    /** @name Cache */
    //@{
    SpatialGrid *grid;
    //@}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual bool insertObject(const IPhysicalObject *object);

  public:
    GridObjectCache();
    virtual ~GridObjectCache();

    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const override;
};

} // namespace physicalenvironment

} // namespace inet

#endif

