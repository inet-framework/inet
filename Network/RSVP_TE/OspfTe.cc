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


Define_Module(OspfTe);


void OspfTe::initialize(int stage)
{
    ev << "OSPF enters init stage " << stage << endl;
    switch(stage)
    {
        case 0:
            //Get the local address
            //Get the ted
            //Get the IP Routing component
            local_addr = IPAddress(par("local_addr").stringValue()).getInt();
            break;
    }

    // to invoke handleMessage() when we start --
    scheduleAt(simTime(), new cMessage());
}

void OspfTe::handleMessage(cMessage *msg)
{
    // one-shot handleMessage()
    // FIXME should really be done from another init stage
    if (!msg->isSelfMessage())
        error("Message arrived -- OSPF/TE doesn't process messages, it is used via direct method calls");
    delete msg;

    ev << "OSPF/TE: I am starting\n";

    bool IsIR =false;
    cModule* curmod = this;
    for (curmod = parentModule(); curmod != NULL; curmod = curmod->parentModule())
    {
        if (curmod->hasPar("isIR"))  // FIXME why not use ancestor par of *this* module? Andras
        {
            IsIR = curmod->par("isIR").boolValue();
            break;
        }
    }

    if (IsIR)
    {
        updateTED();
    }
}


/*******************************************************************************
*                           OSPF-TE (CSPF)ROUTING
*
*******************************************************************************/
void OspfTe::TEAddCandidates(const FlowSpecObj_t& fspec,
                             std::vector<CSPFVertex>& CandidatesList)
{
    //CSPFVertex VertexVV = *(CShortestPathTree.back());
    CSPFVertex* VertexV = &CShortestPathTree.back();

    // FIXME this is a workaround -- vertices in CandidatesList will have
    // pointers to each other so we have to prevent reallocation.
    // Here we just reserve enough so that realloc is never necessary.
    CandidatesList.reserve(ted.size());

    // for each link
    std::vector<TELinkState>::iterator tedIter;
    for (tedIter=ted.begin(); tedIter!=ted.end(); tedIter++)
    {
        const TELinkState& linkstate = (*tedIter);

        //Other ends of the links to V only
        if (linkstate.advrouter!= VertexV->VertexId)
            continue;

        //Not in the ConstrainedShortestPathTree only
        bool IsFound =false;
        std::vector<CSPFVertex>::iterator sptIter;
        CSPFVertex sptVertex;
        for (sptIter = CShortestPathTree.begin();sptIter !=CShortestPathTree.end();sptIter++)
        {
            sptVertex = (*sptIter);
            if (linkstate.linkid == sptVertex.VertexId)
            {
               IsFound = true;
               break;
            }
        }
        if (IsFound)
             continue;

        //Satisfy the resource constraint of BW only
        if ((linkstate.UnResvBandwith[0]) < (fspec.req_bandwidth))
             continue;

        //Normal Dijkstra Algorithm for adding candidate
        CSPFVertex VertexW;
        VertexW.VertexId = (linkstate.linkid);
        VertexW.DistanceToRoot = (VertexV->DistanceToRoot) + (linkstate.metric);
        VertexW.Parent = VertexV;

        IsFound = false;
        if (!CandidatesList.empty())
        {
            for (sptIter=CandidatesList.begin(); sptIter != CandidatesList.end(); sptIter++)
            {
               sptVertex = (*sptIter);

               if (sptVertex.VertexId == VertexW.VertexId)
               {
                   IsFound = true;
                   break;
               }
            }
        }

        //Relaxation
        if (IsFound)
        {
            if (sptVertex.DistanceToRoot > VertexW.DistanceToRoot)
            {
                sptVertex.DistanceToRoot = VertexW.DistanceToRoot;
                sptVertex.Parent = VertexV;
                //TECalculateNextHops( VertexW, VertexV, *linkstate);
            }
        }
        else
        {
            CandidatesList.push_back(VertexW);
        }
    }

    CspfBuildSPT(fspec, CandidatesList);
}

std::vector<int>
OspfTe::doCalculateERO(const IPAddress& dest,
                       std::vector<CSPFVertex>& CandidatesList,
                       double& outTotalMetric)
{
    std::vector<int> EROList;
    if (!CandidatesList.empty())
    {
        // find dest among candidates
        std::vector<CSPFVertex>::iterator i;
        for (i=CandidatesList.begin(); i!=CandidatesList.end(); i++)
            if ((*i).VertexId.isEqualTo(dest))
                break;

        // if not found -- do what??? FIXME
        if (i==CandidatesList.end())
            error("doCalculateERO(): %s not found", dest.getString());

        // totalMetric is its distance to root
        outTotalMetric = (*i).DistanceToRoot;

        // insert all parents up to the root into EROList
        CSPFVertex *curVertex = &(*i);
        while (curVertex!=NULL)
        {
            EROList.push_back(curVertex->VertexId.getInt());
            curVertex = curVertex->Parent;
        }
    }
    return EROList;
}



