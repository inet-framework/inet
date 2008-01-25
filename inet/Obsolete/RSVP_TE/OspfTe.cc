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
#include "OspfTe.h"
#include "IPAddress.h"
#include "IPAddressResolver.h"
#include "RoutingTableAccess.h"


Define_Module(OspfTe);


void OspfTe::initialize(int stage)
{
    if (stage==1)
    {
        routerId = RoutingTableAccess().get()->getRouterId().getInt();
        ASSERT(!routerId.isUnspecified());

        // to invoke handleMessage() when we start  FIXME what?????? Andras
        scheduleAt(simTime(), new cMessage());
    }
}

void OspfTe::handleMessage(cMessage * msg)
{
    // one-shot handleMessage()
    // FIXME should really be done from another init stage
    if (!msg->isSelfMessage())
        error("Message arrived -- OSPF/TE doesn't process messages, it is used via direct method calls");
    delete msg;

    ev << "OSPF/TE: I am starting\n";

    bool isIR = false;
    cModule *curmod = this;
    for (curmod = parentModule(); curmod != NULL; curmod = curmod->parentModule())
    {
        if (curmod->hasPar("IsIR"))     // FIXME why not use ancestor par of *this* module? Andras
        {
            isIR = curmod->par("IsIR").boolValue();
            break;
        }
    }

    if (isIR)
    {
        updateTED();
    }
}


