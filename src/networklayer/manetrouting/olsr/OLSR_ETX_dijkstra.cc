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

#include <OLSR_ETX_dijkstra.h>
#include <OLSR_ETX.h>


Dijkstra::Dijkstra()
{
    highest_hop_ = 0;

    dijkstraMap.clear();
    nonprocessed_nodes_ = new NodesSet;
    link_array_ = new LinkArray;
    all_nodes_ = new NodesSet;
    parameter = &(dynamic_cast<OLSR_ETX*>(getOwner())->parameter_);
}

void Dijkstra::add_edge (const nsaddr_t & dest_node, const nsaddr_t & last_node, double delay,
                         double quality, bool direct_connected)
{
    edge *link = new edge;

    // The last hop is the interface in which we have this neighbor...
    link->last_node() = last_node;
    // Also record the link delay and quality..
    link->getDelay() = delay;
    link->quality() = quality;
    LinkArray::iterator it = link_array_->find(dest_node);
    if (it==link_array_->end())
    {
    	std::vector<edge*> val;
    	link_array_->insert(std::pair<nsaddr_t,std::vector<edge*> >(dest_node,val));
    	it = link_array_->find(dest_node);
    }


    it->second.push_back(link);

    if (direct_connected)
    {

        // Since dest_node is directly connected to the node running the djiksra
        // algorithm, this link has hop count 1
    	dijkstraMap[dest_node].hop_count() = 1;
        // Report the best link we have at the moment to these nodes
    	dijkstraMap[dest_node].link().last_node() = link->last_node();
    	dijkstraMap[dest_node].link().getDelay() = link->getDelay();
    	dijkstraMap[dest_node].link().quality() = link->quality();
    }
    else if (all_nodes_->find(dest_node) == all_nodes_->end())
        // Since we don't have an edge connecting dest_node to the node running
        // the dijkstra algorithm, the cost is set to infinite...
    	dijkstraMap[dest_node].hop_count() = -1;

    // Add dest_node to the list of nodes we will have to iterate through
    // while calculating the best path among nodes...
    all_nodes_->insert(dest_node);

    // Add dest_node to the list of nodes we will have to process...
    nonprocessed_nodes_->insert(dest_node);
}

Dijkstra::NodesSet::iterator Dijkstra::best_cost ()
{
	NodesSet::iterator best = nonprocessed_nodes_->end();

    // Search for a node that was not processed yet and that has
    // the best cost to be reached from the node running dijkstra...
    NodesSet::iterator it;
    for (it = nonprocessed_nodes_->begin();it != nonprocessed_nodes_->end(); it++)
    {
        DijkstraMap::iterator itDij = dijkstraMap.find(*it);
        if (itDij==dijkstraMap.end())
            opp_error("dijkstraMap error");
        if (itDij->second.hop_count() == -1) // there is no such link yet, i. e., it's cost is infinite...
            continue;
        if (best == nonprocessed_nodes_->end())
            best = it;
        else
        {
        	DijkstraMap::iterator itBest = dijkstraMap.find(*best);
        	if (itBest==dijkstraMap.end())
        	    opp_error("dijkstraMap error");
            if (parameter->link_delay())
            {
                /// Link quality extension
                if (itDij->second.link().getDelay() < itBest->second.link().getDelay())
                    best = it;
            }
            else
            {
                switch (parameter->link_quality())
                {
                case OLSR_ETX_BEHAVIOR_ETX:
                    if (itDij->second.link().quality() < itBest->second.link().quality())
                        best = it;
                    break;

                case OLSR_ETX_BEHAVIOR_ML:
                    if (itDij->second.link().quality() > itBest->second.link().quality())
                        best = it;
                    break;

                case OLSR_ETX_BEHAVIOR_NONE:
                default:
                    //
                    break;
                }
            }
        }
    }
    return best;
}