void OspfTe::CspfBuildSPT(const FlowSpecObj_t& fspec,
                          std::vector<CSPFVertex>& CandidatesList)
{
    double shortestDist = OSPFType::LSInfinity;

    if (!CandidatesList.empty())
    {
         // Find shortest distance to root, then find which vertex has it (2nd loop)
         // FIXME could be done in one step!
         std::vector<CSPFVertex>::iterator vertexIter;
         CSPFVertex vertex;
         for (vertexIter=CandidatesList.begin(); vertexIter!=CandidatesList.end(); vertexIter++)
         {
             vertex = (*vertexIter);
             if (vertex.DistanceToRoot < shortestDist)
                 shortestDist = vertex.DistanceToRoot;
         }
         for (vertexIter=CandidatesList.begin(); vertexIter!=CandidatesList.end(); vertexIter++)
         {
             vertex = (*vertexIter);
             if (vertex.DistanceToRoot == shortestDist)
                 break;
         }

         //Add to Tree
         CSPFVertex VertexW;
         VertexW.DistanceToRoot = shortestDist;
         VertexW.Parent = vertex.Parent;
         VertexW.VertexId = vertex.VertexId;
         CShortestPathTree.push_back(VertexW);

         CandidatesList.erase(vertexIter);

         TEAddCandidates(fspec, CandidatesList);
    }
}


std::vector<int> OspfTe::CalculateERO(const IPAddress& dest,
                                      const FlowSpecObj_t& fspec,
                                      double& outMetric)
{
    Enter_Method("CalculateERO()");

    //Get Latest TED
    updateTED();

    //Reset the Constraint Shortest Path Tree
    CShortestPathTree.clear();

    // FIXME this is a workaround -- vertices in CandidatesList will have
    // pointers to each other so we have to prevent reallocation.
    // Here we just reserve enough so that realloc is never necessary.
    CShortestPathTree.reserve(ted.size());

    std::vector<CSPFVertex> candidates;
    CSPFVertex rootVertex;
    rootVertex.DistanceToRoot =0;
    rootVertex.Parent = NULL;
    rootVertex.VertexId = IPAddress(local_addr);
    candidates.push_back(rootVertex);

    CspfBuildSPT(fspec, candidates);
    return doCalculateERO(dest, CShortestPathTree, outMetric);
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
    ev << "*****************OSPF TED *****************\n";
    std::vector<TELinkState>::iterator i;
    for (i=ted.begin(); i!=ted.end(); i++)
    {
         const TELinkState& linkstate = *i;
         ev << "Adv Router: " << linkstate.advrouter.getString() << "\n";
         ev << "Link Id (neighbour IP): " << linkstate.linkid.getString() << "\n";
         ev << "Max Bandwidth: " << linkstate.MaxBandwith << "\n";
         ev << "Metric: " << linkstate.metric << "\n\n";
    }
}

std::vector<int> OspfTe::CalculateERO(const IPAddress& dest,
                                      const std::vector<simple_link_t>& links,
                                      const FlowSpecObj_t& old_fspec,
                                      const FlowSpecObj_t& new_fspec,
                                      double& outTotalDelay)
{
    Enter_Method("CalculateERO()");

    //Get Latest TED
    updateTED();

    //Reset the Constraint Shortest Path Tree
    CShortestPathTree.clear();

    // FIXME this is a workaround -- vertices in CandidatesList will have
    // pointers to each other so we have to prevent reallocation.
    // Here we just reserve enough so that realloc is never necessary.
    CShortestPathTree.reserve(ted.size());

    std::vector<CSPFVertex> candidates;
    CSPFVertex rootVertex;
    rootVertex.DistanceToRoot =0;
    rootVertex.Parent = NULL;
    rootVertex.VertexId = IPAddress(local_addr);
    candidates.push_back(rootVertex);

    CspfBuildSPT(links, old_fspec, new_fspec, candidates);
    return doCalculateERO(dest, CShortestPathTree, outTotalDelay);
}

