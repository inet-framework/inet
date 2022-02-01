//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NEIGHBORLISTNEIGHBORCACHE_H
#define __INET_NEIGHBORLISTNEIGHBORCACHE_H

#include <set>
#include <vector>

#include "inet/common/ModuleRefByPar.h"
#include "inet/physicallayer/wireless/common/medium/RadioMedium.h"

namespace inet {
namespace physicallayer {

class INET_API NeighborListNeighborCache : public cSimpleModule, public INeighborCache
{
  public:
    struct RadioEntry {
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
    ModuleRefByPar<RadioMedium> radioMedium;
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

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual void addRadio(const IRadio *radio) override;
    virtual void removeRadio(const IRadio *radio) override;
    virtual void sendToNeighbors(IRadio *transmitter, const IWirelessSignal *signal, double range) const override;
};

} // namespace physicallayer
} // namespace inet

#endif

