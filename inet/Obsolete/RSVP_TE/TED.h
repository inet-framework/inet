/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
#ifndef __OSPF_TED____H
#define __OSPF_TED____H

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <vector>
#include "MPLSModule.h"
#include "IntServ_m.h" // FIXME only for IPADDR -- remove after the transition!


/**
 * Link state structure in TED
 */
struct TELinkState
{
    IPAddress advrouter;
    int type;
    IPAddress linkid;
    IPAddress local;
    IPAddress remote;
    double metric;
    double MaxBandwith;
    double MaxResvBandwith;
    double UnResvBandwith[8];
    int AdminGrp;
};

struct simple_link_t
{
    IPADDR advRouter;
    IPADDR id;
};

/**
 * Traffic Engineering Database.
 */
class INET_API TED : public cSimpleModule
{
private:
    static int tedModuleId;
    std::vector<TELinkState> ted;

public:
    /**
     * Access to TED instance.
     */
    static TED *getGlobalInstance();

    Module_Class_Members(TED, cSimpleModule, 0);
    virtual ~TED();

    virtual int numInitStages() const  {return 4;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *);

    /**
     * Extracts model topology using cTopology, and completely
     * rebuilds link database.
     */
    void buildDatabase();

    /**
     * Access TED database (readonly)
     */
    const std::vector<TELinkState>& getTED();

    /**
     * Replace TED database with an updated one.
     */
    void updateTED(const std::vector<TELinkState>& copy);

    /**
     * Utility function
     */
    void printDatabase();

    /**
     * Dynamically change link bandwidth. Used by testing scenarios.
     */
    void updateLink(simple_link_t* aLink, double metric, double bw);
};

std::ostream& operator<<(std::ostream& os, const TELinkState& linkstate);

#endif
