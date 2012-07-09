
#include "graph.h"

#include "util/concurrent_aggregation.h"
#include "util/partition.h"

#include <boost/lexical_cast.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
//#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using vibello::Graph;
using vibello::util::Max;
using vibello::util::partition;
using vibello::util::Sum;
using vibello::vec_t;

using std::binary_function;
using std::cout;
using std::endl;
using std::sort;
using std::unary_function;
using std::vector;

using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using boost::posix_time::not_a_date_time;

namespace {
struct SharedData
{

    SharedData(Graph& _world, double _tolerance,
               size_t _nthreads)
    : world(_world), tolerance(_tolerance), total_error(), iterations(0), last_time(), max_displace(1), nthreads(_nthreads), barrier(_nthreads) { }

    Graph& world;
    double tolerance;
    Sum<double> total_error;
    double prev_total_error;
    size_t iterations;
    ptime last_time;
    Max<double> max_displace;
    double prev_max_displace;
    boost::mutex io_mutex;

    size_t nthreads;
    boost::barrier barrier;
} ;

class Worker
{
public:

    /**
     * Each cycle is split into two phases. First the displacement for each node is computed. Then the nodes are updated.
     * Effort of phase I is roughly to the number of links, effort of phase II to the number of nodes.
     * First range is work load for phase I, second range is work load for phase II.
     */
    Worker(SharedData& _shared,
             vector<Graph::Node*>::const_iterator _begin,  vector<Graph::Node*>::const_iterator _end,
             vector<Graph::Node*>::const_iterator _begin2, vector<Graph::Node*>::const_iterator _end2)
    : shared(_shared),
    begin(_begin), end(_end),
    begin2(_begin2), end2(_end2) { }

    inline void operator()()
    {
        const double ce(0.1); // weight of current sample for updating local error ?
        const double cc(0.25); // weight for computing displacement
        double prev_max_displace;

        do
        { // main loop
            // ********** PHASE I  (cont'd) **********
            double local_total_error(0);
            for (n_t::const_iterator iti = begin; iti!=end; ++iti)
            {
                Graph::Node&i(**iti);
                i.total_displacement = vec_t();
                //		    cout << i.id << " " << i.pos << endl;
                for (Graph::Node::links_t::const_iterator itj = i.links.begin(); itj!=i.links.end(); ++itj)
                {
                    Graph::Node&j(*itj->node);
                    const double L(itj->dist);
                    //		        cout << " " << j.id << " " << j.pos << endl;

                    // Sample weight balances local and remote error
                    double w(i.error/(i.error+j.error));

                    const vec_t diff(i.pos-j.pos);
                    const double abs_diff(abs(diff));

                    // Compute error/force of this spring
                    double e = L-abs_diff;

                    // Relative error of this sample
                    double es = e/L;

                    // Update weighted moveing average of local error
                    i.error = es*ce*w+i.error*(1-ce*w);

                    //double delta = cc*w;
                    double delta = 0.0001;
                    //vec_t fij = (-e)*u(diff);
                    vec_t fij = e*u(diff);

                    // 		        cout << " diff:" << diff << endl;
                    // 		        cout << " e:" << e << endl;
                    // 		        cout << " u(diff):" << u(diff) << endl;
                    // 		        cout << " Fij:" << fij << endl;

                    // Add the force vector of this spring to the total force
                    //total_displacement += delta*(-e)*u(diff);
                    i.total_displacement += delta*fij;

                    if (i.id<j.id)
                        local_total_error += e*e;
                }
                //		    cout << " Disp:" << total_displacement << endl;
            }
            shared.total_error += local_total_error;


            // ********** PHASE II **********
            // displacement is constant; Position & max_displace are updated
            if (shared.barrier.wait())
            {
                shared.prev_total_error = shared.total_error;
                shared.total_error = 0;
                ++shared.iterations;
            }
            prev_max_displace = shared.prev_max_displace;
            double local_max_displace(0);
            for (n_t::const_iterator iti = begin2; iti!=end2; ++iti)
            {
                Graph::Node&i(**iti);
                i.pos += i.total_displacement;
                local_max_displace = std::max(abs(i.total_displacement), local_max_displace);
            }
            shared.max_displace.update(local_max_displace);

            // ********** PHASE I  **********
            // Positions & max_displace are constant; displacement is computed
            if (shared.barrier.wait())
            {
                shared.prev_max_displace = shared.max_displace;
                shared.max_displace = Max<double>(0.0);
                if ((shared.iterations%100)==0)
                {
                    cout << "iterations:" << shared.iterations << " E:" << shared.prev_total_error << " md:" << prev_max_displace << endl;
                    ptime t(microsec_clock::universal_time());

                    if (!shared.last_time.is_not_a_date_time())
                    {
                        long duration((t-shared.last_time).total_microseconds());
                        cout << (100*1.0E06/duration) << " iterations per second" << endl;
                    }
                    shared.last_time = t;

                    shared.world.updateView();
                    if ((shared.iterations%10000)==0)
                    {
                        shared.world.writeHosts();
                    }
                }
            }
        } while (shared.iterations<1000||shared.prev_total_error>shared.tolerance&&prev_max_displace>0.0001);
    }

private:
    SharedData& shared;
    typedef std::vector<Graph::Node*> n_t;
    n_t::const_iterator begin, end, begin2, end2;
} ;

struct estimator : public unary_function<Graph::Node*, size_t>
{

    size_t operator() (const Graph::Node*x) const
    {
        return x->links.size();
    }
};

} // anonymous namespace

void Graph::vibello(size_t nthreads)
{
    // use a vector for speed
    typedef std::vector<Graph::Node*> n_t;
    n_t ns; // just the values of nodes

    cout << "Copying nodes.." << endl;
    for (nodes_t::const_iterator i = nodes.begin(); i!=nodes.end(); ++i)
    {
        ns.push_back(i->second);
        //            edges += i->links.size();
    }

    boost::thread_group threadGroup;
    SharedData shared(*this, 0.01, nthreads);

    cout << "Partitioning work into " << nthreads << " work packages..." << endl;

    typedef std::vector<n_t::iterator> partends_t;
    partends_t partends;
    partends.resize(nthreads);
    partition<n_t::iterator, partends_t::iterator, estimator>(ns.begin(), ns.end(), partends.begin(), partends.end());

    size_t workload = (ns.size()+nthreads-1)/nthreads;

    n_t::const_iterator b = ns.begin();
    n_t::const_iterator begin = ns.begin();
    partends_t::const_iterator p = partends.begin();
    for (size_t t = 0; t<nthreads; ++t)
    {
        size_t end_idx = std::min((t+1)*workload, ns.size());
        cout << "Thread" << t << ": " << t*workload << " - " << end_idx << endl;
        std::vector<Node*>::const_iterator b2(&ns[t*workload]);
        std::vector<Node*>::const_iterator e2(&ns[end_idx]);
        Worker worker(shared, b, *p,
                   b2, e2);
        b = *p;
        ++p;
        threadGroup.create_thread(worker);
    }

    threadGroup.join_all();

    cout << "Convergence after " << shared.iterations << " iterations."<< endl;
    cout << "Total squared error " << shared.total_error << endl;
    writeHosts();
    cout << "Done." << endl;
}
