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

#ifndef __INET_QUADTREENEIGHBORCACHE_H
#define __INET_QUADTREENEIGHBORCACHE_H

#include "inet/common/geometry/container/QuadTree.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"

namespace inet {
namespace physicallayer {

class INET_API QuadTreeNeighborCache : public cSimpleModule, public INeighborCache
{
  public:
    typedef std::vector<const IRadio *> Radios;

  protected:
    class QuadTreeNeighborCacheVisitor : public IVisitor
    {
      protected:
        RadioMedium *radioMedium;
        IRadio *transmitter;
        const ISignal *signal;

      public:
        void visit(const cObject *radio) const override;
        QuadTreeNeighborCacheVisitor(RadioMedium *radioMedium, IRadio *transmitter, const ISignal *signal) :
            radioMedium(radioMedium), transmitter(transmitter), signal(signal) {}
    };

  protected:
    QuadTree *quadTree;
    Radios radios;
    RadioMedium *radioMedium;
    cMessage *rebuildQuadTreeTimer;
    Coord constraintAreaMax, constraintAreaMin;
    unsigned int maxNumOfPointsPerQuadrant;
    double refillPeriod;
    double maxSpeed;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    void fillQuadTreeWithRadios();
    void rebuildQuadTree();

  public:
    QuadTreeNeighborCache();
    ~QuadTreeNeighborCache();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;
    virtual void sendToNeighbors(IRadio *transmitter, const ISignal *signal, double range) const override;
};

} // namespace physicallayer
} // namespace inet

#endif // ifndef __INET_QUADTREENEIGHBORCACHE_H

