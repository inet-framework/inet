import cppyy
import logging
import re

from omnetpp.runtime import *

from inet.common import *
from inet.simulation.project import *

_logger = logging.getLogger(__name__)

class InprocessResult:
    def __init__(self, returncode):
        self.returncode = returncode
        self.stdout = b""
        self.stderr = b""

    def __repr__(self):
        return repr(self)

class InprocessSimulationRunner:
    def __init__(self):
        self.simulation_projects = []

    def setup(self, simulation_project):
        if not simulation_project in self.simulation_projects:
            SimTime.setScaleExp(-12)
            static_flag = cStaticFlag()
            static_flag.__python_owns__ = False
            library_full_path = simulation_project.get_full_path(simulation_project.library_folder)
            _logger.debug("Adding library path: " + library_full_path)
            cppyy.add_library_path(library_full_path)
            libsuffix = "_dbg"
            for dynamic_library in simulation_project.dynamic_libraries:
                _logger.debug("Loading C++ library: " + dynamic_library + libsuffix)
                cppyy.load_library("lib" + dynamic_library + libsuffix)
            for external_library_folder in simulation_project.external_library_folders:
                external_library_full_path = simulation_project.get_full_path(external_library_folder)
                _logger.debug("Adding external library path: " + external_library_full_path)
                cppyy.add_library_path(external_library_full_path)
            CodeFragments.executeAll(CodeFragments.STARTUP)
            self.ned_loader = cNedLoader()
            self.ned_loader.__python_owns__ = False
            self.ned_loader.setNedPath("")
            for ned_folder in simulation_project.ned_folders:
                ned_full_path = simulation_project.get_full_path(ned_folder)
                _logger.debug("Loading NED files: " + ned_full_path)
                self.ned_loader.loadNedFolder(ned_full_path)
            self.simulation_projects.append(simulation_project)

    def teardown(self):
        CodeFragments.executeAll(CodeFragments.SHUTDOWN)

    def run(self, simulation_task, args, capture_output=True):
        old_working_directory = os.getcwd()
        simulation_config = simulation_task.simulation_config
        simulation_project = simulation_config.simulation_project
        working_directory = simulation_config.working_directory
        full_working_directory = simulation_project.get_full_path(working_directory)
        # TODO change working directory is not thread safe, because it is stored per-process
        os.chdir(full_working_directory)
        self.setup(simulation_project)
        inifile_contents = InifileContents(simulation_config.ini_file)
        config = inifile_contents.extractConfig(simulation_config.config)
        # options = {}
        # if "--release" in args:
        #     args.remove("--release")
        # if "--debug" in args:
        #     args.remove("--debug")
        # args += ["--cmdenv-output-file=/dev/null", "--cmdenv-redirect-output=true"]
        # for arg in args:
        #     match = re.match(r"--(.+)=(.+)", arg)
        #     if match:
        #         options[match.group(1)] = match.group(2)
        # config.setCommandLineConfigOptions(options, ".")
        simulation = cSimulation("simulation", self.ned_loader)
        simulation.setupNetwork(config)
        simulation.run.__release_gil__ = False
        returncode = simulation.run()
        os.chdir(old_working_directory)
        return InprocessResult(returncode)
