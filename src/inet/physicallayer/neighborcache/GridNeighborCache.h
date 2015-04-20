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

#ifndef __INET_GRIDNEIGHBORCACHE_H
#define __INET_GRIDNEIGHBORCACHE_H

#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include "inet/common/geometry/container/SpatialGrid.h"

namespace inet {

namespace physicallayer {

class INET_API GridNeighborCache : public cSimpleModule, public INeighborCache
{
  public:
    typedef std::vector<const IRadio *> Radios;

  protected:
    class GridNeighborCacheVisitor : public IVisitor
    {
      protected:
        RadioMedium *radioMedium;
        IRadio *transmitter;
        const IRadioFrame *frame;

      public:
        void visit(const cObject *radio) const override;
        GridNeighborCacheVisitor(RadioMedium *radioMedium, IRadio *transmitter, const IRadioFrame *frame) :
            radioMedium(radioMedium), transmitter(transmitter), frame(frame) {}
    };
  protected:
    SpatialGrid *grid;
    Radios radios;
    RadioMedium *radioMedium;
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

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;
    virtual void sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame, double range) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_GRIDNEIGHBORCACHE_H

