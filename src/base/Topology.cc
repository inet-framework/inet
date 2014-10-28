//
// Copyright (C) 1992-2012 Andras Varga
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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <deque>
#include <list>
#include <algorithm>
#include <sstream>
#include "Topology.h"
#include "PatternMatcher.h"
#include "stlutils.h"


Register_Class(Topology);


Topology::LinkIn *Topology::Node::getLinkIn(int i)
{
    if (i<0 || i>=(int)inLinks.size())
        throw cRuntimeError("Topology::Node::getLinkIn: invalid link index %d", i);
    return (Topology::LinkIn *)inLinks[i];
}

Topology::LinkOut *Topology::Node::getLinkOut(int i)
{
    if (i<0 || i>=(int)outLinks.size())
        throw cRuntimeError("Topology::Node::getLinkOut: invalid index %d", i);
    return (Topology::LinkOut *)outLinks[i];
}

//----

Topology::Topology(const char *name) : cOwnedObject(name)
{
    target = NULL;
}

Topology::Topology(const Topology& topo) : cOwnedObject(topo)
{
    throw cRuntimeError(this,"copy ctor not implemented yet");
}

Topology::~Topology()
{
    clear();
}

std::string Topology::info() const
{
    std::stringstream out;
    out << "n=" << nodes.size();
    return out.str();
}

void Topology::parsimPack(cCommBuffer *buffer)
{
    throw cRuntimeError(this,"parsimPack() not implemented");
}

void Topology::parsimUnpack(cCommBuffer *buffer)
{
    throw cRuntimeError(this,"parsimUnpack() not implemented");
}

Topology& Topology::operator=(const Topology&)
{
    throw cRuntimeError(this,"operator= not implemented yet");
}

void Topology::clear()
{
    for (int i=0; i<(int)nodes.size(); i++)
    {
        for (int j=0; j<(int)nodes[i]->outLinks.size(); j++)
            delete nodes[i]->outLinks[j];  // delete links from their source side
        delete nodes[i];
    }
    nodes.clear();
}

//---

static bool selectByModulePath(cModule *mod, void *data)
{
    using inet::PatternMatcher;

    // actually, this is selectByModuleFullPathPattern()
    const std::vector<std::string>& v = *(const std::vector<std::string> *)data;
    std::string path = mod->getFullPath();
    for (int i=0; i<(int)v.size(); i++)
        if (PatternMatcher(v[i].c_str(), true, true, true).matches(path.c_str()))
            return true;
    return false;
}

static bool selectByNedTypeName(cModule *mod, void *data)
{
    const std::vector<std::string>& v = *(const std::vector<std::string> *)data;
    return std::find(v.begin(), v.end(), mod->getNedTypeName()) != v.end();
}

static bool selectByProperty(cModule *mod, void *data)
{
    struct ParamData {const char *name; const char *value;};
    ParamData *d = (ParamData *)data;
    cProperty *prop = mod->getProperties()->get(d->name);
    if (!prop)
        return false;
    const char *value = prop->getValue(cProperty::DEFAULTKEY, 0);
    if (d->value)
        return opp_strcmp(value, d->value)==0;
    else
        return opp_strcmp(value, "false")!=0;
}

static bool selectByParameter(cModule *mod, void *data)
{
    struct PropertyData{const char *name; const char *value;};
    PropertyData *d = (PropertyData *)data;
    return mod->hasPar(d->name) && (d->value==NULL || mod->par(d->name).str()==std::string(d->value));
}

//---

void Topology::extractByModulePath(const std::vector<std::string>& fullPathPatterns)
{
    extractFromNetwork(selectByModulePath, (void *)&fullPathPatterns);
}

void Topology::extractByNedTypeName(const std::vector<std::string>& nedTypeNames)
{
    extractFromNetwork(selectByNedTypeName, (void *)&nedTypeNames);
}

void Topology::extractByProperty(const char *propertyName, const char *value)
{
    struct {const char *name; const char *value;} data = {propertyName, value};
    extractFromNetwork(selectByProperty, (void *)&data);
}

void Topology::extractByParameter(const char *paramName, const char *paramValue)
{
    struct {const char *name; const char *value;} data = {paramName, paramValue};
    extractFromNetwork(selectByParameter, (void *)&data);
}

//---

static bool selectByPredicate(cModule *mod, void *data)
{
    Topology::Predicate *predicate = (Topology::Predicate *)data;
    return predicate->matches(mod);
}

