//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#ifndef __INET_PIMINTERFACETABLE_H
#define __INET_PIMINTERFACETABLE_H

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

/**
 * An entry of PimInterfaceTable holding PIM specific parameters and state of the interface.
 */
class INET_API PimInterface : public cObject
{
  public:
    enum PimMode {
        DenseMode  = 1,
        SparseMode = 2
    };

  protected:
    NetworkInterface *ie;

    // parameters
    PimMode mode;
    bool stateRefreshFlag;

    // state
    Ipv4Address drAddress;

  public:
    PimInterface(NetworkInterface *ie, PimMode mode, bool stateRefreshFlag)
        : ie(ie), mode(mode), stateRefreshFlag(stateRefreshFlag) { ASSERT(ie); }
    virtual std::string str() const override;

    int getInterfaceId() const { return ie->getInterfaceId(); }
    NetworkInterface *getInterfacePtr() const { return ie; }
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
    virtual PimInterface *createInterface(NetworkInterface *ie, cXMLElement *config);
    virtual PimInterfaceVector::iterator findInterface(NetworkInterface *ie);
    virtual void addInterface(NetworkInterface *ie);
    virtual void removeInterface(NetworkInterface *ie);
};

} // namespace inet

#endif

