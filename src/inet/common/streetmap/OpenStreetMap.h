//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OPENSTREETMAP_H
#define __INET_OPENSTREETMAP_H

#include "inet/common/INETDefs.h"

namespace inet {

namespace osm {

typedef int64_t id_t;

class INET_API Tags
{
  private:
    std::vector<const char *> kvpairs;

  public:
    void add(const char *k, const char *v) { kvpairs.push_back(k); kvpairs.push_back(v); }
    const char *get(const char *k) const;
};

class INET_API Node
{
  private:
    friend class OpenStreetMap;
    id_t id;
    double lat, lon;
    Tags tags;

  public:
    id_t getId() const { return id; }
    double getLat() const { return lat; }
    double getLon() const { return lon; }
    const char *getTag(const char *k) const { return tags.get(k); }
};

class INET_API Way
{
  private:
    friend class OpenStreetMap;
    id_t id;
    std::vector<const Node *> nodes;
    Tags tags;

  public:
    id_t getId() const { return id; }
    const std::vector<const Node *>& getNodes() const { return nodes; }
    const char *getTag(const char *k) const { return tags.get(k); }
};

class Relation;

class INET_API Member
{
  private:
    friend class OpenStreetMap;
    enum Type { NODE, WAY, RELATION } type;
    bool resolved;
    union {
        Node *node;
        Way *way;
        Relation *relation;
        id_t unresolvedId;
    };
    const char *role;

  public:
    Type getType() const { return type; }
    bool isResolved() const { return resolved; }
    Node *getNode() const { return type == NODE && resolved ? node : nullptr; }
    Way *getWay() const { return type == WAY && resolved ? way : nullptr; }
    Relation *getRelation() const { return type == RELATION && resolved ? relation : nullptr; }
    id_t getUnresolvedId() const { return resolved ? 0 : unresolvedId; }
    const char *getRole() const { return role; }
};

class INET_API Relation
{
  private:
    friend class OpenStreetMap;
    id_t id;
    std::vector<Member> members;
    Tags tags;

  public:
    id_t getId() const { return id; }
    const std::vector<Member>& getMembers() const { return members; }
    const char *getTag(const char *k) const { return tags.get(k); }
};

struct Bounds
{
    double minlat, minlon, maxlat, maxlon;
};

/**
 * Represents OpenStreetMap map data, as loaded from an OSM file.
 * OSM files can be obtained e.g. by exporting from http://openstreetmap.org.
 */
class INET_API OpenStreetMap
{
  private:
    Bounds bounds;
    std::vector<const Node *> nodes;
    std::vector<const Way *> ways;
    std::vector<const Relation *> relations;
    std::set<std::string> *strings = new std::set<std::string>();
    const char *getPooled(const char *s);
    void releaseAllocations();
    void parseTags(cXMLElement *parent, Tags& intoTags);

  public:
    OpenStreetMap() {}
    OpenStreetMap(const OpenStreetMap&) = delete;
    OpenStreetMap(OpenStreetMap&& other);
    ~OpenStreetMap();
    void operator=(const OpenStreetMap&) = delete;
    void operator=(OpenStreetMap&&);
    const Bounds& getBounds() const { return bounds; };
    const std::vector<const Node *>& getNodes() const { return nodes; }
    const std::vector<const Way *>& getWays() const { return ways; }
    const std::vector<const Relation *>& getRelations() const { return relations; }
    static OpenStreetMap from(cXMLElement *mapRoot);
};

} // namespace osm

} // namespace inet

#endif

