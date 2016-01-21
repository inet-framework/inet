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

#ifndef __INET_NEIGHBORLISTNEIGHBORCACHE_H
#define __INET_NEIGHBORLISTNEIGHBORCACHE_H

#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include <set>
#include <vector>

namespace inet {

namespace physicallayer {

class INET_API NeighborListNeighborCache : public cSimpleModule, public INeighborCache
{
  public:
    struct RadioEntry
    {
        RadioEntry(const IRadio *radio) : radio(radio) {};
        const IRadio *radio;
        std::vector<const IRadio *> neighborVector;
        bool operator==(RadioEntry *rhs) const
        {
            return this->radio->getId() == rhs->radio->getId();
        }
    };
    typedef std::vector<RadioEntry *> RadioEntries;
    typedef std::vector<const IRadio *> Radios;
    typedef std::map<const IRadio *, RadioEntry *> RadioEntryCache;

  protected:
    RadioMedium *radioMedium;
    RadioEntries radios;
    cMessage *updateNeighborListsTimer;
    RadioEntryCache radioToEntry;
    double refillPeriod;
    double range;
    double maxSpeed;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    void updateNeighborList(RadioEntry *radioEntry);
    void updateNeighborLists();
    void removeRadioFromNeighborLists(const IRadio *radio);

  public:
    NeighborListNeighborCache();
    ~NeighborListNeighborCache();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;
    virtual void sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame, double range) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_NEIGHBORLISTNEIGHBORCACHE_H

