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

#ifndef __INET_NEIGHBORLISTCACHE_H
#define __INET_NEIGHBORLISTCACHE_H

#include "RadioMedium.h"
#include <set>
#include <vector>

namespace inet {
namespace physicallayer {
class NeighborListCache : public RadioMedium::INeighborCache, public cSimpleModule
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
    double updatePeriod;
    double range;
    double maxSpeed;

  protected:
    virtual int numInitStages() const { return NUM_INIT_STAGES; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
    void updateNeighborList(RadioEntry *radioEntry);
    void updateNeighborLists();
    void removeRadioFromNeighborLists(const IRadio *radio);

  public:
    void addRadio(const IRadio *radio);
    void removeRadio(const IRadio *radio);
    void sendToNeighbors(IRadio *transmitter, const IRadioFrame *frame);
    NeighborListCache() : radioMedium(NULL), updateNeighborListsTimer(NULL) {};
    ~NeighborListCache();
};
} // namespace physicallayer
} // namespace inet

#endif /* NEIGHBORLISTCACHE_H_ */

