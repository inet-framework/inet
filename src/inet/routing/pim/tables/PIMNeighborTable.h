//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#ifndef __INET_PIMNEIGHBORTABLE_H
#define __INET_PIMNEIGHBORTABLE_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

class PIMNeighborTable;

/**
 * Class holding information about a neighboring PIM router.
 * Routers are identified by the link to which they are connected
 * and their address.
 *
 * Currently only the version of the routers are stored.
 * TODO add fields for options received in Hello Messages (RFC 3973 4.7.5, RFC 4601 4.9.2).
 */
class INET_API PIMNeighbor : public cObject
{
    friend class PIMNeighborTable;

  protected:
    PIMNeighborTable *nt;
    InterfaceEntry *ie;
    IPv4Address address;
    int version;
    unsigned int generationId;
    long drPriority;    // -1 if not present
    cMessage *livenessTimer;

  public:
    PIMNeighbor(InterfaceEntry *ie, IPv4Address address, int version);
    virtual ~PIMNeighbor();
    virtual std::string info() const override;

    int getInterfaceId() const { return ie->getInterfaceId(); }
    InterfaceEntry *getInterfacePtr() const { return ie; }
    IPv4Address getAddress() const { return address; }
    int getVersion() const { return version; }
    unsigned int getGenerationId() const { return generationId; }
    long getDRPriority() const { return drPriority; }
    cMessage *getLivenessTimer() const { return livenessTimer; }

    void setGenerationId(unsigned int genId) { if (generationId != genId) { generationId = genId; changed(); } }
    void setDRPriority(long priority) { if (drPriority != priority) { drPriority = priority; changed(); } }

  protected:
    void changed();
};

/**
 * Class holding informatation about neighboring PIM routers.
 * Routers are identified by the link to which they are connected and their address.
 *
 * Expired entries are automatically deleted.
 */
class INET_API PIMNeighborTable : public cSimpleModule
{
  public:
    enum TimerKind {
        NeighborLivenessTimer = 1
    };

  protected:
    typedef std::vector<PIMNeighbor *> PIMNeighborVector;
    typedef std::map<int, PIMNeighborVector> InterfaceToNeighborsMap;
    friend std::ostream& operator<<(std::ostream& os, const PIMNeighborVector& v);

    // contains at most one neighbor with a given (ie,address)
    InterfaceToNeighborsMap neighbors;

  public:
    virtual ~PIMNeighborTable();

    /**
     * Adds the a neighbor to the table. The operation might fail
     * if there is a neighbor with the same (ie,address) in the table.
     * Success is indicated by the returned value.
     */
    virtual bool addNeighbor(PIMNeighbor *neighbor, double holdTime);

    /**
     * Deletes a neighbor from the table. If the neighbor was
     * not found in the table then it is untouched, otherwise deleted.
     * Returns true if the neighbor object was deleted.
     */
    virtual bool deleteNeighbor(PIMNeighbor *neighbor);

    /**
     * Restarts the Neighbor Liveness timer of the given neighbor.
     * When the timer expires, the neigbor is automatically deleted.
     */
    virtual void restartLivenessTimer(PIMNeighbor *neighbor, double holdTime);

    /**
     * Returns the neighbor that is identified by the given (interfaceId,addr),
     * or nullptr if no such neighbor.
     */
    virtual PIMNeighbor *findNeighbor(int interfaceId, IPv4Address addr);

    /**
     * Returns the number of neighbors on the given interface.
     */
    virtual int getNumNeighbors(int interfaceId);

    /**
     * Returns the neighbor on the given interface at the specified position.
     */
    virtual PIMNeighbor *getNeighbor(int interfaceId, int index);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *) override;
    virtual void processLivenessTimer(cMessage *timer);
};

}    // namespace inet

#endif // ifndef __INET_PIMNEIGHBORTABLE_H

