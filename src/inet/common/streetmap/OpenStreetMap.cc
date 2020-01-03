//
// Copyright (C) OpenSim Ltd.
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

#include "inet/common/streetmap/OpenStreetMap.h"

namespace inet {

namespace osm {

const char *Tags::get(const char *k) const
{
    for (size_t i = 0; i < kvpairs.size(); i += 2)
        if (strcmp(kvpairs[i], k) == 0)
            return kvpairs[i+1];
    return nullptr;
}

OpenStreetMap::~OpenStreetMap()
{
    releaseAllocations();
}

OpenStreetMap::OpenStreetMap(OpenStreetMap&& other)
{
    bounds = other.bounds;
    nodes = std::move(other.nodes);
    ways = std::move(other.ways);
    relations = std::move(other.relations);
    strings = other.strings;
    other.strings = nullptr;
}

void OpenStreetMap::releaseAllocations()
{
    for (const Way *way : ways)
        delete way;
    for (const Node *node : nodes)
        delete node;
    for (const Relation *relation : relations)
        delete relation;
     delete strings;
}

void OpenStreetMap::operator=(OpenStreetMap&& other)
{
    if (this != &other) {
        releaseAllocations();
        bounds = other.bounds;
        nodes = std::move(other.nodes);
        ways = std::move(other.ways);
        relations = std::move(other.relations);
        strings = other.strings;
        other.strings = nullptr;
    }
}

inline double parseDouble(const char *s)
{
    return std::strtod(s, nullptr);
}

inline id_t parseId(const char *s)
{
    return std::strtoll(s, nullptr, 10);
}

inline const char *nullToEmpty(const char *s)
{
    return s ? s : "";
}

inline bool isEmpty(const char *s)
{
    return !s || !s[0];
}

const char *OpenStreetMap::getPooled(const char *s)
{
    if (s == nullptr)
        return nullptr;
    auto it = strings->find(s);
    if (it == strings->end())
        it = strings->insert(s).first;
    return it->c_str();
}

void OpenStreetMap::parseTags(cXMLElement *parent, Tags& tags)
{
    for (cXMLElement *child : parent->getChildrenByTagName("tag")) {
        const char *k = child->getAttribute("k");
        const char *v = child->getAttribute("v");
        tags.add(getPooled(k), getPooled(v));
    }
}

OpenStreetMap OpenStreetMap::from(cXMLElement *mapRoot)
{
    OpenStreetMap map;
    std::map<id_t,Node*> nodeById;
    std::map<id_t,Way*> wayById;
    std::map<id_t,Relation*> relationById;

    cXMLElement *boundsElement = mapRoot->getFirstChildWithTag("bounds");
    Bounds& bounds = map.bounds;
    bounds.minlat = bounds.minlon = bounds.maxlat = bounds.maxlon = NAN;

    if (boundsElement) {
        bounds.minlat = parseDouble(boundsElement->getAttribute("minlat"));
        bounds.minlon = parseDouble(boundsElement->getAttribute("minlon"));
        bounds.maxlat = parseDouble(boundsElement->getAttribute("maxlat"));
        bounds.maxlon = parseDouble(boundsElement->getAttribute("maxlon"));
    }

    for (cXMLElement *nodeElem : mapRoot->getChildrenByTagName("node")) {
        Node *node = new Node();
        node->id = parseId(nodeElem->getAttribute("id"));
        node->lat = parseDouble(nodeElem->getAttribute("lat"));
        node->lon = parseDouble(nodeElem->getAttribute("lon"));
        map.parseTags(nodeElem, node->tags);
        map.nodes.push_back(node);
        nodeById[node->id] = node;
    }

    for (cXMLElement *wayElem : mapRoot->getChildrenByTagName("way")) {
        Way *way = new Way();
        way->id = parseId(wayElem->getAttribute("id"));
        for (cXMLElement *nodeElem : wayElem->getChildrenByTagName("nd")) {
            id_t ref = parseId(nodeElem->getAttribute("ref"));
            auto it = nodeById.find(ref);
            if (it == nodeById.end())
                throw cRuntimeError("Referenced node not found at %s", nodeElem->getSourceLocation());
            Node *node = it->second;
            way->nodes.push_back(node);
        }
        map.parseTags(wayElem, way->tags);
        map.ways.push_back(way);
        wayById[way->id] = way;
    }

    for (cXMLElement *relationElem : mapRoot->getChildrenByTagName("relation")) {
        Relation *relation = new Relation();
        relation->id = parseId(relationElem->getAttribute("id"));
        for (cXMLElement *memberElem : relationElem->getChildrenByTagName("member")) {
            Member member;
            const char *type = memberElem->getAttribute("type");
            id_t ref = parseId(memberElem->getAttribute("ref"));
            if (strcmp(type, "node")==0) {
                member.type = Member::NODE;
                auto it = nodeById.find(ref);
                member.resolved = (it != nodeById.end());
                if (member.resolved)
                    member.node = it->second;
                else
                    member.unresolvedId = ref;
            }
            else if (strcmp(type, "way")==0) {
                member.type = Member::WAY;
                auto it = wayById.find(ref);
                member.resolved = (it != wayById.end());
                if (member.resolved)
                    member.way = it->second;
                else
                    member.unresolvedId = ref;
            }
            else if (strcmp(type, "relation")==0) {
                member.type = Member::RELATION;
                auto it = relationById.find(ref);
                member.resolved = (it != relationById.end());
                if (member.resolved)
                    member.relation = it->second;
                else
                    member.unresolvedId = ref;
            }
            else {
                throw cRuntimeError("Invalid member type '%s' at %s", type, memberElem->getSourceLocation());
            }
            member.role = map.getPooled(memberElem->getAttribute("role"));
            relation->members.push_back(member);
        }
        map.parseTags(relationElem, relation->tags);
        map.relations.push_back(relation);
        relationById[relation->id] = relation;
    }

    // resolve references to relations defined out of order
    for (const Relation *relation : map.relations) {
        for (Member& member : const_cast<Relation*>(relation)->members) {
            if (member.type == Member::RELATION && !member.resolved) {
                auto it = relationById.find(member.unresolvedId);
                if (it != relationById.end()) {
                    member.resolved = true;
                    member.relation = it->second;
                }
            }
        }
    }

    return map;
}

} // namespace osm

} // namespace inet

