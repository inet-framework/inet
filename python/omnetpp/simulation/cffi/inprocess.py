import cppyy
import re

from inet.common import *
from inet.simulation.cffi.inet import *
from inet.simulation.cffi.omnetpp import *

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
    def __init__(selfs):
        pass

    def setup(self):
        CodeFragments.executeAll(CodeFragments.STARTUP)
        SimTime.setScaleExp(-12)
        static_flag = cStaticFlag()
        static_flag.__python_owns__ = False
        cSimulation.loadNedSourceFolder(get_workspace_path("inet/src"))
        cSimulation.loadNedSourceFolder(get_workspace_path("inet/examples"))
        cSimulation.loadNedSourceFolder(get_workspace_path("inet/showcases"))
        cSimulation.loadNedSourceFolder(get_workspace_path("inet/tutorials"))
        cSimulation.loadNedSourceFolder(get_workspace_path("inet/tests/networks"))
        cSimulation.doneLoadingNedFiles()

    def teardown(self):
        CodeFragments.executeAll(CodeFragments.SHUTDOWN)

    def run(self, simulation_task, args, capture_output=True):
        old_working_directory = os.getcwd()
        simulation_config = simulation_task.simulation_config
        simulation_project = simulation_config.simulation_project
        working_directory = simulation_config.working_directory
        full_working_directory = simulation_project.get_full_path(working_directory)
        os.chdir(full_working_directory)
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
        returncode = environment.run(args, configuration)
        cSimulation.setActiveSimulation(cppyy.nullptr)
        simulation.__python_owns__ = False
        os.chdir(old_working_directory)
        return InprocessResult(returncode)

inprocess_simulation_runner = InprocessSimulationRunner()
inprocess_simulation_runner.setup()