void Topology::extractFromNetwork(Predicate *predicate)
{
    extractFromNetwork(selectByPredicate, (void *)predicate);
}

void Topology::extractFromNetwork(bool (*predicate)(cModule *,void *), void *data)
{
    clear();

    // Loop through all modules and find those that satisfy the criteria
#if OMNETPP_VERSION < 0x500
    for (int modId=0; modId<=simulation.getLastModuleId(); modId++)
#else
    for (int modId=0; modId<=simulation.getLastComponentId(); modId++)
#endif
    {
        cModule *module = simulation.getModule(modId);
        if (module && predicate(module, data)) {
            Node *node = createNode(module);
            nodes.push_back(node);
        }
    }

    // Discover out neighbors too.
    for (int k=0; k<(int)nodes.size(); k++)
    {
        // Loop through all its gates and find those which come
        // from or go to modules included in the topology.

        Node *node = nodes[k];
        cModule *mod = simulation.getModule(node->moduleId);

        for (cModule::GateIterator i(mod); !i.end(); i++)
        {
            cGate *gate = i();
            if (gate->getType()!=cGate::OUTPUT)
                continue;

            // follow path
            cGate *srcGate = gate;
            do {
                gate = gate->getNextGate();
            }
            while(gate && !predicate(gate->getOwnerModule(),data));

            // if we arrived at a module in the topology, record it.
            if (gate)
            {
                Link *link = createLink();
                link->srcNode = node;
                link->srcGateId = srcGate->getId();
                link->destNode = getNodeFor(gate->getOwnerModule());
                link->destGateId = gate->getId();
                node->outLinks.push_back(link);
            }
        }
    }

    // fill inLinks vectors
    for (int k=0; k<(int)nodes.size(); k++)
    {
        for (int l=0; l<(int)nodes[k]->outLinks.size(); l++)
        {
            Topology::Link *link = nodes[k]->outLinks[l];
            link->destNode->inLinks.push_back(link);
        }
    }
}

int Topology::addNode(Node *node)
{
    if (node->moduleId == -1)
    {
        // elements without module ID are stored at the end
        nodes.push_back(node);
        return nodes.size() - 1;
    }
    else
    {
        // must find an insertion point because nodes[] is ordered by module ID
        std::vector<Node*>::iterator it = std::lower_bound(nodes.begin(), nodes.end(), node, lessByModuleId);
        it = nodes.insert(it, node);
        return it - nodes.begin();
    }
}

void Topology::deleteNode(Node *node)
{
    // remove outgoing links
    for (int i=0; i<(int)node->outLinks.size(); i++) {
        Link *link = node->outLinks[i];
        unlinkFromDestNode(link);
        delete link;
    }
    node->outLinks.clear();

    // remove incoming links
    for (int i=0; i<(int)node->inLinks.size(); i++) {
        Link *link = node->inLinks[i];
        unlinkFromSourceNode(link);
        delete link;
    }
    node->inLinks.clear();

    // remove from nodes[]
    std::vector<Node*>::iterator it = find(nodes, node);
    ASSERT(it != nodes.end());
    nodes.erase(it);

    delete node;
}

void Topology::addLink(Link *link, Node *srcNode, Node *destNode)
{
    // remove from graph if it's already in
    if (link->srcNode)
        unlinkFromSourceNode(link);
    if (link->destNode)
        unlinkFromDestNode(link);

    // insert
    if (link->srcNode != srcNode)
        link->srcGateId = -1;
    if (link->destNode != destNode)
        link->destGateId = -1;
    link->srcNode = srcNode;
    link->destNode = destNode;
    srcNode->outLinks.push_back(link);
    destNode->inLinks.push_back(link);
}

void Topology::addLink(Link *link, cGate *srcGate, cGate *destGate)
{
    // remove from graph if it's already in
    if (link->srcNode)
        unlinkFromSourceNode(link);
    if (link->destNode)
        unlinkFromDestNode(link);

    // insert
    Node *srcNode = getNodeFor(srcGate->getOwnerModule());
    Node *destNode = getNodeFor(destGate->getOwnerModule());
    if (!srcNode)
        throw cRuntimeError("cTopology::addLink: module of source gate \"%s\" is not in the graph", srcGate->getFullPath().c_str());
    if (!destNode)
        throw cRuntimeError("cTopology::addLink: module of destination gate \"%s\" is not in the graph", destGate->getFullPath().c_str());
    link->srcNode = srcNode;
    link->destNode = destNode;
    link->srcGateId = srcGate->getId();
    link->destGateId = destGate->getId();
    srcNode->outLinks.push_back(link);
    destNode->inLinks.push_back(link);
}

