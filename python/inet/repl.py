import argparse
import IPython
import logging
import omnetpp

from omnetpp.scave.analysis import *
from omnetpp.scave.results import *

from inet.project.omnetpp import *
from inet.simulation import *
from inet.test import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

def parse_run_repl_arguments():
    description = "Starts the OMNeT++ Python read-eval-print-loop."
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("-p", "--simulation-project", default=None, help="specifies the name of the project")
    parser.add_argument("-l", "--log-level", choices=["ERROR", "WARN", "INFO", "DEBUG"], default="INFO", help="specifies the log level for the root logging category")
    parser.add_argument("--handle-exception", default=True, action=argparse.BooleanOptionalAction, help="disables displaying stacktraces for exceptions")
    return parser.parse_args(sys.argv[1:])

def process_run_repl_arguments(args):
    initialize_logging(args.log_level)
    logging.getLogger("distributed.deploy.ssh").setLevel(args.log_level)
    define_sample_projects()
    simulation_project = determine_default_simulation_project(name=args.simulation_project, required=False)

def run_repl_main():
    try:
        args = parse_run_repl_arguments()
        kwargs = process_run_repl_arguments(args)
        if "-h" in sys.argv:
            sys.exit(0)
        else:
            _logger.info("OMNeT++ Python support is loaded.")
            app = IPython.terminal.ipapp.TerminalIPythonApp.instance()
            app.display_banner = False
            app.exec_lines = ["from inet import *", "enable_autoreload()"]
            app.initialize(argv=[])
            app.start()
    except KeyboardInterrupt:
        _logger.warn("Program interrupted by user")
    except Exception as e:
        if args.handle_exception:
            _logger.error(str(e))
        else:
            raise e
