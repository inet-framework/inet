//
// Copyright (C) 2010 Alfonso Ariza, Malaga University
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
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

#include "inet/common/DijktraKShortest.h"

#include <omnetpp.h>
#include "inet/networklayer/common/ModuleIdAddress.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include <deque>

namespace inet{

DijkstraKshortest::CostVector DijkstraKshortest::minimumCost;
DijkstraKshortest::CostVector DijkstraKshortest::maximumCost;

inline bool operator < ( const DijkstraKshortest::SetElem& x, const DijkstraKshortest::SetElem& y )
{
    for (unsigned int i =0;i<x.cost.size();i++)
    {
        switch(x.cost[i].metric)
        {
        case aditiveMin:
        case concaveMin:
            if (x.cost[i].value<y.cost[i].value)
                return true;
            break;
        case aditiveMax:
        case concaveMax:
            if (x.cost[i].value>y.cost[i].value)
                return true;
            break;
        }
    }
    return false;
}

inline bool operator < ( const DijkstraKshortest::CostVector& x, const DijkstraKshortest::CostVector& y )
{
    for (unsigned int i =0;i<x.size();i++)
    {
        switch(x[i].metric)
        {
        case aditiveMin:
        case concaveMin:
            if (x[i].value<y[i].value)
                return true;
            break;
        case aditiveMax:
        case concaveMax:
            if (x[i].value>y[i].value)
                return true;
            break;
        }
    }
    return false;
}


DijkstraKshortest::State::State()
{
    idPrev = UndefinedAddr;
    idPrevIdx=-1;
    label=tent;
}

DijkstraKshortest::State::State(const CostVector &costData)
{
    idPrev = UndefinedAddr;
    idPrevIdx=-1;
    label=tent;
    cost = costData;
}

DijkstraKshortest::State::~State() {
    cost.clear();
}

void DijkstraKshortest::State::setCostVector(const CostVector &costData)
{
    cost=costData;
}

void DijkstraKshortest::addCost (CostVector &val, const CostVector & a, const CostVector & b)
{
    val.clear();
    for (unsigned int i=0;i<a.size();i++)
    {
        Cost aux;
        aux.metric=a[i].metric;
        switch(aux.metric)
        {
            case aditiveMin:
            case aditiveMax:
                aux.value =a[i].value+b[i].value;
                break;
            case concaveMin:
                aux.value = std::min (a[i].value,b[i].value);
                break;
            case concaveMax:
                aux.value = std::max (a[i].value,b[i].value);
                break;
        }
        val.push_back(aux);
    }
}


void DijkstraKshortest::initMinAndMax()
{
    CostVector defaulCost;
    Cost costData;
    costData.metric = aditiveMin;
    costData.value = 0;
    minimumCost.push_back(costData);
    costData.metric = aditiveMin;
    costData.value = 0;
    minimumCost.push_back(costData);
    costData.metric = concaveMax;
    costData.value = 100e100;
    minimumCost.push_back(costData);
    costData.metric = aditiveMin;
    costData.value = 0;
    minimumCost.push_back(costData);

    costData.metric = aditiveMin;
    costData.value = std::numeric_limits<double>::max();
    maximumCost.push_back(costData);
    costData.metric = aditiveMin;
    costData.value = std::numeric_limits<double>::max();
    maximumCost.push_back(costData);
    costData.metric = concaveMax;
    costData.value = 0;
    maximumCost.push_back(costData);
    costData.metric = aditiveMin;
    costData.value = std::numeric_limits<double>::max();
    maximumCost.push_back(costData);
}

void DijkstraKshortest::initMinAndMaxWs()
{
   CostVector defaulCost;
   Cost costData;
   costData.metric=aditiveMin;
   costData.value=0;
   minimumCost.push_back(costData);
   costData.metric=concaveMax;
   costData.value=100e100;
   minimumCost.push_back(costData);

   costData.metric=aditiveMin;
   costData.value=10e100;
   maximumCost.push_back(costData);
   costData.metric=concaveMax;
   costData.value=0;
   maximumCost.push_back(costData);

}

void DijkstraKshortest::initMinAndMaxSw()
{

    CostVector defaulCost;
    Cost costData;
    costData.metric=aditiveMin;
    costData.value=10e100;
    maximumCost.push_back(costData);
    costData.metric=concaveMax;
    costData.value=0;
    maximumCost.push_back(costData);

    costData.metric=aditiveMin;
    costData.value=0;
    minimumCost.push_back(costData);
    costData.metric=concaveMax;
    costData.value=100e100;
    minimumCost.push_back(costData);
}


DijkstraKshortest::DijkstraKshortest(int limit)
{
    initMinAndMax();
    K_LIMITE = limit;
    resetLimits();
}

void DijkstraKshortest::cleanLinkArray()
{
    for (auto it=linkArray.begin();it!=linkArray.end();it++)
        while (!it->second.empty())
        {
            delete it->second.back();
            it->second.pop_back();
        }
    linkArray.clear();
}


DijkstraKshortest::~DijkstraKshortest()
{
    cleanLinkArray();
    kRoutesMap.clear();
}

void DijkstraKshortest::setLimits(const std::vector<double> & vectorData)
{

    limitsData =maximumCost;
    for (unsigned int i=0;i<vectorData.size();i++)
    {
        if (i>=limitsData.size()) continue;
        limitsData[i].value=vectorData[i];
    }
}



void DijkstraKshortest::addEdgeWs (const NodeId & originNode, const NodeId & last_node,double cost,double bw)
{
    auto it = linkArray.find(originNode);
    if (it!=linkArray.end())
    {
         for (unsigned int i=0;i<it->second.size();i++)
         {
             if (last_node == it->second[i]->last_node_)
             {
                  it->second[i]->Cost()=cost;
                  it->second[i]->Bandwith()=bw;
                  return;
             }
         }
    }
    EdgeWs *link = new EdgeWs;
    // The last hop is the interface in which we have this neighbor...
    link->last_node() = last_node;
    // Also record the link delay and quality..
    link->Cost()=cost;
    link->Bandwith()=bw;
    linkArray[originNode].push_back(link);
}

void DijkstraKshortest::addEdgeSw (const NodeId & originNode, const NodeId & last_node,double cost,double bw)
{
    auto it = linkArray.find(originNode);
    if (it!=linkArray.end())
    {
         for (unsigned int i=0;i<it->second.size();i++)
         {
             if (last_node == it->second[i]->last_node_)
             {
                  it->second[i]->Cost()=cost;
                  it->second[i]->Bandwith()=bw;
                  return;
             }
         }
    }
    EdgeSw *link = new EdgeSw;
    // The last hop is the interface in which we have this neighbor...
    link->last_node() = last_node;
    // Also record the link delay and quality..
    link->Cost()=cost;
    link->Bandwith()=bw;
    linkArray[originNode].push_back(link);
}

void DijkstraKshortest::addEdge (const NodeId & originNode, const NodeId & last_node,double cost,double delay,double bw,double quality)
{
    auto it = linkArray.find(originNode);
    if (it!=linkArray.end())
    {
         for (unsigned int i=0;i<it->second.size();i++)
         {
             if (last_node == it->second[i]->last_node_)
             {
                  it->second[i]->Cost()=cost;
                  it->second[i]->Delay()=delay;
                  it->second[i]->Bandwith()=bw;
                  it->second[i]->Quality()=quality;
                  return;
             }
         }
    }
    Edge *link = new Edge;
    // The last hop is the interface in which we have this neighbor...
    link->last_node() = last_node;
    // Also record the link delay and quality..
    link->Cost()=cost;
    link->Delay()=delay;
    link->Bandwith()=bw;
    link->Quality()=quality;
    linkArray[originNode].push_back(link);
}

void DijkstraKshortest::deleteEdge(const NodeId & originNode, const NodeId & last_node)
{
    DijkstraKshortest::Edge * e = removeEdge(originNode, last_node);
    if (e != nullptr)
        delete e;
}

DijkstraKshortest::Edge * DijkstraKshortest::removeEdge(const NodeId & originNode, const NodeId & last_node)
{
    auto it = linkArray.find(originNode);
    if (it != linkArray.end()) {
        for (auto itAux = it->second.begin(); itAux != it->second.end(); ++itAux) {
            Edge *edge = *itAux;
            if (last_node == edge->last_node_) {
                it->second.erase(itAux);
                routeMap.clear();
                return (edge);
            }
        }
    }
    return (nullptr);
}

const DijkstraKshortest::Edge * DijkstraKshortest::getEdge(const NodeId & originNode, const NodeId & last_node)
{
    auto it = linkArray.find(originNode);
    if (it != linkArray.end()) {
        for (auto itAux = it->second.begin(); itAux != it->second.end(); ++itAux) {
            Edge *edge = *itAux;
            if (last_node == edge->last_node_) {
                return (edge);
            }
        }
    }
    return (nullptr);
}



void DijkstraKshortest::setRoot(const NodeId & dest_node)
{
    rootNode = dest_node;

}

bool DijkstraKshortest::nodeExist(const NodeId &) {
    auto it = linkArray.find(rootNode);
    if (it == linkArray.end())
        return false;
    return true;
}

void DijkstraKshortest::run()
{
    std::multiset<SetElem> heap;
    routeMap.clear();

    // include routes in the map
    kRoutesMap.clear();

    auto it = linkArray.find(rootNode);
    if (it==linkArray.end())
        throw cRuntimeError("Node not found");
    for (int i=0;i<K_LIMITE;i++)
    {
        State state(minimumCost);
        state.label=perm;
        routeMap[rootNode].push_back(state);
    }
    SetElem elem;
    elem.iD = rootNode;
    elem.idx = 0;
    elem.cost=minimumCost;
    heap.insert(elem);
    while (!heap.empty())
    {
        SetElem elem=*heap.begin();
        heap.erase(heap.begin());
        auto  it = routeMap.find(elem.iD);
        if (it==routeMap.end())
            throw cRuntimeError("node not found in routeMap");

        if (elem.iD != rootNode)
        {
            if (it->second.size() > elem.idx && it->second[elem.idx].label == perm)
                continue; // set
            if ((int)it->second.size() == K_LIMITE)
            {
                bool continueLoop = true;
                for (int i=0;i<K_LIMITE;i++)
                {
                    if (it->second[i].label != perm)
                    {
                        continueLoop = false;
                        break;
                    }
                }
                if (continueLoop)
                    continue; // nothing to do with this element
            }
        }

        if ((int)it->second.size()<=elem.idx)
        {
            for (int i = elem.idx -((int)it->second.size()-1);i>=0;i--)
            {
                State state(maximumCost);
                state.label=tent;
                it->second.push_back(state);
            }
        }

        /// Record the route in the map
        auto itAux = it;
        Route pathActive;
        Route pathNode;
        int prevIdx = elem.idx;
        NodeId currentNode =  elem.iD;
        while (currentNode!=rootNode)
        {
            pathActive.push_back(currentNode);
            currentNode = itAux->second[prevIdx].idPrev;
            prevIdx = itAux->second[prevIdx].idPrevIdx;
            itAux = routeMap.find(currentNode);
            if (itAux == routeMap.end())
                throw cRuntimeError("error in data");
            if (prevIdx >= (int) itAux->second.size())
                throw cRuntimeError("error in data");
        }

        bool routeExist = false;
        if (!pathActive.empty()) // valid path, record in the map
        {
            while (!pathActive.empty())
            {
                pathNode.push_back(pathActive.back());
                pathActive.pop_back();
            }
            auto itKroutes = kRoutesMap.find(elem.iD);
            if (itKroutes == kRoutesMap.end())
            {
                Kroutes kroutes;
                kroutes.push_back(pathNode);
                kRoutesMap.insert(std::make_pair(elem.iD, kroutes));
            }
            else
            {
                for (unsigned int j = 0; j < itKroutes->second.size(); j++)
                {
                    if (pathNode == itKroutes->second[j])
                    {
                        routeExist = true;
                        break;
                    }
                }
                if (!routeExist)
                {
                    if ((int)itKroutes->second.size() < K_LIMITE)
                        itKroutes->second.push_back(pathNode);
                }
            }
        }

        if (routeExist)
            continue; // next
        it->second[elem.idx].label=perm;

        // next hop
        auto linkIt=linkArray.find(elem.iD);
        if (linkIt == linkArray.end())
            throw cRuntimeError("Error link not found in linkArray");

        for (unsigned int i=0;i<linkIt->second.size();i++)
        {
            Edge *current_edge= (linkIt->second)[i];
            CostVector cost;
            CostVector maxCost = maximumCost;
            int nextIdx;

            // check if the node is in the path
            if (std::find(pathNode.begin(),pathNode.end(),current_edge->last_node())!=pathNode.end())
                continue;

            auto itNext = routeMap.find(current_edge->last_node());

            addCost(cost,current_edge->cost,(it->second)[elem.idx].cost);
            if (!limitsData.empty())
            {
                if (limitsData<cost)
                    continue;
            }

            if (itNext==routeMap.end() || (itNext!=routeMap.end() && (int)itNext->second.size()<K_LIMITE))
            {
                State state;
                state.idPrev=elem.iD;
                state.idPrevIdx=elem.idx;
                state.cost=cost;
                state.label=tent;
                routeMap[current_edge->last_node()].push_back(state);
                nextIdx = routeMap[current_edge->last_node()].size()-1;
                SetElem newElem;
                newElem.iD=current_edge->last_node();
                newElem.idx=nextIdx;
                newElem.cost=cost;
                heap.insert(newElem);
            }
            else
            {
                bool permanent = true;
                for (unsigned i=0;i<itNext->second.size();i++)
                {
                    if ((maxCost<itNext->second[i].cost)&&(itNext->second[i].label==tent))
                    {
                        maxCost = itNext->second[i].cost;
                        nextIdx=i;
                        permanent = false;
                    }
                }
                if (cost<maxCost && !permanent)
                {
                    itNext->second[nextIdx].cost=cost;
                    itNext->second[nextIdx].idPrev=elem.iD;
                    itNext->second[nextIdx].idPrevIdx=elem.idx;
                    SetElem newElem;
                    newElem.iD=current_edge->last_node();
                    newElem.idx=nextIdx;
                    newElem.cost=cost;
                    for (auto it = heap.begin(); it != heap.end(); ++it)
                    {
                        if (it->iD == newElem.iD && it->idx == newElem.idx && it->cost > newElem.cost)
                        {
                            heap.erase(it);
                            break;
                        }
                    }
                    heap.insert(newElem);
                }
            }
        }
    }
}

void DijkstraKshortest::runUntil (const NodeId &target)
{
    std::multiset<SetElem> heap;
    routeMap.clear();

    auto it = linkArray.find(rootNode);
    if (it==linkArray.end())
        throw cRuntimeError("Node not found");
    for (int i=0;i<K_LIMITE;i++)
    {
        State state(minimumCost);
        state.label=perm;
        routeMap[rootNode].push_back(state);
    }
    SetElem elem;
    elem.iD=rootNode;
    elem.idx=0;
    elem.cost=minimumCost;
    heap.insert(elem);
    while (!heap.empty())
    {
        SetElem elem=*heap.begin();
        heap.erase(heap.begin());
        auto it = routeMap.find(elem.iD);
        if (it==routeMap.end())
            throw cRuntimeError("node not found in routeMap");

        if (elem.iD != rootNode)
        {
            if (it->second.size() > elem.idx && it->second[elem.idx].label == perm)
                continue; // set
            if ((int)it->second.size() == K_LIMITE)
            {
                bool continueLoop = true;
                for (int i=0;i<K_LIMITE;i++)
                {
                    if (it->second[i].label != perm)
                    {
                        continueLoop = false;
                        break;
                    }
                }
                if (continueLoop)
                    continue;
            }
        }

        if ((int)it->second.size()<=elem.idx)
        {
            for (int i = elem.idx -((int)it->second.size()-1);i>=0;i--)
            {
                State state(maximumCost);
                state.label=tent;
                it->second.push_back(state);
            }
        }
        (it->second)[elem.idx].label=perm;
        if ((int)it->second.size()==K_LIMITE && target==elem.iD)
            return;
        auto linkIt=linkArray.find(elem.iD);
        if (linkIt == linkArray.end())
            throw cRuntimeError("Error link not found in linkArray");
        for (unsigned int i=0;i<linkIt->second.size();i++)
        {
            Edge* current_edge= (linkIt->second)[i];
            CostVector cost;
            CostVector maxCost = maximumCost;
            int nextIdx;
            auto itNext = routeMap.find(current_edge->last_node());
            addCost(cost,current_edge->cost,(it->second)[elem.idx].cost);
            if (!limitsData.empty())
            {
                if (limitsData<cost)
                    continue;
            }
            if ((itNext==routeMap.end()) ||  (itNext!=routeMap.end() && (int)itNext->second.size()<K_LIMITE))
            {
                State state;
                state.idPrev=elem.iD;
                state.idPrevIdx=elem.idx;
                state.cost=cost;
                state.label=tent;
                routeMap[current_edge->last_node()].push_back(state);
                nextIdx = routeMap[current_edge->last_node()].size()-1;
                SetElem newElem;
                newElem.iD=current_edge->last_node();
                newElem.idx=nextIdx;
                newElem.cost=cost;
                heap.insert(newElem);
            }
            else
            {
                bool permanent = true;
                for (unsigned i=0;i<itNext->second.size();i++)
                {
                    if ((maxCost<itNext->second[i].cost)&&(itNext->second[i].label==tent))
                    {
                        maxCost = itNext->second[i].cost;
                        nextIdx=i;
                        permanent = false;
                    }
                }
                if (cost<maxCost && !permanent)
                {
                    itNext->second[nextIdx].cost = cost;
                    itNext->second[nextIdx].idPrev = elem.iD;
                    itNext->second[nextIdx].idPrevIdx = elem.idx;
                    SetElem newElem;
                    newElem.iD=current_edge->last_node();
                    newElem.idx=nextIdx;
                    newElem.cost=cost;
                    for (auto it = heap.begin(); it != heap.end(); ++it)
                    {
                        if (it->iD == newElem.iD && it->idx == newElem.idx && it->cost > newElem.cost)
                        {
                            heap.erase(it);
                            break;
                        }
                    }
                    heap.insert(newElem);
                }
            }
        }
    }
}

int DijkstraKshortest::getNumRoutes(const NodeId &nodeId)
{
    auto it = routeMap.find(nodeId);
    if (it==routeMap.end())
        return -1;
    return (int)it->second.size();
}

bool DijkstraKshortest::getRoute(const NodeId &nodeId,std::vector<NodeId> &pathNode,int k)
{
    auto it = routeMap.find(nodeId);
    if (it == routeMap.end())
        return false;
    if (k >= (int) it->second.size())
        return false;
    std::vector<NodeId> path;
    NodeId currentNode = nodeId;
    int idx = k;
    while (currentNode != rootNode) {
        path.push_back(currentNode);
        currentNode = it->second[idx].idPrev;
        idx = it->second[idx].idPrevIdx;
        it = routeMap.find(currentNode);
        if (it == routeMap.end())
            throw cRuntimeError("error in data");
        if (idx >= (int) it->second.size())
            throw cRuntimeError("error in data");
    }
    pathNode.clear();
    while (!path.empty()) {
        pathNode.push_back(path.back());
        path.pop_back();
    }
    return true;
}


void DijkstraKshortest::getAllRoutes(std::map<NodeId, std::vector<std::vector<NodeId>>> & routes)
{
    routes.clear();
    for (auto elem : routeMap) {
        std::vector<std::vector<NodeId>> nodePaths;

        for (int i = 0; i < elem.second.size(); i++) {
            std::vector<NodeId> path;
            std::vector<NodeId> pathNode;
            NodeId currentNode = elem.first;
            int idx = i;
            auto it = routeMap.find(currentNode);
            while (currentNode != rootNode) {
                path.push_back(currentNode);
                currentNode = it->second[idx].idPrev;
                idx = it->second[idx].idPrevIdx;
                it = routeMap.find(currentNode);
                if (it == routeMap.end())
                    throw cRuntimeError("error in data");
                if (idx >= (int) it->second.size())
                    throw cRuntimeError("error in data");
            }
            while (!path.empty()) {
                pathNode.push_back(path.back());
                path.pop_back();
            }
            nodePaths.push_back(path);
        }
        routes[elem.first] = nodePaths;
    }
}

void DijkstraKshortest::setRouteMapK()
{
    kRoutesMap.clear();
    std::vector<NodeId> pathNode;
    for (auto it = routeMap.begin();it != routeMap.begin();++it)
    {
        for (int i = 0; i < (int)it->second.size();i++)
        {
            std::vector<NodeId> path;
            NodeId currentNode = it->first;
            int idx=it->second[i].idPrevIdx;
            while (currentNode!=rootNode)
            {
                path.push_back(currentNode);
                currentNode = it->second[idx].idPrev;
                idx=it->second[idx].idPrevIdx;
                it = routeMap.find(currentNode);
                if (it==routeMap.end())
                    throw cRuntimeError("error in data");
                if (idx>=(int)it->second.size())
                    throw cRuntimeError("error in data");
            }
            pathNode.clear();
            while (!path.empty())
            {
                pathNode.push_back(path.back());
                path.pop_back();
            }
            kRoutesMap[it->first].push_back(pathNode);
        }
    }
}


void DijkstraKshortest::getRouteMapK(const NodeId &nodeId, Kroutes &routes)
{
    routes.clear();
    auto it = kRoutesMap.find(nodeId);
    if (it == kRoutesMap.end())
        return;
    routes = it->second;
}

static L3Address getId(cModule *mod , L3Address::AddressType type)
{
    NodeId id;
    switch(type) {
    case L3Address::MAC:
        id = L3AddressResolver().addressOf(mod, L3AddressResolver::ADDR_MAC);
        break;
    case L3Address::IPv6:
        id = L3AddressResolver().addressOf(mod, L3AddressResolver::ADDR_IPv6);
        break;
    case L3Address::NONE:
        id = L3AddressResolver().addressOf(mod);
        break;
    default:
        id = L3AddressResolver().addressOf(mod, L3AddressResolver::ADDR_IPv4);
    }
    return id;
}

void DijkstraKshortest::setFromTopo(const cTopology *topo, L3Address::AddressType type)
{
    for (int i = 0; i < topo->getNumNodes(); i++) {
        cTopology::Node *node = const_cast<cTopology*>(topo)->getNode(i);
        NodeId id = getId(node->getModule(), type);
        for (int j = 0; j < node->getNumOutLinks(); j++) {
            NodeId idNex = getId(node->getLinkOut(j)->getRemoteNode()->getModule(), type);

            double cost=node->getLinkOut(j)->getWeight();
            addEdge (id,idNex,cost,0,1000,0);
        }
    }
}


inline bool operator <(const Dijkstra::SetElem& x, const Dijkstra::SetElem& y)
{
    if (x.m == Dijkstra::widestshortest) {
        if (x.cost != y.cost)
            return (x.cost < y.cost);
        return (x.cost2 > y.cost2);
    }
    if (x.m == Dijkstra::shortestwidest) {
        if (x.cost2 != y.cost2)
            return (x.cost2 > y.cost2);
        return (x.cost < y.cost);
    }
    return (x.cost < y.cost);
}

inline bool operator >(const Dijkstra::SetElem& x, const Dijkstra::SetElem& y)
{
    if (x.m == Dijkstra::widestshortest) {
        if (x.cost != y.cost)
            return (x.cost > y.cost);
        return (x.cost2 < y.cost2);
    }
    if (x.m == Dijkstra::shortestwidest) {
        if (x.cost2 != y.cost2)
            return (x.cost2 < y.cost2);
        return (x.cost > y.cost);
    }
    return (x.cost > y.cost);
}


Dijkstra::State::State(): idPrev(UndefinedAddr), label(tent)
{
}

Dijkstra::State::State(const double &costData, const double &c ) :Dijkstra::State::State()
{
    cost = costData;
    cost2 = c;
}

Dijkstra::State::~State()
{

}

Dijkstra::Dijkstra()
{
    rootNode = UndefinedAddr;
}

void Dijkstra::cleanLinkArray()
{
    cleanLinkArray(linkArray);
}

void Dijkstra::clearAll()
{
    cleanLinkArray();
    routeMap.clear();
}

void Dijkstra::cleanLinkArray(LinkArray &linkArray)
{
    for (auto it = linkArray.begin(); it != linkArray.end(); it++)
        while (!it->second.empty()) {
            delete it->second.back();
            it->second.pop_back();
        }
    linkArray.clear();
}

Dijkstra::~Dijkstra()
{
    cleanLinkArray();
}

void Dijkstra::addEdge(const NodeId & originNode, const NodeId & last_node, const double &cost, const double &cost2, LinkArray & linkArray)
{
    auto it = linkArray.find(originNode);
    if (it != linkArray.end()) {
        for (unsigned int i = 0; i < it->second.size(); i++) {
            if (last_node == it->second[i]->last_node_) {
                // check changes in the cost
                if (it->second[i]->Cost() != cost || it->second[i]->Cost2() != cost2) {
                    it->second[i]->Cost() = cost;
                    it->second[i]->Cost2() = cost2;
                    // delete routing stored information
                    routeMap.clear();
                }
                return;
            }
        }
    }
    Edge *link = new Edge;
    // The last hop is the interface in which we have this neighbor...
    link->last_node() = last_node;
    // Also record the link delay and quality..
    link->Cost() = cost;
    link->Cost2() = cost2;
    linkArray[originNode].push_back(link);
    routeMap.clear();
}

void Dijkstra::addEdge(const NodeId & originNode, Edge * edge, LinkArray & linkArray)
{
    auto it = linkArray.find(originNode);
    routeMap.clear();
    if (it != linkArray.end()) {
        for (unsigned int i = 0; i < it->second.size(); i++) {
            if (edge->last_node() == it->second[i]->last_node_) {
                it->second[i]->Cost() = edge->Cost();
                delete edge;
                return;
            }
        }
    }
    linkArray[originNode].push_back(edge);
}

void Dijkstra::deleteEdge(const NodeId & originNode, const NodeId & last_node, LinkArray & linkArray)
{
    Dijkstra::Edge * e = removeEdge(originNode, last_node,linkArray);
    if (e != nullptr)
        delete e;
}

Dijkstra::Edge * Dijkstra::removeEdge(const NodeId & originNode, const NodeId & last_node, LinkArray & linkArray)
{
    auto it = linkArray.find(originNode);
    if (it != linkArray.end()) {
        for (auto itAux = it->second.begin(); itAux != it->second.end(); ++itAux) {
            Edge *edge = *itAux;
            if (last_node == edge->last_node_) {
                it->second.erase(itAux);
                routeMap.clear();
                return (edge);
            }
        }
    }
    return (nullptr);
}

const Dijkstra::Edge * Dijkstra::getEdge(const NodeId & originNode, const NodeId & last_node, LinkArray & linkArray)
{
    auto it = linkArray.find(originNode);
    if (it != linkArray.end()) {
        for (auto itAux = it->second.begin(); itAux != it->second.end(); ++itAux) {
            Edge *edge = *itAux;
            if (last_node == edge->last_node_) {
                return (edge);
            }
        }
    }
    return (nullptr);
}


void Dijkstra::addEdge(const NodeId & originNode, const NodeId & last_node, const double &cost,  const double &cost2)
{
    addEdge(originNode, last_node, cost, cost2, linkArray);
}

void Dijkstra::addEdge(const NodeId & originNode, Edge * edge)
{
    addEdge(originNode, edge,linkArray);
}

void Dijkstra::deleteEdge(const NodeId & originNode, const NodeId & last_node)
{
    deleteEdge(originNode, last_node, linkArray);
}

const Dijkstra::Edge *Dijkstra::getEdge(const NodeId & originNode, const NodeId & last_node)
{
    return getEdge(originNode, last_node, linkArray);
}

void Dijkstra::setRoot(const NodeId & dest_node)
{
    rootNode = dest_node;
}

void Dijkstra::run()
{
    run(rootNode, linkArray, routeMap);
}

void Dijkstra::runUntil(const NodeId &target)
{
    runUntil(target, rootNode, linkArray, routeMap);
}

void Dijkstra::run(const NodeId &rootNode, const LinkArray &linkArray, RouteMap &routeMap)
{
    runUntil(UndefinedAddr, rootNode, linkArray, routeMap);
}

bool Dijkstra::nodeExist(const NodeId &) {
    auto it = linkArray.find(rootNode);
    if (it == linkArray.end())
        return false;
    return true;
}

void Dijkstra::runUntil(const NodeId &target, const NodeId &rootNode, const LinkArray &linkArray, RouteMap &routeMap)
{
    //std::multiset<SetElem> heap;
    std::deque<Dijkstra::SetElem> heap;
    routeMap.clear();

    auto it = linkArray.find(rootNode);
    if (it == linkArray.end())
        throw cRuntimeError("Node not found");

    State state(0,1e300);
    state.label = perm;
    routeMap[rootNode] = state;

    SetElem elem;
    elem.iD = rootNode;
    elem.cost = 0;
    //heap.insert(elem);
    heap.push_back(elem);
    while (!heap.empty()) {
        auto itHeap = heap.begin();
        double cost1It = itHeap->cost;
        double cost2It = itHeap->cost2;
        // search min
        for (auto it = heap.begin(); it!=heap.end(); ++it)
        {
            if (method == Method::widestshortest) {
                if (it->cost < cost1It || (it->cost == cost1It && it->cost2 > cost2It)) {
                    itHeap = it;
                    cost1It = itHeap->cost;
                    cost2It = itHeap->cost2;
                }
            }
            else if (method == Method::shortestwidest) {
                if (it->cost2 > cost2It || (it->cost2 == cost2It && it->cost < cost1It)) {
                    itHeap = it;
                    cost1It = itHeap->cost;
                    cost2It = itHeap->cost2;
                }
            }
            else {
                if (it->cost < cost1It) {
                    itHeap = it;
                    cost1It = itHeap->cost;
                    cost2It = itHeap->cost2;
                }
            }
        }

        // SetElem elem = *heap.begin();
        //heap.erase(heap.begin());
        SetElem elem = *itHeap;
        heap.erase(itHeap);
        auto it = routeMap.find(elem.iD);
        if (it == routeMap.end())
            throw cRuntimeError("node not found in routeMap");

        if (elem.iD != rootNode) {
            if (it->second.label == perm)
                continue; // nothing to do with this element
        }
        it->second.label = perm;

        if (target == elem.iD)
            return;
        auto linkIt = linkArray.find(elem.iD);
        if (linkIt == linkArray.end())
            throw cRuntimeError("Error link not found in linkArray");
        for (unsigned int i = 0; i < linkIt->second.size(); i++) {

            Edge* current_edge = (linkIt->second)[i];
            double cost;
            double cost2;
            auto itNext = routeMap.find(current_edge->last_node());
            cost = current_edge->cost + it->second.cost;
            cost2 = std::min(current_edge->cost2, it->second.cost2);

           if (itNext == routeMap.end()) {
                State state;
                state.idPrev = elem.iD;
                state.cost = cost;
                state.cost2 = cost2;
                state.label = tent;
                routeMap[current_edge->last_node()] = state;

                SetElem newElem;
                newElem.iD = current_edge->last_node();
                newElem.cost = cost;
                newElem.cost2 = cost2;
                heap.push_back(newElem);
                //heap.insert(newElem);
            }
            else {
                if (itNext->second.label == perm)
                    continue;

                double maxCost = itNext->second.cost;
                double maxCost2 = itNext->second.cost2;
                bool actualize = false;

                if (method == Method::widestshortest) {
                    if (cost < maxCost || (cost == maxCost && cost2 > maxCost2))
                        actualize = true;
                }
                else if (method == Method::shortestwidest) {
                    if (cost2 > maxCost2 || (cost2 == maxCost2 && cost < maxCost))
                        actualize = true;
                }
                else {
                    if (cost < maxCost)
                        actualize = true;
                }

                if (actualize) {
                    itNext->second.cost = cost;
                    itNext->second.idPrev = elem.iD;
                    SetElem newElem;
                    newElem.iD = current_edge->last_node();
                    newElem.cost = cost;
                    //for (auto it = heap.begin(); it != heap.end(); ++it) {
                    //    if (it->iD == newElem.iD && it->cost > newElem.cost) {
                    //        heap.erase(it);
                    //        break;
                    //    }
                    //}
                    heap.push_back(newElem);
                    //heap.insert(newElem);
                }
            }
        }
    }
}

bool Dijkstra::getRoute(const NodeId &nodeId, std::vector<NodeId> &pathNode, const RouteMap &routeMap)
{
    auto it = routeMap.find(nodeId);
    if (it == routeMap.end())
        return (false);
    std::vector<NodeId> path;
    NodeId currentNode = nodeId;

    while (currentNode != rootNode) {
        path.push_back(currentNode);
        currentNode = it->second.idPrev;
        it = routeMap.find(currentNode);
        if (it == routeMap.end())
            throw cRuntimeError("error in data");
    }
    path.push_back(rootNode);
    pathNode.clear();
    while (!path.empty()) {
        pathNode.push_back(path.back());
        path.pop_back();
    }
    return (true);

}

bool Dijkstra::getRoute(const NodeId &nodeId, std::vector<NodeId> &pathNode)
{
    return (getRoute(nodeId, pathNode, routeMap));
}

void Dijkstra::getRoutes(std::map<NodeId, std::vector<NodeId>> &paths) {
    getRoutes(paths, routeMap);
}


void Dijkstra::getRoutes(std::map<NodeId, std::vector<NodeId>> &paths, const RouteMap &routeMap)
{
    for (const auto &elem : routeMap) {
        std::vector<NodeId> path;
        std::vector<NodeId> pathNode;
        NodeId currentNode = elem.first;
        auto it = routeMap.find(currentNode);

        while (currentNode != rootNode) {
            path.push_back(currentNode);
            currentNode = it->second.idPrev;
            it = routeMap.find(currentNode);
            if (it == routeMap.end())
                throw cRuntimeError("error in data");
        }

        path.push_back(rootNode);
        pathNode.clear();
        while (!path.empty()) {
            pathNode.push_back(path.back());
            path.pop_back();
        }
        paths[elem.first] = pathNode;
    }
}



void Dijkstra::setFromTopo(const cTopology *topo, L3Address::AddressType type )
{
    for (int i = 0; i < topo->getNumNodes(); i++) {
        cTopology::Node *node = const_cast<cTopology*>(topo)->getNode(i);
        NodeId id = getId(node->getModule(), type);
        for (int j = 0; j < node->getNumOutLinks(); j++) {
            NodeId idNex = getId(node->getLinkOut(j)->getRemoteNode()->getModule(), type);
            cChannel * channel = node->getLinkOut(j)->getLocalGate()->getTransmissionChannel();
            double cost = 1 / channel->getNominalDatarate();
            addEdge(id, idNex, cost,channel->getNominalDatarate());
        }
    }
}

void Dijkstra::discoverPartitionedLinks(std::vector<NodeId> &pathNode, const LinkArray & topo, NodePairs &links)
{

    LinkArray topoAux = topo;
    for (unsigned int i = 0; i < pathNode.size() - 1; i++) {
        auto it1 = topoAux.find(pathNode[i]);
        if (it1 == topoAux.end())
            throw cRuntimeError("Node not found %s", pathNode[i].str().c_str());
        auto it = topoAux.find(pathNode[i]);
        Edge *tempEdge = nullptr;
        NodeId origin = it1->first;
        NodeId nodeId = pathNode[i + 1];
        for (auto itAux = it->second.begin(); itAux != it->second.end(); ++it) {
            if ((*itAux)->last_node() == pathNode[i + 1]) {
                tempEdge = *itAux;
                it->second.erase(itAux);
                break;
            }
        }
        if (tempEdge == nullptr)
            throw cRuntimeError("Link not found %s - %s", pathNode[i].str().c_str(), pathNode[i + 1].str().c_str());
        RouteMap routeMap;

        runUntil(nodeId, origin, topoAux, routeMap);
        setRoot(origin);
        std::vector<NodeId> pathNode;
        bool has = getRoute(nodeId, pathNode, routeMap);
        // include the link other time
        it->second.push_back(tempEdge);
        if (!has) {
            links.push_back(std::make_pair(origin,nodeId));
        }
    }
}

void Dijkstra::discoverAllPartitionedLinks(const LinkArray & topo, NodePairs &links)
{
    LinkArray topoAux = topo;
    NodePairs tested;
    for (auto elem : topo)
    {
        NodeId node = elem.first;
        setRoot(node);
        for (unsigned int i = 0; i < elem.second.size();i++)
        {
            auto it1 = std::find(tested.begin(),tested.end(),std::make_pair(node, elem.second[i]->last_node()));
            if (it1 != tested.end())
                continue;
            Edge * edge = removeEdge(node, elem.second[i]->last_node(), topoAux);
            RouteMap routeMap;
            runUntil(elem.second[i]->last_node(), node, topoAux, routeMap);
            std::vector<NodeId> pathNode;
            bool has = getRoute(elem.second[i]->last_node(), pathNode, routeMap);
            if (!has) {
                links.push_back(std::make_pair(node,elem.second[i]->last_node()));
                links.push_back(std::make_pair(elem.second[i]->last_node(),node));
            }
            addEdge(node, edge, topoAux);
            tested.push_back(std::make_pair(node, elem.second[i]->last_node()));
            tested.push_back(std::make_pair(elem.second[i]->last_node(),node));
        }
    }
}


}
