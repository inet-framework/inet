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
#include "ospf_type.h"
#include "tcp.h"
#include "IPAddress.h"
#include "intserv.h"
#include "TED.h"



/**
 * Graph vertex used by CSPF algorithm in OspfTe
 */
struct CSPFVertex
{
    IPAddress VertexId;
    CSPFVertex *Parent;  // FIXME pointer is a bad idea here! as std::vector reallocates,
                         // every Vertex in it moves to a new pointer address...
                         // --> dereferencing the ptr will CRASH!
    double DistanceToRoot;
};



/**
 * Implements the constrained shortest path algorithm
 */
class OspfTe : public cSimpleModule
{
private:

    std::vector<CSPFVertex> CShortestPathTree;
    std::vector<TELinkState> ted;
    int local_addr;

    void  TEAddCandidates(const FlowSpecObj_t& fspec,
                          std::vector<CSPFVertex>& CandidatesList);

    std::vector<int>
    doCalculateERO(const IPAddress& dest,
                   std::vector<CSPFVertex>& CandidatesList,
                   double& outTotalMetric);

    void CspfBuildSPT(const FlowSpecObj_t& fspec,
                      std::vector<CSPFVertex>& CandidatesList);

    void CspfBuildSPT(const std::vector<simple_link_t>& links,
                      const FlowSpecObj_t& old_fspec,
                      const FlowSpecObj_t& new_fspec,
                      std::vector<CSPFVertex>& CandidatesList);

    void TEAddCandidates(const std::vector<simple_link_t>& links,
                         const FlowSpecObj_t& old_fspec,
                         const FlowSpecObj_t& new_fspec,
                         std::vector<CSPFVertex>& CandidatesList);
    void updateTED();
    void printTED();

public:
    Module_Class_Members(OspfTe, cSimpleModule, 0);
    void initialize(int stage);
    int numInitStages() { return 2; }
    virtual void handleMessage(cMessage *msg);

    /**
     * Calculates and returns ERO (Explicit Route Object) from local_addr
     * to destination using the given flow spec (required bandwidth+link delay),
     * and also returns total metric of the resulting route.
     *
     * Returned vector contains router IP addresses as int; all hops are
     * to be understood as strict.
     */
    std::vector<int> CalculateERO(const IPAddress& dest,
                                  const FlowSpecObj_t& fspec,
                                  double& outTotalMetric);

    /**
     * Calculates and returns ERO (Explicit Route Object) from local_addr to destination
     * on the given links, using the given flow spec (required bandwidth+link delay),
     * and also returns total metric of the resulting route.
     *
     * Returned vector contains router IP addresses as int; all hops are
     * to be understood as strict.
     */
    std::vector<int> CalculateERO(const IPAddress& dest,
                                  const std::vector<simple_link_t>& links,
                                  const FlowSpecObj_t& old_fspec,
                                  const FlowSpecObj_t& new_fspec,
                                  double& outTotalDelay);
};

#endif


