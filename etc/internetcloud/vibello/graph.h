#ifndef VIBELLO_GRAPH_H
#define VIBELLO_GRAPH_H

#include "vec_t.h"
#include "xlib++/shapes.hpp"

#include <set>
#include <map>
#include <stdint.h>
#include <vector>

namespace xlib {
class window;
}

namespace vibello
{
class Graph
{
public:
    class Node
    {
        struct Link
        {
            Link(Node* _node, double _dist)
            : node(_node), dist(_dist) { }
            Node* node;
            double dist;
        };

    public:
        const uint32_t id;
        vec_t pos;
        vec_t total_displacement;

        typedef std::vector<Link> links_t;
        links_t links;
        double error;

    public:

        Node(uint32_t _id, const vec_t& _coords)
        : id(_id),
        pos(_coords),
        links(),
        error(10) { }

        void addLink(Node*n, double _dist);
        void print(std::ostream& os) const;
    };


public:

    Graph(const char* hosts_filename, const char* latencies_filename);

    inline void setView(xlib::window* _view) { win = _view; }

    void updateView();

    // Called by xlib
    void paint();

    // compute GNP coordinates
    void vibello(size_t nthreads);

    void writeHosts();

private:

    void getRandomLocation(vec_t& res);

    Node& createNode(uint32_t id);

    void createLink(uint32_t id0, uint32_t id1, double dist);

    xlib::point pos2point(const vec_t& v);

    void readHosts(const char* hosts_filename);

    void checkIds(const char* latencies_filename);

    void readLatencies(const char* latencies_filename);

    typedef std::map<uint32_t, Node*> nodes_t;
    typedef std::set<uint32_t> ids_t; // TODO: use array
    ids_t client_ids, server_ids;

    nodes_t nodes;
    xlib::window* win;
    vec_t min, max, scale;

    friend std::ostream&operator<<(std::ostream& os, const Node& n);
};

inline std::ostream&operator<<(std::ostream& os, const Graph::Node& n)
{
    n.print(os);
    return os;
}

}

#endif // not defined VIBELLO_GRAPH_H