/*******************************************************************************
*                           OSPF-TE (CSPF)ROUTING
*
*******************************************************************************/
void OspfTe::TEAddCandidates(const FlowSpecObj_t& fspec,
                             CSPFVertexVector &candidatesList)
{
    CSPFVertex *vertexV = &cshortestPathTree.back();

    // FIXME this is a workaround -- vertices in candidatesList will have
    // pointers to each other so we have to prevent reallocation.
    // Here we just reserve enough so that realloc is never necessary.
    candidatesList.reserve(ted.size());

    // for each link
    TELinkStateVector::iterator tedIter;
    for (tedIter = ted.begin(); tedIter != ted.end(); tedIter++)
    {
        const TELinkState& linkstate = (*tedIter);

        // Other ends of the links to V only
        if (linkstate.advrouter != vertexV->vertexId)
            continue;

        // Not in the ConstrainedShortestPathTree only
        bool found = false;
        CSPFVertexVector::iterator sptIter;
        CSPFVertex sptVertex;
        for (sptIter = cshortestPathTree.begin(); sptIter != cshortestPathTree.end(); sptIter++)
        {
            sptVertex = (*sptIter);
            if (linkstate.linkid == sptVertex.vertexId)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;

        // Satisfy the resource constraint of BW only
        if (linkstate.UnResvBandwith[0] < fspec.req_bandwidth)
            continue;

        // Normal Dijkstra Algorithm for adding candidate
        CSPFVertex vertexW;
        vertexW.vertexId = linkstate.linkid;
        vertexW.distanceToRoot = vertexV->distanceToRoot + linkstate.metric;
        vertexW.parent = vertexV;

        found = false;
        if (!candidatesList.empty())
        {
            for (sptIter = candidatesList.begin(); sptIter != candidatesList.end(); sptIter++)
            {
                sptVertex = (*sptIter);

                if (sptVertex.vertexId == vertexW.vertexId)
                {
                    found = true;
                    break;
                }
            }
        }

        // Relaxation
        if (found)
        {
            if (sptVertex.distanceToRoot > vertexW.distanceToRoot)
            {
                sptVertex.distanceToRoot = vertexW.distanceToRoot;
                sptVertex.parent = vertexV;
                // TECalculateNextHops( vertexW, vertexV, *linkstate);
            }
        }
        else
        {
            candidatesList.push_back(vertexW);
        }
    }

    CspfBuildSPT(fspec, candidatesList);
}

IPADDRVector OspfTe::doCalculateERO(const IPAddress& dest,
                                    CSPFVertexVector& candidatesList,
                                    double &outTotalMetric)
{
    IPADDRVector EROList;
    if (!candidatesList.empty())
    {
        // find dest among candidates
        CSPFVertexVector::iterator i;
        for (i = candidatesList.begin(); i != candidatesList.end(); i++)
            if ((*i).vertexId.equals(dest))
                break;

        // if not found -- do what??? FIXME
        if (i == candidatesList.end())
            error("doCalculateERO(): %s not found", dest.str().c_str());

        // totalMetric is its distance to root
        outTotalMetric = (*i).distanceToRoot;

        // insert all parents up to the root into EROList
        CSPFVertex *curVertex = &(*i);
        while (curVertex != NULL)
        {
            EROList.push_back(curVertex->vertexId.getInt());
            curVertex = curVertex->parent;
        }
    }
    return EROList;
}



void OspfTe::CspfBuildSPT(const FlowSpecObj_t& fspec, CSPFVertexVector &candidatesList)
{
    double shortestDist = LS_INFINITY;

    if (!candidatesList.empty())
    {
        // Find shortest distance to root, then find which vertex has it (2nd loop)
        // FIXME could be done in one step!
        CSPFVertexVector::iterator vertexIter;
        CSPFVertex vertex;
        for (vertexIter = candidatesList.begin(); vertexIter != candidatesList.end(); vertexIter++)
        {
            vertex = (*vertexIter);
            if (vertex.distanceToRoot < shortestDist)
                shortestDist = vertex.distanceToRoot;
        }
        for (vertexIter = candidatesList.begin(); vertexIter != candidatesList.end(); vertexIter++)
        {
            vertex = (*vertexIter);
            if (vertex.distanceToRoot == shortestDist)
                break;
        }

        // Add to Tree
        CSPFVertex vertexW;
        vertexW.distanceToRoot = shortestDist;
        vertexW.parent = vertex.parent;
        vertexW.vertexId = vertex.vertexId;
        cshortestPathTree.push_back(vertexW);

        candidatesList.erase(vertexIter);

        TEAddCandidates(fspec, candidatesList);
    }
}


IPADDRVector OspfTe::CalculateERO(const IPAddress& dest,
                                  const FlowSpecObj_t& fspec, double& outMetric)
{
    Enter_Method("CalculateERO()");

    // Get Latest TED
    updateTED();

    // Reset the Constraint Shortest Path Tree
    cshortestPathTree.clear();

    // FIXME this is a workaround -- vertices in candidatesList will have
    // pointers to each other so we have to prevent reallocation.
    // Here we just reserve enough so that realloc is never necessary.
    cshortestPathTree.reserve(ted.size());

    CSPFVertexVector candidates;
    CSPFVertex rootVertex;
    rootVertex.distanceToRoot = 0;
    rootVertex.parent = NULL;
    rootVertex.vertexId = routerId;
    candidates.push_back(rootVertex);

    CspfBuildSPT(fspec, candidates);
    return doCalculateERO(dest, cshortestPathTree, outMetric);
}

/******************************************************************************
*This is a tricky part of the simulation since only computation part of OSPF is built
*This force the immediate database convergence
******************************************************************************/
void OspfTe::updateTED()
{
    // copy the full table
    ted = TED::getGlobalInstance()->getTED();
}

void OspfTe::printTED()
{
    ev << "*** OSPF TED:\n";
    TELinkStateVector::iterator i;
    for (i = ted.begin(); i != ted.end(); i++)
    {
        const TELinkState& linkstate = *i;
        ev << "Adv Router: " << linkstate.advrouter << "\n";
        ev << "Link Id (neighbour IP): " << linkstate.linkid << "\n";
        ev << "Max Bandwidth: " << linkstate.MaxBandwith << "\n";
        ev << "Metric: " << linkstate.metric << "\n\n";
    }
}

IPADDRVector OspfTe::CalculateERO(const IPAddress& dest,
                                  const simple_link_tVector &links,
                                  const FlowSpecObj_t& old_fspec,
                                  const FlowSpecObj_t& new_fspec,
                                  double &outTotalDelay)
{
    Enter_Method("CalculateERO()");

    // Get Latest TED
    updateTED();

    // Reset the Constraint Shortest Path Tree
    cshortestPathTree.clear();

    // FIXME this is a workaround -- vertices in candidatesList will have
    // pointers to each other so we have to prevent reallocation.
    // Here we just reserve enough so that realloc is never necessary.
    cshortestPathTree.reserve(ted.size());

    CSPFVertexVector candidates;
    CSPFVertex rootVertex;
    rootVertex.distanceToRoot = 0;
    rootVertex.parent = NULL;
    rootVertex.vertexId = routerId;
    candidates.push_back(rootVertex);

    CspfBuildSPT(links, old_fspec, new_fspec, candidates);
    return doCalculateERO(dest, cshortestPathTree, outTotalDelay);
}

void OspfTe::CspfBuildSPT(const simple_link_tVector& links,
                          const FlowSpecObj_t& old_fspec,
                          const FlowSpecObj_t& new_fspec,
                          CSPFVertexVector& candidatesList)
{
    // FIXME this function seems to be exactly the same as the other similar one,
    // except for the TEAddCandidates() call at the end!!! eliminate!
    double shortestDist = LS_INFINITY;

    if (!candidatesList.empty())
    {
        CSPFVertexVector::iterator vertexIter;
        CSPFVertex vertex;

        // Find shortest distance to root, then find which vertex has it (2nd loop)
        // FIXME could be done in one step! FIXME same code as in similar function...
        for (vertexIter = candidatesList.begin(); vertexIter != candidatesList.end(); vertexIter++)
        {
            vertex = (*vertexIter);
            if (vertex.distanceToRoot < shortestDist)
                shortestDist = vertex.distanceToRoot;
        }

        for (vertexIter = candidatesList.begin(); vertexIter != candidatesList.end(); vertexIter++)
        {
            vertex = (*vertexIter);
            if (vertex.distanceToRoot == shortestDist)
                break;
        }

        // Add to Tree
        CSPFVertex vertexW;
        vertexW.distanceToRoot = shortestDist;
        vertexW.parent = vertex.parent;
        vertexW.vertexId = vertex.vertexId;

        cshortestPathTree.push_back(vertexW);
        candidatesList.erase(vertexIter);

        TEAddCandidates(links, old_fspec, new_fspec, candidatesList);
    }
}

void OspfTe::TEAddCandidates(const simple_link_tVector& links,
                             const FlowSpecObj_t& old_fspec,
                             const FlowSpecObj_t& new_fspec,
                             CSPFVertexVector& candidatesList)
{
    CSPFVertex *vertexV = &cshortestPathTree.back();

    // FIXME this is a workaround -- vertices in candidatesList will have
    // pointers to each other so we have to prevent reallocation.
    // Here we just reserve enough so that realloc is never necessary.
    candidatesList.reserve(ted.size());

    TELinkStateVector::iterator tedIter;
    for (tedIter = ted.begin(); tedIter != ted.end(); tedIter++)
    {
        const TELinkState& linkstate = (*tedIter);

        CSPFVertex sptVertex;

        // Other ends of the links to V only
        if (linkstate.advrouter != vertexV->vertexId)
            continue;

        // Skip if it's in ConstrainedShortestPathTree already
        bool found = false;
        CSPFVertexVector::iterator sptIter;
        for (sptIter = cshortestPathTree.begin(); sptIter != cshortestPathTree.end(); sptIter++)
        {
            sptVertex = (*sptIter);
            if (linkstate.linkid == sptVertex.vertexId)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;

        // Satisfy the resource constraint of BW only
        bool sharedLink = false;
        simple_link_tVector::const_iterator iterS;
        for (iterS = links.begin(); iterS != links.end(); iterS++)
        {
            simple_link_t aLink;
            aLink = (*iterS);
            if (aLink.advRouter == linkstate.advrouter.getInt() &&
                aLink.id == linkstate.linkid.getInt())
            {
                sharedLink = true;
                break;
            }

        }
        if (sharedLink)
        {
            if (linkstate.UnResvBandwith[0] + old_fspec.req_bandwidth < new_fspec.req_bandwidth)
                continue;
        }
        else
        {
            if (linkstate.UnResvBandwith[0] < new_fspec.req_bandwidth)
                continue;
        }

        // Normal Dijkstra Algorithm for adding candidate
        CSPFVertex vertexW;
        vertexW.vertexId = linkstate.linkid;
        vertexW.distanceToRoot = vertexV->distanceToRoot + linkstate.metric;
        vertexW.parent = vertexV;

        found = false;
        if (!candidatesList.empty())
        {
            for (sptIter = candidatesList.begin(); sptIter != candidatesList.end(); sptIter++)
            {
                sptVertex = (*sptIter);

                if (sptVertex.vertexId == vertexW.vertexId)
                {
                    found = true;
                    break;
                }
            }
        }

        // Relaxation
        if (found)
        {
            if (sptVertex.distanceToRoot > vertexW.distanceToRoot)
            {
                sptVertex.distanceToRoot = vertexW.distanceToRoot;
                sptVertex.parent = vertexV;
                // TECalculateNextHops( vertexW, vertexV, *linkstate);
            }
        }
        else
        {
            candidatesList.push_back(vertexW);
        }
    }

    CspfBuildSPT(links, old_fspec, new_fspec, candidatesList);

}
