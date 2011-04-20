/***************************************************************************
 *   Copyright (C) 2004 by Francisco J. Ros                                *
 *   fjrm@dif.um.es                                                        *
 *                                                                         *
 *   Modified by Weverton Cordeiro                                         *
 *   (C) 2007 wevertoncordeiro@gmail.com                                   *
 *   Adapted for omnetpp                                                   *
 *   2008 Alfonso Ariza Quintana aarizaq@uma.es                            *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*** New File Added ***/

#ifndef __OLSR_ETX_dijkstra_h__
#define __OLSR_ETX_dijkstra_h__

#include <OLSR_ETX_parameter.h>
#include <OLSR_repositories.h>
#include <vector>
#include <set>
#include <map>

typedef struct edge
{
    nsaddr_t last_node_; // last node to reach node X
    double   quality_; // link quality
    double   delay_; // link delay

    inline nsaddr_t& last_node() { return last_node_; }
    inline double&   quality()     { return quality_; }
    inline double&   getDelay()     { return delay_; }
} edge;

typedef struct hop
{
    edge     link_; // indicates the total cost to reach X and the last node to reach it
    int      hop_count_; // number of hops in this path

    inline   edge& link() { return link_; }
    inline   int&  hop_count() { return hop_count_; }
} hop;



///
/// \brief Class that implements the Dijkstra Algorithm
///
///
class Dijkstra : public cOwnedObject
{
  private:
	typedef std::set<nsaddr_t> NodesSet;
	typedef std::map<nsaddr_t, std::vector<edge*> > LinkArray;
	NodesSet * nonprocessed_nodes_;
	LinkArray * link_array_;
    int highest_hop_;

    NodesSet::iterator best_cost();
    edge* get_edge (const nsaddr_t &, const nsaddr_t &);
    OLSR_ETX_parameter *parameter;

  public:
    NodesSet *all_nodes_;

    // D[node].first == cost to reach 'node' through this hop
    // D[node].second.first == last hop to reach 'node'
    // D[node].second.second == number of hops to reach 'node'
    typedef std::map<nsaddr_t, hop >  DijkstraMap;
    DijkstraMap dijkstraMap;

    Dijkstra();
    ~Dijkstra();

    void add_edge (const nsaddr_t &, const nsaddr_t &, double, double, bool);
    void run();
    void clear();

    inline int highest_hop() { return highest_hop_; }
    inline std::set<nsaddr_t> * all_nodes() { return all_nodes_; }
    inline hop& D(const nsaddr_t &node) { return dijkstraMap[node]; }
};

#endif
