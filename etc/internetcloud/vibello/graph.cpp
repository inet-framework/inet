#include "graph.h"

#include "xlib++/graphics_context.hpp"

#include "util/CountingOutputIterator.h"

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <cstdlib> // for getenv
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>

using vibello::D;
using vibello::Graph;
using vibello::util::CountingOutputIterator;
using vibello::vec_t;

using namespace xlib;

using boost::lexical_cast;
using boost::tokenizer;

using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::ostringstream;
using std::set;
using std::set_intersection;
using std::string;


namespace {
typedef tokenizer<boost::escaped_list_separator<char> > tokenizer_t;
}

void Graph::Node::addLink(Node* n, double dist)
{
    links.push_back(Link(n, dist));
}

void Graph::Node::print(ostream& os) const
{
    os << id;
    for (size_t i = 0; i < D; ++i)
        os << ", " << pos[i];
}

Graph::Graph(const char* hosts_filename, const char* latencies_filename)
: nodes(),
  win(0),
  min(),
  max(),
  scale()
{
    readHosts(hosts_filename);
    checkIds(latencies_filename);
    readLatencies(latencies_filename);
}

void Graph::readHosts(const char* hosts_filename)
{
    cout<<"Reading hosts..."<<flush;
    assert(D>0);
    ifstream ifs(hosts_filename);
    while (ifs)
    {
        string line;
        getline(ifs, line);
        if (line.empty())
            continue;
        tokenizer_t tok(line);
        tokenizer_t::iterator t = tok.begin();
        uint32_t id(lexical_cast<uint32_t>(*t++));
        double latitude = lexical_cast<double>(*t++);
        double longitude = lexical_cast<double>(*t++);

        Node& n = createNode(id);
        n.pos[0] = longitude*3;
        if (D>1)
            n.pos[1] = latitude*3;
    }
    cout << nodes.size() << " hosts read"<<endl;
}

void Graph::checkIds(const char* latencies_filename)
{
    cout<<"Checking latencies for unique client/server Ids..."<<flush;
    ifstream ifs(latencies_filename);
    size_t cnt(0);
    while (ifs)
    {
        string line;
        getline(ifs, line);
        if (line.empty())
            continue;
        tokenizer_t tok(line);
        tokenizer_t::iterator t = tok.begin();
        uint32_t client_id(lexical_cast<uint32_t>(*t++));
        uint32_t server_id(lexical_cast<uint32_t>(*t++));
        client_ids.insert(client_id);
        server_ids.insert(server_id);
    }

    CountingOutputIterator counter;
    set_intersection(client_ids.begin(), client_ids.end(),
                     server_ids.begin(), server_ids.end(),
                     counter);
    if (counter)
    {
        cerr<<"client_ids and server_ids have "<<counter<<" elements in common!"<<endl;
        throw 1;
    }
    cout<<"OK"<<endl;
}

void Graph::readLatencies(const char* latencies_filename)
{
    cout<<"Reading latencies..."<<flush;
    ifstream ifs(latencies_filename);
    size_t cnt(0), unknown_hosts(0);
    while (ifs)
    {
        string line;
        getline(ifs, line);
        if (line.empty())
            continue;
        tokenizer_t tok(line);
        tokenizer_t::iterator t = tok.begin();
        uint32_t client_id(lexical_cast<uint32_t>(*t++));
        uint32_t server_id(lexical_cast<uint32_t>(*t++));
        double latency = lexical_cast<double>(*t++);

        if (nodes.find(client_id)==nodes.end())
        {
            ++unknown_hosts;
            continue;
        }
        if (latency>2000)
            cout<<client_id<<" <-> "<<server_id<<" lat:"<<latency<<endl;
        createLink(client_id, server_id, latency);
        ++cnt;
    }
    cout<<cnt<<endl;
    cout<<"Dropping "<<unknown_hosts<<" unknown hosts"<<endl;
}


