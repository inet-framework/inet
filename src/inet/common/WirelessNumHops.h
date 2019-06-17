//
// Copyright (C) 2012 Univerdidad de Malaga.
// Author: Alfonso Ariza
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef WIRELESSNUMHOPS_H_
#define WIRELESSNUMHOPS_H_

#include <vector>
#include <map>
#include <deque>
#include "inet/common/geometry/common/Coord.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"

namespace inet{

class IMobility;
class IInterfaceTable;

class WirelessNumHops : public cOwnedObject
{
        struct nodeInfo
        {
            IMobility* mob;
            IInterfaceTable* itable;
            std::vector<MacAddress> macAddress;
            std::vector<Ipv4Address> ipAddress;
        };
        struct LinkPair
        {
            int node1;
            int node2;
            double cost;
            LinkPair()
            {
                node1 = node2 = -1;
            }
            LinkPair(int i, int j)
            {
                node1 = i;
                node2 = j;
                cost = -1;
            }

            LinkPair(int i, int j,double c)
            {
                node1 = i;
                node2 = j;
                cost = c;
            }

            LinkPair& operator=(const LinkPair& val)
            {
                this->node1 = val.node1;
                this->node2 = val.node2;
                this->cost = val.cost;
                return *this;
            }
        };

        std::vector<nodeInfo> vectorList;
        std::map<MacAddress,int> related;
        std::map<Ipv4Address,int> relatedIp;
        typedef std::map<MacAddress,std::deque<MacAddress> > RouteCacheMac;
        typedef std::map<Ipv4Address,std::deque<Ipv4Address> > RouteCacheIp;

        typedef std::set<LinkPair> LinkCache;
        RouteCacheIp routeCacheIp;
        LinkCache linkCache;
        RouteCacheMac routeCacheMac;
        bool staticScenario;
    protected:
        cModule *node;
        enum StateLabel {perm,tent};
        class DijkstraShortest
        {
            public:
            class SetElem
            {
            public:
                int iD;
                unsigned int cost;
                double costAdd;
                double costMax;
                SetElem()
                {
                    iD = -1;
                }
                SetElem& operator=(const SetElem& val)
                {
                    this->iD=val.iD;
                    this->cost = val.cost;
                    this->costAdd = val.costAdd;
                    this->costMax = val.costMax;
                    return *this;
                }
                bool operator<(const SetElem& val)
                {
                    if (this->cost > val.cost)
                        return false;
                    if (this->cost < val.cost)
                        return true;
                    if (this->costAdd < val.costAdd)
                        return true;
                    if (this->costMax < val.costMax)
                       return true;
                    return false;
                }
                bool operator>(const SetElem& val)
                {
                    if (this->cost < val.cost)
                        return false;
                    if (this->cost > val.cost)
                        return true;
                    if (this->costAdd > val.costAdd)
                        return true;
                    if (this->costMax > val.costMax)
                       return true;
                    return false;
                }
            };
            class State
            {
            public:
                unsigned int cost;
                double costAdd;
                double costMax;
                int idPrev;
                StateLabel label;
                State();
                State(const unsigned int &cost);
                State(const unsigned int &cost, const double &cost1, const double &cost2);
                void setCostVector(const unsigned int &cost);
                void setCostVector(const unsigned int &cost, const double &cost1, const double &cost2);
            };

            struct Edge
            {
                int last_node_; // last node to reach node X
                unsigned int cost;
                double costAdd;
                double costMax;
                Edge()
                {
                    cost = 200000;
                    costAdd = 1e30;
                    costMax = 1e30;
                }
            };
        };

        typedef std::map<int,WirelessNumHops::DijkstraShortest::State> RouteMap;
        typedef std::vector<WirelessNumHops::DijkstraShortest::Edge*> LinkCon;

        typedef std::map<int, LinkCon> LinkArray;
        LinkArray linkArray;
        RouteMap routeMap;
        int rootNode;

        virtual void cleanLinkArray();
        virtual void addEdge (const int & dest_node, const int & last_node,unsigned int cost);
        virtual void addEdge (const int & originNode, const int & last_node,unsigned int cost, double costAdd, double costMax);
        virtual bool getRoute(const int &nodeId,std::deque<int> &);
        virtual bool getRouteCost(const int &nodeId,std::deque<int> &,double &costAdd, double &costMax);
        virtual void setRoot(const int & dest_node);
        virtual void runUntil (const int &);
        virtual int getIdNode(const MacAddress &add);
        virtual int getIdNode(const  Ipv4Address &add);
        virtual std::deque<int> getRoute(int i);

