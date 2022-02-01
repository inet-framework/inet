//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BVHOBJECTCACHE_H
#define __INET_BVHOBJECTCACHE_H

#include "inet/common/IVisitor.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/container/BvhTree.h"
#include "inet/environment/common/PhysicalEnvironment.h"
#include "inet/environment/contract/IObjectCache.h"
#include "inet/environment/contract/IPhysicalObject.h"

namespace inet {

namespace physicalenvironment {

class INET_API BvhObjectCache : public IObjectCache, public cModule
{
  protected:
    /** @name Parameters */
    //@{
    ModuleRefByPar<PhysicalEnvironment> physicalEnvironment;
    unsigned int leafCapacity;
    const char *axisOrder;
    //@}

    /** @name Cache */
    //@{
    mutable BvhTree *bvhTree;
    mutable std::vector<const IPhysicalObject *> objects;
    //@}

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    virtual bool insertObject(const IPhysicalObject *object);

  public:
    BvhObjectCache();
    virtual ~BvhObjectCache();

    virtual void visitObjects(const IVisitor *visitor, const LineSegment& lineSegment) const override;
};

} // namespace physicalenvironment

} // namespace inet

#endif

