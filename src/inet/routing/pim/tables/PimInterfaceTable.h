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

#ifndef __INET_PIMINTERFACETABLE_H
#define __INET_PIMINTERFACETABLE_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

/**
 * An entry of PimInterfaceTable holding PIM specific parameters and state of the interface.
 */
class INET_API PimInterface : public cObject
{
  public:
    enum PimMode {
        DenseMode = 1,
        SparseMode = 2
    };

  protected:
    InterfaceEntry *ie;

    // parameters
    PimMode mode;
    bool stateRefreshFlag;

    // state
    Ipv4Address drAddress;

  public:
    PimInterface(InterfaceEntry *ie, PimMode mode, bool stateRefreshFlag)
        : ie(ie), mode(mode), stateRefreshFlag(stateRefreshFlag) { ASSERT(ie); }
    virtual std::string str() const override;

    int getInterfaceId() const { return ie->getInterfaceId(); }
    InterfaceEntry *getInterfacePtr() const { return ie; }
    PimMode getMode() const { return mode; }
    bool getSR() const { return stateRefreshFlag; }

    Ipv4Address getDRAddress() const { return drAddress; }
    void setDRAddress(Ipv4Address address) { drAddress = address; }
};

/**
 * PimInterfaceTable contains an PimInterface entry for each interface on which PIM is enabled.
 * When interfaces are added to/deleted from the InterfaceTable, then the corresponding
 * PimInterface entry is added/deleted automatically.
 */
class INET_API PimInterfaceTable : public cSimpleModule, protected cListener
{
  protected:
    typedef std::vector<PimInterface *> PimInterfaceVector;

    PimInterfaceVector pimInterfaces;

  public:
    virtual ~PimInterfaceTable();

    virtual int getNumInterfaces() { return pimInterfaces.size(); }
    virtual PimInterface *getInterface(int k) { return pimInterfaces[k]; }
    virtual PimInterface *getInterfaceById(int interfaceId);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    virtual void configureInterfaces(cXMLElement *config);
    virtual PimInterface *createInterface(InterfaceEntry *ie, cXMLElement *config);
    virtual PimInterfaceVector::iterator findInterface(InterfaceEntry *ie);
    virtual void addInterface(InterfaceEntry *ie);
    virtual void removeInterface(InterfaceEntry *ie);
};

}    // namespace inet

#endif // ifndef __INET_PIMINTERFACETABLE_H