void Graph::getRandomLocation(vec_t& res)
{
    for (int i = 0; i<D; ++i)
        res[i] = drand48();
    //res /= abs(res);
}

Graph::Node& Graph::createNode(uint32_t id)
{
    vec_t coords;
    getRandomLocation(coords);
    Node* n(new Node(id, coords));
    nodes[id] = n;
    return *n;
}

void Graph::createLink(uint32_t id0, uint32_t id1, double dist)
{
    if (nodes.find(id0)==nodes.end())
        createNode(id0);
    if (nodes.find(id1)==nodes.end())
        createNode(id1);

    Node* n0 = nodes[id0];
    Node* n1 = nodes[id1];

    n0->addLink(n1, dist);
    n1->addLink(n0, dist);
}

vec_t min, max, scale;

point Graph::pos2point(const vec_t& v)
{
    return point((v[0]-min[0])*scale[0],
                 (max[1]-v[1])*scale[1]);
}

void Graph::updateView()
{
    if (win)
        win->refresh();
}

void Graph::paint()
{
    if (!win)
        return;

    for (int i = 0; i<D; ++i)
    {
        min[i] = 1e99;
        max[i] = 1e-99;
    }
    for (nodes_t::const_iterator n = nodes.begin(); n!=nodes.end(); ++n)
    {
        for (int i = 0; i<D; ++i)
        {
            min[i] = std::min(min[i], n->second->pos[i]);
            max[i] = std::max(max[i], n->second->pos[i]);

        }
        //cout << n->first << " " << n->second->pos << endl;
    }
    for (int i = 0; i<D; ++i)
    {
        scale[i] = 1000.0/(max[i]-min[i]);
        cout<<"min "<<min[i]<<"max"<<max[i]<<"scale"<<scale[i]<<endl;
    }

    graphics_context gc(win->get_display(),
                        win->id());

    color black(win->get_display(), 0, 0, 0);
    color red(win->get_display(), 255, 0, 0);
    for (nodes_t::const_iterator i=nodes.begin(); i!=nodes.end(); ++i)
    {
        //gc.draw_line ( line ( point(0,0), point(50,50) ) );
        point p(pos2point(i->second->pos));
        // 		ostringstream os;
        // 		os << i->first;
        // 		gc.draw_text( pos2point(i->second->pos, min, max), os.str() );
        if (i->first<1000)
        {
            gc.set_foreground(red);
            gc.draw_line(line(p, point(p.x()+1, p.y()+1)));
            gc.draw_line(line(point(p.x()+1, p.y()), point(p.x(), p.y()+1)));
        } else
        {
            gc.set_foreground(black);
            gc.draw_line(line(p, p));
        }
    }
}

void Graph::writeHosts()
{
    ofstream errors("data/errors.csv");
    ofstream ehosts("data/ehosts.csv");
    if (errors.fail()||ehosts.fail())
    {
        cerr<<"Error opening results file for writing!"<<endl;
        exit(-1);
    }
    errors << "client_ip_id, server_id, nominal, real"<<endl;
    ehosts << "client_ip_id";
    for (size_t i=0; i<D; ++i)
        ehosts << ", c" << i;
    ehosts << endl;

    for (ids_t::const_iterator iti = client_ids.begin(); iti!=client_ids.end(); ++iti)
    {
        Node& i(*nodes[*iti]);
        ehosts<<i<<endl;

        for (Node::links_t::const_iterator itj = i.links.begin(); itj!=i.links.end(); ++itj)
        {
            Node& j(*itj->node);
            const double L(itj->dist);
            const vec_t diff(i.pos-j.pos);
            const double abs_diff(abs(diff));
            errors<<i.id<<", "<<j.id<<", "<<L<<", "<<abs_diff<<" "<<endl;
        }
    }

    ehosts.close();
    errors.close();
    if (errors.fail()||ehosts.fail())
    {
        cerr<<"Error writing results file!"<<endl;
        throw 1;
    }
}
