import omnetpp
from omnetpp.repl import *

import inet
from inet import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

def run_repl_main():
    try:
        args = parse_run_repl_arguments()
        kwargs = process_run_repl_arguments(args)
        if "-h" in sys.argv:
            sys.exit(0)
        else:
            _logger.info("OMNeT++ Python support is loaded.")
            IPython.embed(banner1="", colors="neutral")
    except KeyboardInterrupt:
        _logger.warn("Program interrupted by user")
    except Exception as e:
        if args.handle_exception:
            _logger.error(str(e))
        else:
            raise e
