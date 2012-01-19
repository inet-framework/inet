//
// Copyright (C) 2011 OpenSim Ltd
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
// @author: Zoltan Bojthe


#include "WeightedTopology.h"


#if OMNETPP_VERSION < 0x0403


void WeightedTopology::calculateWeightedSingleShortestPathsTo(Node *_target)
{
    if (!_target)
        throw cRuntimeError(this,"..ShortestPathTo(): target node is NULL");
    target = _target;

    // clean path infos
    for (int i=0; i<num_nodes; i++)
    {
       ((WNode *)nodev)[i].known = false;
       ((WNode *)nodev)[i].dist = INFINITY;
       ((WNode *)nodev)[i].out_path = NULL;
    }

    ((WNode *)target)->dist = 0;

    std::list<Node*> q;

    q.push_back(target);

    while (!q.empty())
    {
       Node *dest = q.front();
       q.pop_front();

       ASSERT(dest->getWeight() >= 0.0);

       // for each w adjacent to v...
       for (int i=0; i < dest->getNumInLinks(); i++)
       {
           if (!(dest->getLinkIn(i)->isEnabled())) continue;

           Node *src = dest->getLinkIn(i)->getRemoteNode();
           if (!src->isEnabled()) continue;

           double linkWeight = dest->getLinkIn(i)->getWeight();
           ASSERT(linkWeight > 0.0);

           double newdist = ((WNode *)dest)->dist + linkWeight;
           if (dest != target)
               newdist += dest->getWeight();  // dest is not the target, uses weight of dest node as price of routing (infinity means dest node doesn't route between interfaces)
           if (newdist != INFINITY && ((WNode *)src)->dist > newdist)  // it's a valid shorter path from src to target node
           {
               if (((WNode *)src)->dist != INFINITY)
                   q.remove(src);   // src is in the queue
               ((WNode *)src)->dist = newdist;
               ((WNode *)src)->out_path = ((WNode *)dest)->in_links[i];

               // insert src node to ordered list
               std::list<Node*>::iterator it;
               for (it = q.begin(); it != q.end(); ++it)
                   if (((WNode *)(*it))->dist > newdist)
                       break;
               q.insert(it, src);
           }
       }
    }
}

#endif // OMNETPP_VERSION < 0x0403