edge* Dijkstra::get_edge (const nsaddr_t & dest_node, const nsaddr_t & last_node)
{
    // Find the edge that connects dest_node and last_node
	LinkArray::iterator itLink = link_array_->find(dest_node);
	if (itLink==link_array_->end())
	    opp_error("link_array_ error");
    for (std::vector<edge*>::iterator it = itLink->second.begin();it != itLink->second.end(); it++)
    {
        edge* current_edge = *it;
        if (current_edge->last_node() == last_node)
            return current_edge;
    }
    return NULL;
}

void Dijkstra::run()
{
    // While there are non processed nodes...
    while (nonprocessed_nodes_->begin() != nonprocessed_nodes_->end())
    {
        // Get the node among those nom processed having best cost...
    	NodesSet::iterator current_node = best_cost();
        // If all non processed nodes have cost equals to infinite, there is
        // nothing left to do, but abort (this might be the case of a not
        // fully connected graph)
        if (current_node == nonprocessed_nodes_->end())
            break;

        // for each node in all_nodes...
        for (NodesSet::iterator dest_node = all_nodes_->begin();dest_node != all_nodes_->end(); dest_node++)
        {
            // ... not processed yet...
            if (nonprocessed_nodes_->find(*dest_node) == nonprocessed_nodes_->end())
                continue;
            // .. and adjacent to 'current_node'...
            // note: edge has destination '*dest_node' and last hop '*current_node'
            edge* current_edge = get_edge(*dest_node, *current_node);
            if (current_edge == NULL)
                continue;
            // D(node) = min (D(node), D(current_node) + edge(current_node, node).cost())
            DijkstraMap::iterator itDest = dijkstraMap.find(*dest_node);
            if (itDest==dijkstraMap.end())
                opp_error("dijkstraMap error node not found");
            DijkstraMap::iterator itCurrent = dijkstraMap.find(*current_node);
            if (itCurrent==dijkstraMap.end())
                opp_error("dijkstraMap error node not found");
            hop destHop =itDest->second;
            hop currentHop =itCurrent->second;
            if (itDest->second.hop_count() == -1)   // there is not a link to dest_node yet...
            {
                switch (parameter->link_quality())
                {
                case OLSR_ETX_BEHAVIOR_ETX:
                	itDest->second.link().last_node() = current_edge->last_node();
                	itDest->second.link().quality() = itCurrent->second.link().quality() + current_edge->quality();
                    /// Link delay extension
                	itDest->second.link().getDelay() = itCurrent->second.link().getDelay() + current_edge->getDelay();
                	itDest->second.hop_count() = itCurrent->second.hop_count() + 1;
                    // Keep track of the highest path we have by means of number of hops...
                    if (itDest->second.hop_count() > highest_hop_)
                        highest_hop_ = itDest->second.hop_count();
                    break;

                case OLSR_ETX_BEHAVIOR_ML:
                	itDest->second.link().last_node() = current_edge->last_node();
                	itDest->second.link().quality() = itCurrent->second.link().quality() * current_edge->quality();
                    /// Link delay extension
                	itDest->second.link().getDelay() = itCurrent->second.link().getDelay() + current_edge->getDelay();
                	itDest->second.hop_count() = itCurrent->second.hop_count() + 1;
                    // Keep track of the highest path we have by means of number of hops...
                    if (itDest->second.hop_count() > highest_hop_)
                        highest_hop_ = itDest->second.hop_count();
                    break;

                case OLSR_ETX_BEHAVIOR_NONE:
                default:
                    //
                    break;
                }
            }
            else
            {
                if (parameter->link_delay())
                {
                    /// Link delay extension
                    switch (parameter->link_quality())
                    {
                    case OLSR_ETX_BEHAVIOR_ETX:
                        if (itCurrent->second.link().getDelay() + current_edge->getDelay() < itDest->second.link().getDelay())
                        {
                        	itDest->second.link().last_node() = current_edge->last_node();
                        	itDest->second.link().quality() = itCurrent->second.link().quality() + current_edge->quality();
                        	itDest->second.link().getDelay() = itCurrent->second.link().getDelay() + current_edge->getDelay();
                        	itDest->second.hop_count() = itCurrent->second.hop_count() + 1;
                            // Keep track of the highest path we have by means of number of hops...
                            if (itDest->second.hop_count() > highest_hop_)
                                highest_hop_ = itDest->second.hop_count();
                        }
                        break;

                    case OLSR_ETX_BEHAVIOR_ML:
                        if (itCurrent->second.link().getDelay() + current_edge->getDelay() < itDest->second.link().getDelay())
                        {
                        	itDest->second.link().last_node() = current_edge->last_node();
                        	itDest->second.link().quality() = itCurrent->second.link().quality() * current_edge->quality();
                        	itDest->second.link().getDelay() = itCurrent->second.link().getDelay() + current_edge->getDelay();
                        	itDest->second.hop_count() = itCurrent->second.hop_count() + 1;
                            // Keep track of the highest path we have by means of number of hops...
                            if (itDest->second.hop_count() > highest_hop_)
                                highest_hop_ = itDest->second.hop_count();
                        }
                        break;

                    case OLSR_ETX_BEHAVIOR_NONE:
                    default:
                        //
                        break;
                    }
                }
                else
                {
                    switch (parameter->link_quality())
                    {
                    case OLSR_ETX_BEHAVIOR_ETX:
                        if (itCurrent->second.link().quality() + current_edge->quality() < itDest->second.link().quality())
                        {
                        	itDest->second.link().last_node() = current_edge->last_node();
                        	itDest->second.link().quality() = itCurrent->second.link().quality() + current_edge->quality();
                        	itDest->second.hop_count() = itCurrent->second.hop_count() + 1;
                            // Keep track of the highest path we have by means of number of hops...
                            if (itDest->second.hop_count() > highest_hop_)
                                highest_hop_ = itDest->second.hop_count();
                        }
                        break;

                    case OLSR_ETX_BEHAVIOR_ML:
                        if (itCurrent->second.link().quality() * current_edge->quality() > itDest->second.link().quality())
                        {
                        	itDest->second.link().last_node() = current_edge->last_node();
                        	itDest->second.link().quality() = itCurrent->second.link().quality() * current_edge->quality();
                        	itDest->second.hop_count() = itCurrent->second.hop_count() + 1;
                            // Keep track of the highest path we have by means of number of hops...
                            if (itDest->second.hop_count() > highest_hop_)
                                highest_hop_ = itDest->second.hop_count();
                        }
                        break;

                    case OLSR_ETX_BEHAVIOR_NONE:
                    default:
                        //
                        break;
                    }
                }
            }
        }
        // Remove it from the list of processed nodes
        nonprocessed_nodes_->erase(current_node);
    }
}

void Dijkstra::clear()
{
    while (!link_array_->empty())
    {
        while (!link_array_->begin()->second.empty())
        {
             delete link_array_->begin()->second.back();
             link_array_->begin()->second.pop_back();
         }
         link_array_->erase(link_array_->begin());
    }
    delete link_array_;
    link_array_->clear();
    delete link_array_;
    dijkstraMap.clear();

    nonprocessed_nodes_->clear();
    delete nonprocessed_nodes_;
    all_nodes_->clear ();
    delete all_nodes_;

    link_array_ = NULL;
    nonprocessed_nodes_ = NULL;
    all_nodes_=NULL;
}

Dijkstra::~Dijkstra()
{
    if (nonprocessed_nodes_)
        delete nonprocessed_nodes_;
    dijkstraMap.clear();
    if (link_array_)
    {
        while (!link_array_->empty())
        {
            while (!link_array_->begin()->second.empty())
            {
                delete link_array_->begin()->second.back();
                link_array_->begin()->second.pop_back();
            }
            link_array_->erase(link_array_->begin());
        }
        delete link_array_;
    }
    if (all_nodes_)
        delete all_nodes_;
}

