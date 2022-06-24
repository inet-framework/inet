//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GRIDNEIGHBORCACHE_H
#define __INET_GRIDNEIGHBORCACHE_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/geometry/container/SpatialGrid.h"
#include "inet/physicallayer/wireless/common/medium/RadioMedium.h"

namespace inet {
namespace physicallayer {

class INET_API GridNeighborCache : public cSimpleModule, public INeighborCache
{
  public:
    typedef std::vector<const IRadio *> Radios;

  protected:
    class INET_API GridNeighborCacheVisitor : public IVisitor {
      protected:
        RadioMedium *radioMedium;
        IRadio *transmitter;
        const IWirelessSignal *signal;

      public:
        void visit(const cObject *radio) const override;
        GridNeighborCacheVisitor(RadioMedium *radioMedium, IRadio *transmitter, const IWirelessSignal *signal) :
            radioMedium(radioMedium), transmitter(transmitter), signal(signal) {}
    };

  protected:
    SpatialGrid *grid;
    Radios radios;
    ModuleRefByPar<RadioMedium> radioMedium;
    Coord constraintAreaMin, constraintAreaMax;
    cMessage *refillCellsTimer;
    double refillPeriod;
    double maxSpeed;
    Coord cellSize;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    void fillCubeVector();

  public:
    GridNeighborCache();
    virtual ~GridNeighborCache();

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;
    virtual void sendToNeighbors(IRadio *transmitter, const IWirelessSignal *signal, double range) const override;
};

} // namespace physicallayer
} // namespace inet

#endif