        virtual bool findRoutePath(const int &dest,std::deque<int> &pathNode);
        virtual bool findRoutePathCost(const int &nodeId,std::deque<int> &pathNode,double &costAdd, double &costMax);
        virtual void setIpRoutingTable(const Ipv4Address &root, const Ipv4Address &desAddress, const Ipv4Address &gateway,  int hops);
        virtual void setIpRoutingTable(const Ipv4Address &desAddress, const Ipv4Address &gateway,  int hops);

    public:
        friend bool operator < (const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y );
        friend bool operator > (const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y );
        friend bool operator == (const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y );
        friend bool operator < ( const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y );
        friend bool operator > ( const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y );
        friend bool operator == ( const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y );

        virtual void fillRoutingTables(const double &tDistance);
        virtual void fillRoutingTablesWitCost(const double &tDistance);

        WirelessNumHops();
        virtual ~WirelessNumHops();
        virtual void reStart();
        virtual bool findRouteWithCost(const double &, const MacAddress &,std::deque<MacAddress> &, bool withCost, double &costAdd, double &costMax);
        virtual bool findRouteWithCost(const double &, const Ipv4Address &,std::deque<Ipv4Address> &, bool withCost, double &costAdd, double &costMax);

        virtual bool findRoute(const double &dist, const MacAddress &dest,std::deque<MacAddress> &pathNode)
        {
            double cost1,cost2;
            return findRouteWithCost(dist, dest,pathNode, false, cost1, cost2);
        }
        virtual bool findRoute(const double &dist, const Ipv4Address &dest,std::deque<Ipv4Address> &pathNode)
        {
            double cost1,cost2;
            return findRouteWithCost(dist, dest,pathNode, false, cost1, cost2);
        }

        virtual void setRoot(const MacAddress & dest_node)
        {
            setRoot(getIdNode(dest_node));
        }

        virtual void setRoot(const Ipv4Address & dest_node)
        {
            setRoot(getIdNode(dest_node));
        }

        void runUntil (const MacAddress &target)
        {
            runUntil(getIdNode(target));
        }

        virtual void run();

        virtual unsigned int getNumRoutes() {return routeMap.size();}

        virtual void getRoute(int ,std::deque<Ipv4Address> &pathNode);

        virtual void getRoute(int ,std::deque<MacAddress> &pathNode);

        virtual Coord getPos(const int &node);

        virtual Coord getPos(const Ipv4Address &node)
        {
            return getPos(getIdNode(node));
        }

        virtual Coord getPos(const MacAddress &node)
        {
            return getPos(getIdNode(node));
        }

        virtual void getNeighbours(const Ipv4Address &node, std::vector<Ipv4Address>&, const double &distance);
        virtual void getNeighbours(const MacAddress &node, std::vector<MacAddress>&, const double &distance);

        void setStaticScenario(bool val) {staticScenario = val;}
        virtual void setIpRoutingTable();
};


inline bool operator < ( const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y )
{
    if (x.iD != y.iD && x.cost < y.cost)
        return true;
    return false;
}

inline bool operator > ( const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y )
{
    if (x.iD != y.iD && x.cost > y.cost)
        return true;
    return false;
}

inline bool operator == (const WirelessNumHops::DijkstraShortest::SetElem& x, const WirelessNumHops::DijkstraShortest::SetElem& y )
{
    if (x.iD == y.iD)
        return true;
    return false;
}

inline bool operator < ( const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y )
{
    if (x.node1 != y.node1)
        return x.node1 < y.node1;
    return x.node2 < y.node2;
}


inline bool operator > ( const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y )
{
    if (x.node1 != y.node1)
       return (x.node1 > y.node1);
    return (x.node2 > y.node2);
}

inline bool operator == (const WirelessNumHops::LinkPair& x, const WirelessNumHops::LinkPair& y )
{
    return (x.node1 == y.node1 &&  x.node2 == y.node2);
}

}

#endif /* WIRELESSNUMHOPS_H_ */
