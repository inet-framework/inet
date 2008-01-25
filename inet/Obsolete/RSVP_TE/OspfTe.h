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
#ifndef __OSPF_TE__H__
#define __OSPF_TE__H__


#include <omnetpp.h>
#include <vector>
#include "IPAddress.h"
#include "IntServ_m.h"
#include "TED.h"


#define LS_INFINITY   100000


/**
 * Graph vertex used by CSPF algorithm in OspfTe
 */
struct CSPFVertex
{
    IPAddress vertexId;
    CSPFVertex *parent;  // FIXME pointer is a bad idea here! as std::vector reallocates,
                         // every Vertex in it moves to a new pointer address...
                         // --> dereferencing the ptr will CRASH!
    double distanceToRoot;
};

typedef std::vector<IPADDR> IPADDRVector;
typedef std::vector<CSPFVertex> CSPFVertexVector;
typedef std::vector<TELinkState> TELinkStateVector;
typedef std::vector<simple_link_t> simple_link_tVector;

/**
 * Implements the constrained shortest path algorithm
 */
class INET_API OspfTe : public cSimpleModule
{
private:

    CSPFVertexVector cshortestPathTree;
    TELinkStateVector ted;
    IPAddress routerId;

    void  TEAddCandidates(const FlowSpecObj_t& fspec,
                          CSPFVertexVector& candidatesList);

    IPADDRVector doCalculateERO(const IPAddress& dest,
                                CSPFVertexVector& candidatesList,
                                double& outTotalMetric);

    void CspfBuildSPT(const FlowSpecObj_t& fspec,
                      CSPFVertexVector& candidatesList);

    void CspfBuildSPT(const simple_link_tVector& links,
                      const FlowSpecObj_t& old_fspec,
                      const FlowSpecObj_t& new_fspec,
                      CSPFVertexVector& candidatesList);

    void TEAddCandidates(const simple_link_tVector& links,
                         const FlowSpecObj_t& old_fspec,
                         const FlowSpecObj_t& new_fspec,
                         CSPFVertexVector& candidatesList);
    void updateTED();
    void printTED();

public:
    Module_Class_Members(OspfTe, cSimpleModule, 0);
    void initialize(int stage);
    int numInitStages() const  {return 2;}
    virtual void handleMessage(cMessage *msg);

    /**
     * Calculates and returns ERO (Explicit Route Object) from routerId
     * to destination using the given flow spec (required bandwidth+link delay),
     * and also returns total metric of the resulting route.
     *
     * Returned vector contains router IP addresses as int; all hops are
     * to be understood as strict.
     */
    IPADDRVector CalculateERO(const IPAddress& dest,
                              const FlowSpecObj_t& fspec,
                              double& outTotalMetric);

    /**
     * Calculates and returns ERO (Explicit Route Object) from routerId to destination
     * on the given links, using the given flow spec (required bandwidth+link delay),
     * and also returns total metric of the resulting route.
     *
     * Returned vector contains router IP addresses as int; all hops are
     * to be understood as strict.
     */
    IPADDRVector CalculateERO(const IPAddress& dest,
                              const simple_link_tVector& links,
                              const FlowSpecObj_t& old_fspec,
                              const FlowSpecObj_t& new_fspec,
                              double& outTotalDelay);
};

#endif


