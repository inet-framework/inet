import cppyy
import logging
import re

from inet.common import *
from inet.simulation.project import *
from inet.simulation.cffi.libomnetpp import *

_logger = logging.getLogger(__name__)

class PythonCmdenv(Cmdenv):
    def loadNEDFiles(self):
        pass

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
            CodeFragments.executeAll(CodeFragments.STARTUP)
            SimTime.setScaleExp(-12)
            static_flag = cStaticFlag()
            static_flag.__python_owns__ = False
            library_full_path = simulation_project.get_full_path(simulation_project.library_folder)
            _logger.info("Adding library path: " + library_full_path)
            cppyy.add_library_path(library_full_path)
            libsuffix = ""
            for dynamic_library in simulation_project.dynamic_libraries:
                _logger.info("Loading C++ library: " + dynamic_library + libsuffix)
                cppyy.load_library("lib" + dynamic_library + libsuffix)
            for external_library_folder in simulation_project.external_library_folders:
                external_library_full_path = simulation_project.get_full_path(external_library_folder)
                _logger.info("Adding external library path: " + external_library_full_path)
                cppyy.add_library_path(external_library_full_path)
            CodeFragments.executeAll(CodeFragments.STARTUP)
            for ned_folder in simulation_project.ned_folders:
                ned_full_path = simulation_project.get_full_path(ned_folder)
                _logger.info("Loading NED files: " + ned_full_path)
                cSimulation.loadNedSourceFolder(ned_full_path)
            cSimulation.doneLoadingNedFiles()
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
        iniReader = InifileReader()
        iniReader.readFile(simulation_config.ini_file)
        configuration = SectionBasedConfiguration()
        configuration.setConfigurationReader(iniReader)
        options = {}
        if "--release" in args:
            args.remove("--release")
        if "--debug" in args:
            args.remove("--debug")
        args += ["--cmdenv-output-file=/dev/null", "--cmdenv-redirect-output=true"]
        for arg in args:
            match = re.match(r"--(.+)=(.+)", arg)
            if match:
                options[match.group(1)] = match.group(2)
        configuration.setCommandLineConfigOptions(options, ".")
        iniReader.__python_owns__ = False
        environment = PythonCmdenv()
        environment.loggingEnabled = False
        simulation = cSimulation("simulation", environment)
        environment.__python_owns__ = False
        cSimulation.setActiveSimulation(simulation)
        environment.run.__release_gil__ = False
        returncode = environment.run(args, configuration)
        cSimulation.setActiveSimulation(cppyy.nullptr)
        simulation.__python_owns__ = False
        os.chdir(old_working_directory)
        return InprocessResult(returncode)