void OspfTe::CspfBuildSPT(const std::vector<simple_link_t>& links,
                          const FlowSpecObj_t& old_fspec,
                          const FlowSpecObj_t& new_fspec,
                          std::vector<CSPFVertex>& CandidatesList)
{
    // FIXME this function seems to be exactly the same as the other similar one,
    // except for the TEAddCandidates() call at the end!!! eliminate!
    double shortestDist = OSPFType::LSInfinity;

    if (!CandidatesList.empty())
    {
         std::vector<CSPFVertex>::iterator vertexIter;
         CSPFVertex vertex;

         // Find shortest distance to root, then find which vertex has it (2nd loop)
         // FIXME could be done in one step! FIXME same code as in similar function...
         for (vertexIter=CandidatesList.begin(); vertexIter != CandidatesList.end(); vertexIter++)
         {
             vertex = (*vertexIter);
             if (vertex.DistanceToRoot < shortestDist)
                 shortestDist = vertex.DistanceToRoot;
         }

         for (vertexIter=CandidatesList.begin(); vertexIter != CandidatesList.end(); vertexIter++)
         {
             vertex = (*vertexIter);
             if (vertex.DistanceToRoot == shortestDist)
                 break;
         }

         //Add to Tree
         CSPFVertex VertexW;
         VertexW.DistanceToRoot = shortestDist;
         VertexW.Parent = vertex.Parent;
         VertexW.VertexId = vertex.VertexId;

         CShortestPathTree.push_back(VertexW);
         CandidatesList.erase(vertexIter);

         TEAddCandidates(links, old_fspec, new_fspec, CandidatesList);
    }
}

void OspfTe::TEAddCandidates(const std::vector<simple_link_t>& links,
                             const FlowSpecObj_t& old_fspec,
                             const FlowSpecObj_t& new_fspec,
                             std::vector<CSPFVertex>& CandidatesList)
{
    //CSPFVertex VertexVV = *(CShortestPathTree.back());
    CSPFVertex* VertexV = &CShortestPathTree.back();

    // FIXME this is a workaround -- vertices in CandidatesList will have
    // pointers to each other so we have to prevent reallocation.
    // Here we just reserve enough so that realloc is never necessary.
    CandidatesList.reserve(ted.size());

    std::vector<TELinkState>::iterator tedIter;
    for (tedIter=ted.begin(); tedIter != ted.end(); tedIter++)
    {
        const TELinkState& linkstate = (*tedIter);

        CSPFVertex sptVertex;

        //Other ends of the links to V only
        if (linkstate.advrouter!= VertexV->VertexId)
            continue;

        //Skip if it's in ConstrainedShortestPathTree already
        bool IsFound =false;
        std::vector<CSPFVertex>::iterator sptIter;
        for (sptIter = CShortestPathTree.begin(); sptIter!=CShortestPathTree.end(); sptIter++)
        {
            sptVertex = (*sptIter);
            if (linkstate.linkid == sptVertex.VertexId)
            {
                IsFound = true;
                break;
            }
        }
        if (IsFound)
             continue;

        //Satisfy the resource constraint of BW only
        bool sharedLink= false;
        std::vector<simple_link_t>::const_iterator iterS;
        for (iterS = links.begin(); iterS!=links.end();iterS++)
        {
            simple_link_t aLink;
            aLink = (*iterS);
            if (aLink.advRouter == linkstate.advrouter.getInt() &&
                aLink.id == linkstate.linkid.getInt())
            {
                sharedLink =true;
                break;
            }

        }
        if (sharedLink)
        {
            if (linkstate.UnResvBandwith[0]+old_fspec.req_bandwidth < new_fspec.req_bandwidth)
                continue;
        }
        else
        {
            if (linkstate.UnResvBandwith[0] < new_fspec.req_bandwidth)
                continue;
        }

        //Normal Dijkstra Algorithm for adding candidate
        CSPFVertex VertexW;
        VertexW.VertexId = (linkstate.linkid);
        VertexW.DistanceToRoot = (VertexV->DistanceToRoot) + (linkstate.metric);
        VertexW.Parent = VertexV;

        IsFound = false;
        if (!CandidatesList.empty())
        {
            for (sptIter=CandidatesList.begin(); sptIter != CandidatesList.end(); sptIter++)
            {
                sptVertex = (*sptIter);

                if (sptVertex.VertexId == VertexW.VertexId)
                {
                    IsFound = true;
                    break;
                }
            }
        }

        //Relaxation
        if (IsFound)
        {
            if (sptVertex.DistanceToRoot > VertexW.DistanceToRoot )
            {
                 sptVertex.DistanceToRoot = VertexW.DistanceToRoot;
                 sptVertex.Parent = VertexV;
                 //TECalculateNextHops( VertexW, VertexV, *linkstate);
            }
        }
        else
        {
            CandidatesList.push_back(VertexW);
        }
    }

    CspfBuildSPT(links, old_fspec,new_fspec, CandidatesList);

}








