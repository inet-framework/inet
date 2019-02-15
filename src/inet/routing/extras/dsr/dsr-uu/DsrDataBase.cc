/*****************************************************************************
 *
 * Copyright (C) 2014 Malaga University.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Alfonso Ariza Quintana.<aarizaq@uma.ea>
 *
 *****************************************************************************/

#include <algorithm>    // std::equal
#include "inet/routing/extras/dsr/dsr-uu/DsrDataBase.h"

#define VALID 0

namespace inet {

namespace inetmanet {


bool DsrDataBase::getPaths(const L3Address &addr, std::vector<PathCacheRoute> &result, std::vector<PathCacheCost>&resultCost)
{
    result.clear();
    resultCost.clear();
    auto it = pathsCache.find(addr);
    if (it == pathsCache.end())
        return false;
    simtime_t now = simTime();

    if (it->second.empty()) // check active routes
    {
        pathsCache.erase(it);
        return false;
    }
    for (auto itPaths = it->second.begin();  itPaths != it->second.end();)
    {
        // check timers
        if (now >= itPaths->getExpires())
            itPaths = it->second.erase(itPaths);
        else
        {
            result.push_back(itPaths->route);
            resultCost.push_back(itPaths->vector_cost);
            ++itPaths;
        }
    }
    if (it->second.empty()) // check active routes
    {
        pathsCache.erase(it);
        return false;
    }
    if (result.empty())
        return false;
    return true;
}

bool DsrDataBase::getPath(const L3Address &dest, PathCacheRoute &route, double &cost,unsigned int timeout)
{
    route.clear();
    cost = 1e100;

    bool found = false;

    auto it = pathsCache.find(dest);
    if (it == pathsCache.end())
        return found;
    if (it->second.empty()) // check active routes
    {
        pathsCache.erase(it);
        return found;
    }

    int position = -1;
    int cont = -1;
    simtime_t now = simTime();
    simtime_t expire;

    for (auto itPaths = it->second.begin();  itPaths != it->second.end();)
    {
        // check timers
        if (now >= itPaths->getExpires())
        {
            itPaths = it->second.erase(itPaths);
            continue;
        }
        cont++;
        if (itPaths->status == VALID)
        {
            double costPath = 0;
            if (!itPaths->vector_cost.empty())
            {
                itPaths->vector_cost[0] = cost;
                for (unsigned int i = 0; i< itPaths->vector_cost.size(); i++)
                {
                    costPath += itPaths->vector_cost[i];
                }
            }
            else
            {
                costPath = itPaths->route.size()+1;
            }
            if (!found || (cost > costPath) || (cost == costPath && expire < itPaths->getExpires()))
            {
                position = cont;
                route = itPaths->getRoute();
                cost = costPath;
                expire = itPaths->getExpires();
            }
            found = true;
        }
        ++itPaths;
    }

    if (it->second.empty()) // check active routes
    {
        pathsCache.erase(it);
        return found;
    }

    if (timeout > 0 && position >= 0)
        it->second[position].expires = simTime() + (((double)timeout)/1000000.0);

    return found;
}


bool DsrDataBase::getPathCosVect(const L3Address &dest, PathCacheRoute &route, PathCacheCost& costVect,double &cost,unsigned int timeout)
{
    route.clear();
    costVect.clear();
    cost = 1e100;


    bool found = false;

    auto it = pathsCache.find(dest);
    if (it == pathsCache.end())
        return found;
    if (it->second.empty()) // check active routes
    {
        pathsCache.erase(it);
        return found;
    }

    int position = -1;
    int cont = -1;
    simtime_t now = simTime();
    simtime_t expire;

    for (auto itPaths = it->second.begin();  itPaths != it->second.end();)
    {
        // check timers
        if (now >= itPaths->getExpires())
        {
            itPaths = it->second.erase(itPaths);
            continue;
        }

        cont++;
        if (itPaths->status == VALID)
        {
            double costPath = 0;
            if (!itPaths->vector_cost.empty())
            {
                for (unsigned int i = 0; i< itPaths->vector_cost.size(); i++)
                {
                    costPath += itPaths->vector_cost[i];
                }
            }
            else
            {
                costPath = itPaths->route.size()+1;
            }
            if (!found || cost > costPath || (cost == costPath && expire < itPaths->getExpires()))
            {
                position = cont;
                route = itPaths->getRoute();
                costVect = itPaths->getCostVector();
                cost = costPath;
                expire = itPaths->getExpires();
            }
            found = true;
        }
        ++itPaths;
    }

    if (it->second.empty()) // check active routes
    {
        pathsCache.erase(it);
        return found;
    }

    if (timeout>0 && position >=0)
        it->second[position].expires = simTime() + (((double)timeout)/1000000.0);

    return found;
}


void DsrDataBase::setPath(const L3Address &dest,const PathCacheRoute &route,const PathCacheCost& costVect,const double &cost, int status, const unsigned int &timeout)
{
    auto it = pathsCache.find(dest);
    simtime_t now = simTime();
    if (it == pathsCache.end())
    {
        pathsCache[dest] = PathsToDestination();
        it = pathsCache.find(dest);
    }
    // check if route exist
    for (auto itPaths = it->second.begin();  itPaths != it->second.end();)
    {
        // check timers
        if (route == itPaths->route)
        {
            itPaths->vector_cost = costVect;
            itPaths->cost = cost;
            itPaths->status = status;
            itPaths->expires = simTime() + (((double)timeout)/1000000.0);
            // actualize and return
            return;
        }
        else if (now >= itPaths->getExpires())
        {
            itPaths = it->second.erase(itPaths);
        }
        else
            ++itPaths;
    }
    // insert at the end
    Path path;
    path.cost = cost;
    path.vector_cost = costVect;
    path.status = status;
    path.route = route;
    path.expires = simTime() + (((double)timeout)/1000000.0);
    it->second.push_back(path);
}

void DsrDataBase::setPathStatus(const L3Address &dest,const PathCacheRoute &route, int status)
{
    auto it = pathsCache.find(dest);
    simtime_t now = simTime();
    if (it == pathsCache.end())
        return;

    // check if route exist
    for (auto itPaths = it->second.begin();  itPaths != it->second.end();)
    {
        // check timers
        if (route == itPaths->route)
        {
            itPaths->status = status;
            // actualize and return
            return;
        }
        else if (now >= itPaths->getExpires())
        {
            itPaths = it->second.erase(itPaths);
        }
        else
            ++itPaths;
    }
    if (it->second.empty())
        pathsCache.erase(it);
}

void DsrDataBase::setPathsTimer(const L3Address &dest,const PathCacheRoute &route, const unsigned int &timeout)
{
    auto it = pathsCache.find(dest);
    simtime_t now = simTime();

    if (timeout == 0)
        return;
    if (it == pathsCache.end())
        return;

    // check if route exist
    for (auto itPaths = it->second.begin();  itPaths != it->second.end();)
    {
        // check timers
        if (route == itPaths->route)
        {
            itPaths->expires = simTime() + (((double)timeout)/1000000.0);
            // actualize and return
            return;
        }
        else if (now >= itPaths->getExpires())
        {
            itPaths = it->second.erase(itPaths);
        }
        else
            ++itPaths;
    }
    if (it->second.empty())
         pathsCache.erase(it);
}


void DsrDataBase::deleteAddress(const L3Address &dest)
{
    auto it = pathsCache.find(dest);
    if (it != pathsCache.end())
        pathsCache.erase(it);
}

void DsrDataBase::erasePathWithNode(const L3Address &dest)
{
    if (pathsCache.empty())
        return;
    deleteAddress(dest);
    simtime_t now = simTime();
    std::vector<PathsDataBase::iterator> erased;

    for (auto itMap = pathsCache.begin();itMap != pathsCache.end();++itMap)
    {
        for(auto itPaths = itMap->second.begin(); itPaths != itMap->second.end();)
        {
            if (now >= itPaths->getExpires())
            {
                itPaths = itMap->second.erase(itPaths);
                continue;
            }
            auto pos = std::find(itPaths->route.begin(),itPaths->route.end(),dest);
            if (pos != itPaths->route.end())
            {
                itPaths = itMap->second.erase(itPaths);
                continue;
            }
            ++itPaths;
        }
        if (itMap->second.empty())
            erased.push_back(itMap);
    }
    while(!erased.empty())
    {
        pathsCache.erase(erased.back());
        erased.pop_back();
    }
}


void DsrDataBase::erasePathWithLink(const L3Address &addr1,const L3Address &addr2, bool eraseFirst,bool bidirectional)
{
    if (pathsCache.empty())
        return;
    simtime_t now = simTime();
    L3Address sequence[] = {addr1,addr2};
    L3Address sequenceRev[] = {addr2,addr1};
    std::vector<PathsDataBase::iterator> erased;

    if (rootNode == addr1)
    {
        auto itMap = pathsCache.find(addr2);
        if (itMap != pathsCache.end())
        {
            for(auto itPaths = itMap->second.begin(); itPaths != itMap->second.end();)
            {
                if (now >= itPaths->getExpires())
                {
                    itPaths = itMap->second.erase(itPaths);
                    continue;
                }
                if (itPaths->route.empty())
                {
                    itPaths = itMap->second.erase(itPaths);
                    continue;
                }
                ++itPaths;
            }
            if (itMap->second.empty())
                pathsCache.erase(itMap);
        }

        for (itMap = pathsCache.begin();itMap != pathsCache.end();++itMap)
        {
            int contador = 0;
            for(auto itPaths = itMap->second.begin(); itPaths != itMap->second.end();)
            {
                contador++;
                if (now >= itPaths->getExpires())
                {
                    itPaths = itMap->second.erase(itPaths);
                    continue;
                }
                if (!itPaths->route.empty() && itPaths->route[0] == addr2)
                {
                    itPaths = itMap->second.erase(itPaths);
                    continue;
                }
                ++itPaths;
            }
            if (itMap->second.empty())
                erased.push_back(itMap);
        }
        while(!erased.empty())
        {
            pathsCache.erase(erased.back());
            erased.pop_back();
        }
        return;
    }

    for (auto itMap = pathsCache.begin();itMap != pathsCache.end();++itMap)
    {
        for(auto itPaths = itMap->second.begin(); itPaths != itMap->second.end();)
        {
            if (now >= itPaths->getExpires())
            {
                itPaths = itMap->second.erase(itPaths);
                continue;
            }
            //
            // special case, if the destination address is addr2 check if the last node in the route is addr1
            //
            if (itMap->first == addr2 && (!itPaths->route.empty() && itPaths->route.back() == addr1))
            {
                itPaths = itMap->second.erase(itPaths);
                continue;
            }
            //
            // special case, if eraseFirst is true delete paths where the first node is addr2
            //
            if (eraseFirst)
            {
                if ((itPaths->route.empty() && itMap->first == addr2) || (!itPaths->route.empty() && itPaths->route[0] == addr2))
                {
                    itPaths = itMap->second.erase(itPaths);
                    continue;
                }
            }
            auto pos = std::search(itPaths->route.begin(),itPaths->route.end(),sequence,sequence+1);
            if (pos != itPaths->route.end())
            {
                itPaths = itMap->second.erase(itPaths);
                continue;
            }
            if (bidirectional)
            {
                if (itMap->first == addr1 && (!itPaths->route.empty() && itPaths->route.back() == addr2))
                {
                    itPaths = itMap->second.erase(itPaths);
                    continue;
                }
                auto pos = std::search(itPaths->route.begin(),itPaths->route.end(),sequenceRev,sequenceRev+1);
                if (pos != itPaths->route.end())
                {
                    itPaths = itMap->second.erase(itPaths);
                    continue;
                }

            }
            ++itPaths;
        }
        if (itMap->second.empty())
            erased.push_back(itMap);
    }
    while(!erased.empty())
    {
        pathsCache.erase(erased.back());
        erased.pop_back();
    }
}




///
/// Link cache data base
///


DsrDataBase::DsrDataBase()
{
    routeCache.clear();
    cleanLinkArray();
    pathsCache.clear();
}

void DsrDataBase::cleanLinkChacheData()
{
    routeCache.clear();
}

DsrDataBase::~DsrDataBase()
{
    // TODO Auto-generated destructor stub
    routeCache.clear();
    pathsCache.clear();
    cleanLinkArray();
}


DsrDataBase::DijkstraShortest::State::State()
{
    idPrev = L3Address();
    label = tent;
    edge = nullptr;
}

DsrDataBase::DijkstraShortest::State::State(const double  &costData)
{
    idPrev = L3Address();
    label = tent;
    edge = nullptr;
    costAdd = costData;
}

void DsrDataBase::DijkstraShortest::State::setCostVector(const double &costData)
{
    costAdd = costData;
}

void DsrDataBase::cleanLinkArray()
{
    for (auto it=linkArray.begin();it!=linkArray.end();it++)
        while (!it->second.empty())
        {
            delete it->second.back();
            it->second.pop_back();
        }
    linkArray.clear();
}

void DsrDataBase::addEdge (const L3Address & originNode, const L3Address & last_node,double cost, const unsigned int &exp)
{
    // invalidate data base
    routeMap.clear();
    auto it = linkArray.find(originNode);
    if (it!=linkArray.end())
    {
         for (unsigned int i=0;i<it->second.size();i++)
         {
             if (last_node == it->second[i]->last_node_)
             {
                  it->second[i]->costAdd =cost;
                  it->second[i]->expires = simTime() + (((double)exp)/1000000.0);
                  return;
             }
         }
    }
    DsrDataBase::DijkstraShortest::Edge *link = new DsrDataBase::DijkstraShortest::Edge;
    // The last hop is the interface in which we have this neighbor...
    link->last_node_ = last_node;
    // Also record the link delay and quality..
    link->costAdd = cost;
    link->expires = simTime() + (((double)exp)/1000000.0);
    linkArray[originNode].push_back(link);
}

void DsrDataBase::deleteEdge (const L3Address & originNode, const L3Address & last_node,bool bidirectional)
{
    if (linkArray.empty())
        return;

    // invalidate data base
    routeMap.clear();

    auto it = linkArray.find(originNode);
    if (it!=linkArray.end())
    {
         for (auto itAux= it->second.begin(); itAux != it->second.end();++itAux)
         {
             if (last_node == (*itAux)->last_node_)
             {
                 it->second.erase(itAux);
                  return;
             }
         }
    }
    if (bidirectional)
    {
        it = linkArray.find(last_node);
        if (it!=linkArray.end())
        {
             for (auto itAux= it->second.begin(); itAux != it->second.end();++itAux)
             {
                 if (originNode == (*itAux)->last_node_)
                 {
                     it->second.erase(itAux);
                      return;
                 }
             }
        }
    }
}

void DsrDataBase::setRoot(const L3Address & dest_node)
{
    if (rootNode != dest_node)
    {
        rootNode = dest_node;
        // invalidate routes
        routeMap.clear();
    }
}

void DsrDataBase::run(const L3Address &target)
{
    std::multiset<DsrDataBase::DijkstraShortest::SetElem> heap;
    routeMap.clear();
    if (linkArray.empty())
        return;

    auto it = linkArray.find(rootNode);
    if (it==linkArray.end())
    {
        return;
    }

    DsrDataBase::DijkstraShortest::State state(0);
    state.label = tent;
    routeMap[rootNode] = state;


    DsrDataBase::DijkstraShortest::SetElem elem;
    elem.iD = rootNode;
    elem.costAdd = 0;
    heap.insert(elem);

    while (!heap.empty())
    {

        auto itHeap = heap.begin();
        // search if exist several with the same cost and extract randomly one
        std::vector<std::multiset<DsrDataBase::DijkstraShortest::SetElem>::iterator> equal;
        while(1)
        {
            equal.push_back(itHeap);
            auto itHeap3 = itHeap;
            ++itHeap3;
            if (itHeap3 == heap.end())
                break;

            if (itHeap3->costAdd > itHeap->costAdd)
                break;
            itHeap = itHeap3;
        }
        int numeq = equal.size()-1;
        int val = numeq > 0?getEnvir()->getRNG(0)->intRand(numeq):0;
        itHeap = equal[val];
        equal.clear();

        //
        DsrDataBase::DijkstraShortest::SetElem elem = *itHeap;
        heap.erase(itHeap);

        auto it = routeMap.find(elem.iD);
        if (it==routeMap.end())
            throw cRuntimeError("node not found in routeMap %s",elem.iD.toIpv4().str().c_str());

        if ((it->second).label == perm)
            continue;

        (it->second).label = perm;
        if (target == elem.iD)
            return;

        auto linkIt=linkArray.find(elem.iD);

        for (auto itCon = linkIt->second.begin() ; itCon != linkIt->second.end();)
        {
            // first check if link is valid
            if ((*itCon)->expires <= simTime())
            {
                itCon = linkIt->second.erase(itCon);
                continue;
            }
            DsrDataBase::DijkstraShortest::Edge* current_edge = *itCon;
            auto itNext = routeMap.find(current_edge->last_node_);
            if (itNext != routeMap.end() && itNext->second.label == perm)
            {
                ++itCon;
                continue;
            }
            double costAdd = current_edge->costAdd + (it->second).costAdd;
            if (itNext == routeMap.end())
            {
                DsrDataBase::DijkstraShortest::State state;
                state.idPrev = elem.iD;
                state.costAdd = costAdd;
                state.label = tent;
                state.edge = current_edge;

                routeMap[current_edge->last_node_] = state;
                DsrDataBase::DijkstraShortest::SetElem newElem;
                newElem.iD = current_edge->last_node_;
                newElem.costAdd = costAdd;
                heap.insert(newElem);
            }
            else
            {
                if (costAdd < itNext->second.costAdd)
                {
                    itNext->second.costAdd = costAdd;
                    itNext->second.idPrev = elem.iD;
                    itNext->second.edge = current_edge;

                    // actualize heap
                    DsrDataBase::DijkstraShortest::SetElem newElem;
                    newElem.iD = current_edge->last_node_;
                    newElem.costAdd = costAdd;

                    heap.insert(newElem);
                }
            }
            ++itCon;
        }
    }
}

bool DsrDataBase::getRoute(const L3Address &nodeId,PathCacheRoute &pathNode, unsigned int exp)
{

    if (routeMap.empty())
        run(); // built routes

    auto it = routeMap.find(nodeId);
    if (it==routeMap.end())
        return false;

    PathCacheRoute path;
    L3Address currentNode = nodeId;
    pathNode.clear();
    simtime_t now = simTime();
    while (currentNode!=rootNode)
    {
        path.push_back(currentNode);
        currentNode = it->second.idPrev;
        if (it->second.edge->expires <= now)
        {
            routeMap.clear();
            return  getRoute(nodeId,pathNode,exp);// the new will be rebuilt with old links deleted
        }
        if (exp > 0)
            it->second.edge->expires = now + ((double)exp/1000000.0);
        it = routeMap.find(currentNode);
        if (it==routeMap.end())
            throw cRuntimeError("error in data routeMap");
    }
    for (unsigned int i = path.size()-1 ; i > 0 ; i--)
        pathNode.push_back(path[i]);
    return true;
}


bool DsrDataBase::getRouteCost(const L3Address &nodeId, PathCacheRoute &pathNode, double &pathCost,unsigned int exp)
{
    if (routeMap.empty())
        run(); // built routes

    auto it = routeMap.find(nodeId);
    if (it==routeMap.end())
        return false;

    PathCacheRoute path;
    PathCacheCost costs;
    L3Address currentNode = nodeId;
    pathNode.clear();

    pathCost = it->second.costAdd;
    simtime_t now = simTime();
    while (currentNode!=rootNode)
    {
        path.push_back(currentNode);
        currentNode = it->second.idPrev;
        if (it->second.edge->expires <= now)
        {
            routeMap.clear();
            return  getRouteCost(nodeId,pathNode,pathCost,exp);// the new will be rebuilt with old links deleted
        }
        if (exp > 0)
            it->second.edge->expires = now + ((double)exp/1000000.0);
        it = routeMap.find(currentNode);
        if (it==routeMap.end())
            throw cRuntimeError("error in data routeMap");
    }
    for (unsigned int i = 0 ; i < path.size(); i++)
        pathNode.push_back(path[i]);
    return true;
}


void DsrDataBase::purgePathCache()
{
    std::vector<PathsDataBase::iterator> erased;
    for (auto itmap = pathsCache.begin();itmap != pathsCache.end();++itmap)
    {
        simtime_t now = simTime();
        for (auto itPaths = itmap->second.begin();  itPaths != itmap->second.end();)
        {
            // check timers
            if (now >= itPaths->getExpires())
                itPaths = itmap->second.erase(itPaths);
            else
                ++itPaths;
        }
        if (itmap->second.empty())
        {
            erased.push_back(itmap);
        }
    }
    while(!erased.empty())
    {
        pathsCache.erase(erased.back());
        erased.pop_back();
    }
}

} // namespace inetmanet

} // namespace inet
