#include "graph.h"

#include "xlib++/event_dispatcher.hpp"
#include "xlib++/window.hpp"

#include <boost/program_options.hpp>

using vibello::Graph;
using xlib::window;
using xlib::display;
using xlib::event_dispatcher;
using xlib::exception_with_text;
namespace po = boost::program_options;

using std::cout;
using std::cerr;
using std::endl;

namespace {

class MainWindow : public window
{
public:
    Graph& world;
    display& disp;

    MainWindow(Graph& _world, display& _display, event_dispatcher& e)
    : window(e),
    world(_world),
    disp(_display) {};

    ~MainWindow() {};

    void on_expose()
    {
        world.paint();
        XSync(disp, 0);
    }

};
}

int main(int argc, char** argv) {
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("threads,t", po::value<int>(), "set concurrency level")
            ("nox,X", "disable graphics");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        cout << desc << endl;
        return 0;
    }

    size_t nthreads(0);
    const char* s = std::getenv("CONCURRENCY_LEVEL");
    if (s && strlen(s)>0)
    {
        nthreads = boost::lexical_cast<size_t >(s);
    }

    if (vm.count("threads"))
        nthreads = vm["threads"].as<int>();

    if (!nthreads)
    {
        cerr << "Use --threads or CONCURRENCY_LEVEL to set number of threads" << endl;
        exit(1);
    }

    Graph world("data/hosts.csv", "data/latencies.csv");

    if (!vm.count("nox")) {
        try {
            // Open a display.
            display d("vibello");
            event_dispatcher events(d);
            MainWindow w(world, d, events); // top-level
            world.setView(&w);
            events.run();

        } catch (exception_with_text& e) {

            cerr << "Exception: " << e.what() << endl
                 << "Use --nox to run headlessly." << endl;
            exit(1);
        }
    }
    world.vibello(nthreads);
    return 0;

    }