void Topology::deleteLink(Link *link)
{
    unlinkFromSourceNode(link);
    unlinkFromDestNode(link);
    delete link;
}

void Topology::unlinkFromSourceNode(Link *link)
{
    std::vector<Link*>& srcOutLinks = link->srcNode->outLinks;
    std::vector<Link*>::iterator it = find(srcOutLinks, link);
    ASSERT(it != srcOutLinks.end());
    srcOutLinks.erase(it);
}

void Topology::unlinkFromDestNode(Link *link)
{
    std::vector<Link*>& destInLinks = link->destNode->inLinks;
    std::vector<Link*>::iterator it = find(destInLinks, link);
    ASSERT(it != destInLinks.end());
    destInLinks.erase(it);
}


Topology::Node *Topology::getNode(int i)
{
    if (i<0 || i>=(int)nodes.size())
        throw cRuntimeError(this,"invalid node index %d",i);
    return nodes[i];
}

Topology::Node *Topology::getNodeFor(cModule *mod)
{
    // binary search because nodes[] is ordered by module ID
    Node tmpNode(mod->getId());
    std::vector<Node*>::iterator it = std::lower_bound(nodes.begin(), nodes.end(), &tmpNode, lessByModuleId);
//TODO: this does not compile with VC9 (VC10 is OK): std::vector<Node*>::iterator it = std::lower_bound(nodes.begin(), nodes.end(), mod->getId(), isModuleIdLess);
    return it==nodes.end() || (*it)->moduleId != mod->getId() ? NULL : *it;
}

void Topology::calculateUnweightedSingleShortestPathsTo(Node *_target)
{
    // multiple paths not supported :-(

    if (!_target)
        throw cRuntimeError(this,"..ShortestPathTo(): target node is NULL");
    target = _target;

    for (int i=0; i<(int)nodes.size(); i++)
    {
       nodes[i]->dist = INFINITY;
       nodes[i]->outPath = NULL;
    }
    target->dist = 0;

    std::deque<Node*> q;

    q.push_back(target);

    while (!q.empty())
    {
       Node *v = q.front();
       q.pop_front();

       // for each w adjacent to v...
       for (int i=0; i<(int)v->inLinks.size(); i++)
       {
           if (!v->inLinks[i]->enabled)
               continue;

           Node *w = v->inLinks[i]->srcNode;
           if (!w->enabled)
               continue;

           if (w->dist == INFINITY)
           {
               w->dist = v->dist + 1;
               w->outPath = v->inLinks[i];
               q.push_back(w);
           }
       }
    }
}

void Topology::calculateWeightedSingleShortestPathsTo(Node *_target)
{
    if (!_target)
        throw cRuntimeError(this,"..ShortestPathTo(): target node is NULL");
    target = _target;

    // clean path infos
    for (int i=0; i<(int)nodes.size(); i++)
    {
       nodes[i]->dist = INFINITY;
       nodes[i]->outPath = NULL;
    }

    target->dist = 0;

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
            if (!(dest->getLinkIn(i)->isEnabled()))
                continue;

            Node *src = dest->getLinkIn(i)->getRemoteNode();
            if (!src->isEnabled())
                continue;

            double linkWeight = dest->getLinkIn(i)->getWeight();
            ASSERT(linkWeight > 0.0);

            double newdist = dest->dist + linkWeight;
            if (dest != target)
                newdist += dest->getWeight();  // dest is not the target, uses weight of dest node as price of routing (infinity means dest node doesn't route between interfaces)
            if (newdist != INFINITY && src->dist > newdist)  // it's a valid shorter path from src to target node
            {
                if (src->dist != INFINITY)
                    q.remove(src);   // src is in the queue
                src->dist = newdist;
                src->outPath = dest->inLinks[i];

                // insert src node to ordered list
                std::list<Node*>::iterator it;
                for (it = q.begin(); it != q.end(); ++it)
                    if ((*it)->dist > newdist)
                        break;
                q.insert(it, src);
            }
        }
    }
}

