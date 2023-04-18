"""
Main package for the INET Framework.

It provides sub-packages for running simulations, analyzing simulation results, generating documentation, automated
testing, etc.

Please note that undocumented features are not supposed to be used by the user.

.. TODO the following documentation should be merged into the simulation manual

INET Framework Python Module
############################

The following sections describe the Python module that comes with the latest INET Framework version.

Using Python Interactively
==========================

The Python programming language is a very adequate tool for interactive development. There are several reasons: running
Python code doesn't require compilation, the interactive code execution creates a bidirectional communication channel
between the user and the system, and the interactive development session is stateful, remembering code blocks executed
earlier and also their results. Moreover, Python comes with a plethora of open source libraries which are easy to install
and use in combination with the functions provided by the INET Framework Python module.

The easiest way to start using INET in the interactive Python environment is to start the :command:`inet_python_repl`
shell script. This script is pretty simple, it loads the :py:mod:`inet.repl` Python package and launches an IPython
interpreter. Alternatively, it's also possible to use any other Python interpreter, and also to import the desired
individual INET packages as needed. A good place to see how to setup the INET Python modules after importing them
is to look at how the aforementioned INET package is implemented.

The OMNeT++ IDE contains built-in interactive Python development support. This is provided in the form of a IPython
console view that automatically imports the :py:mod:`inet.repl` package and it's immediately ready to use when the
IDE is started.

Once the Python interpreter is up and running, the user can immediately start interacting with it and run simulations
and carry out many other tasks. The easiest way to get used to the interactive development, is to run the code fragments
presented in this document. Later with more experience, the user can run any other Python code that is developed with
the help of the INET Python API reference documentation.

Installation
============

The INET Python module requires certain other Python modules to be installed before it can be used. The following
command installs all such required and optional modules:

.. code-block:: console

    levy@valardohaeris:~/workspace/inet$ pip install cppyy dask distributed IPython matplotlib numpy pandas scipy
    Collecting cppyy
    ...

After the installation is completed, starting the INET Python interpreter from a terminal is pretty straightforward: 

.. code-block:: console

    levy@valardohaeris:~/workspace/inet$ . setenv
    Environment for 'omnetpp-6.0' in directory '/home/levy/workspace/omnetpp' is ready.
    levy@valardohaeris:~/workspace/inet$ inet_python_repl
    WARNING inet.simulation.project No enclosing simulation project is found from current working directory, and no simulation project is specified explicitly (project.py:510)
    INFO inet.repl OMNeT++ Python support is loaded. (repl.py:40)

.. note::

    The above warning isn't a problem, it simply states that there's no default simulation project.

When the Python interpreter starts the following prompt is displayed:

.. code-block:: ipython

    In [1]:

You can start typing in the prompt. For example, type `run` and press the TAB key to get the completion options. The
Python interpreter provides completion options for module and package names, class and function names, and method names
and their parameters. Each time you complete an input and press `ENTER` the interpreter executes the code and displays
the returned result:

.. code-block:: ipython

    In [1]: 2 + 2
    Out[1]: 4

Main Concepts
=============

The INET Python module contains many useful classes and functions. The following lists the most important concepts
represented by Python classes:

 - :py:class:`SimulationProject <inet.simulation.project.SimulationProject>`: represents a project that usually comes
   with its own modules, their C++ implementation, and also with several example simulations
 - :py:class:`SimulationConfig <inet.simulation.config.SimulationConfig>`: represents a specific configuration section
   from an INI file under a working directory in a specific simulation project
 - :py:class:`SimulationTask <inet.simulation.task.SimulationTask>`: represents a completely parameterized simulation
   using a simulation config which can be run multiple times
 - :py:class:`SimulationTaskResult <inet.simulation.task.SimulationTaskResult>`: represents the result that is created
   when a simulation task is run

.. note::

    Undocumented functions, classes and their methods are not supposed to be used by the user.

There are several other important concepts represented in Python: multiple simulation tasks and their results, smoke
tests, fingerprint tests, and so on.

Defining Projects
=================

The simulation project is an essential concept of the INET Python module. Many functions take a simulation project
as parameter. OMNeT++ comes with a set of sample projects which are already defined in the INET Python module.

For example, the following `.omnetpp` project definition file describes the sample simulation project called `aloha`.
The file can be found under the `aloha` sample project directory. It simply defines a simulation project with a root
director set to `samples/aloha` relative to the value of the `__omnetpp_root_dir` environment variable.

.. code-block:: json

    {"name": "aloha",
     "folder_environment_variable": "__omnetpp_root_dir",
     "folder": "samples/aloha"}

There are many other options that can be used in the project definition: C++ include folders, NED folders, external
libraries, etc. The simulation project could also be defined by calling the
:py:func:`define_simulation_project <inet.simulation.project.define_simulation_project>`
function. The :py:class:`SimulationProject <inet.simulation.project.SimulationProject>` class constructor has the
same set of parameters.

An important concept related to simulation projects is the default simulation project. Having a default project greatly
simplifies using several functions of the INET Python module by implicitly using the default project without always
explicitly passing it in as a parameter. The default simulation project is automatically set to the one enclosing the
current working directory when the Python interpreter is started. Alternatively, the default simulation project can also
be set explicitly using the :py:func:`set_default_simulation_project <inet.simulation.project.set_default_simulation_project>`
function.

Building Projects
=================

It's essential to make sure that the simulation project is built before running a simulation. OMNeT++ already provides
several ways to build your simulation project. You can build from the terminal using the :command:`make` command. You
can also use the OMNeT++ IDE, which has built-in support for automatically building the project before running a
simulation. Alternatively, you can also start the build manually from the IDE.

Unfortunately, none of the above can be done easily when you are working from the Python interpreter. To avoid running
a stale binary, the INET Python module also supports building the simulation project:

.. code-block:: ipython

    In [1]: p = get_simulation_project("aloha")

    In [2]: build_project(simulation_project=p)
    INFO inet.simulation.build Building aloha started (build.py:61)
    INFO inet.simulation.build Building aloha ended (build.py:67)

The :py:func:`build_project <inet.simulation.build.build_project>` function currently runs the :command:`make` command
in the project root directory. Similarly to the :command:`make` build system, you can also build the binaries in different
modes:

.. code-block:: ipython

    In [1]: build_project(simulation_project=p, mode="debug")
    ...

The INET Python module also contains a new build system that uses tasks instead of the :command:`make` build system.

.. code-block:: ipython

    In [1]: build_project(simulation_project=p, build_mode="task")
    ...

The main benefit of building projects like this is that you can easily add your own Python code to the build process or
even perform automatic cross project builds.

Running Simulations
===================

The most common task performed by users is running simulations. There are already several ways to do this: simulations
can be run individually from the IDE with a few mouse clicks, they can also be run individually from the command line
using :command:`opp_run` or the simulation model binary, and repetitions or parameter studies can be run in parallel
batches from the command line using :command:`opp_runall`.

But there are a few other use cases for running simulations. For example, running multiple unrelated simulations from
the same simulation project, that may have different working directories, INI files, and configurations. For another
example, running multiple simulations on a cluster of computers connected to a LAN. Ideally, all of these use cases,
including running single simulations, repetitions and parameter studies, should be provided for the users by a single
entry point in the toolchain.

The INET Python library contains a single function that covers all of the above tasks. This function is called
:py:meth:`run_simulations <inet.simulation.task.run_simulations>` and it is capable of running multiple simulations
matching the provided filter criteria. The simulations can be run sequentially or concurrently, on the local computer
or on an SSH cluster.

In the following we demonstrate this and other functions with a number of examples.

.. TODO

    filtering the tasks
    filtering the results
    explain store/restore task results (but comment out?)

The simplest example is running all simulations from a specific simulation project. In this context, all simulations
means all simulation runs from all configurations from all INI files found under the specific simulation project.

.. code-block:: ipython

    In [1]: run_simulations(simulation_project=get_simulation_project("fifo"))
    [3/7] . -c TandemQueueExperiment DONE
    [5/7] . -c TandemQueueExperiment -r 2 DONE
    ...
    [1/7] . -c Fifo1 DONE
    [2/7] . -c Fifo2 DONE
    Out[1]: Multiple simulation results: DONE, summary: 7 DONE in 0:00:15.642444

.. note::

    The order of simulation runs is random because they run in parallel utilizing all CPUs by default.

The result of running the above simulations is a :py:class:`MultipleTaskResults <inet.common.task.MultipleTaskResults>`
object, which is represented by a human readable summary in the console output. This object contains several details for
the individual task results and also for the total result.

In most cases, it's useful to set a default simulation project in the Python interpreter. This allows running simulations
from the same simulation project without always explicitly passing it in as a parameter:

.. code-block:: ipython

    In [1]: set_default_simulation_project(get_simulation_project("fifo"))

The same effect can be achieved simply by starting the Python interpreter from the directory of the desired simulation
project:

.. code-block:: console

    levy@valardohaeris:~/workspace/omnetpp/samples/fifo$ inet_python_repl
    INFO inet.simulation.project Default project is set to fifo (project.py:512)
    INFO inet.repl OMNeT++ Python support is loaded. (repl.py:40)

After the default simulation project is set, running all simulations can be done with a single parameterless function call:

.. code-block:: ipython

    In [1]: run_simulations()

.. note::

    The project is automatically built by :py:func:`run_simulations <inet.simulation.task.run_simulations>` unless
    the `build=False` parameter is used.

In some cases, running all simulations from a simulation project (e.g. aloha) may not terminate because one or more
simulations don't have a pre-configured simulation time limit in the respective INI files:

.. code-block:: ipython

    In [1]: run_simulations(simulation_project=get_simulation_project("aloha"))
    [44/49] . -c PureAlohaExperiment -r 39 DONE
    [34/49] . -c PureAlohaExperiment -r 29 DONE
    ...
    ^C[04/49] . -c PureAloha3 CANCEL (unexpected) (Cancel by user)
    [47/49] . -c SlottedAloha1 CANCEL (unexpected) (Cancel by user)
    ...
    [49/49] . -c SlottedAloha3 CANCEL (unexpected) (Cancel by user)
    [02/49] . -c PureAloha1 CANCEL (unexpected) (Cancel by user)
    Out[1]: Multiple simulation results: CANCEL, summary: 49 TOTAL, 42 DONE, 1 SKIP (expected), 6 CANCEL (unexpected) in 0:00:06.791716

Pressing Control-C (see ^C above) cancels the execution of the remaining simulations. The result object still contains
all simulation task results including those that were collected up to the cancellation point and also those describing
the cancelled simulation tasks.

Running a set of simulation configs from a specific simulation project up to a specific simulation time limit:

.. code-block:: ipython

    In [1]: r = run_simulations(config_filter="PureAloha", sim_time_limit="1s")
    [02/45] . -c PureAloha2 for 1s DONE
    [06/45] . -c PureAlohaExperiment -r 2 for 1s DONE
    ...
    [44/45] . -c PureAlohaExperiment -r 40 for 1s DONE
    [35/45] . -c PureAlohaExperiment -r 31 for 1s DONE

    In [2]: r
    Out[2]: Multiple simulation results: DONE, summary: 45 DONE in 0:00:00.144779

.. note::

    For more details on filter parameters see the :py:meth:`matches_filter <inet.simulation.config.SimulationConfig.matches_filter>`
    method.

Storing the result object allows the user to later re-run the same set of simulations with a simple method call:

.. code-block:: ipython

    In [1]: r = r.rerun()
    [02/45] . -c PureAloha2 for 1s DONE
    [03/45] . -c PureAloha3 for 1s DONE
    ...
    [40/45] . -c PureAlohaExperiment -r 36 for 1s DONE
    [41/45] . -c PureAlohaExperiment -r 37 for 1s DONE
    Out[1]: Multiple simulation results: DONE, summary: 45 DONE in 0:00:00.131351

You can also filter the result for the simulations which terminated with error and re-run only them:

.. code-block:: ipython

    In [5]: r.get_error_results().rerun()
    Out[5]: Empty simulation result

In this case, there were no tasks resulting in error, so there was nothing to do.

You can also control many other aspects of running simulations. The `mode` parameter allows choosing between release
and debug mode binaries, the `sim_time_limit` and `cpu_time_limit` parameters can be used to control the termination of
simulations, the `concurrent`, `scheduler`, and `simulation_runner` parameters can be used to control how and where
simulations are run, etc.

The :py:meth:`run_simulations <inet.simulation.task.run_simulations>` function (and all similar run functions) is
implemented using the :py:meth:`get_simulation_tasks <inet.simulation.task.get_simulation_tasks>` function (and other
similarly named functions). The latter simply returns a list of tasks that can be stored in variables, passed around in
functions, and run at any later moment, even multiple times if desired.

Running Simulations on a Cluster
================================

Running multiple simulations can be drastically sped up by utilizing multiple computers called a cluster. The INET
Python package provides direct support to use SSH clusters. An SSH cluster is a set of network nodes usually connected
to a single LAN, all of which can login into each other using SSH passwordless login. The SSH cluster utilizes all
network nodes with automatic and transparent load balancing as if it were a single computer.

The first step to use an SSH cluster is to create one by specifying the scheduler and worker hostnames and start it:

.. code-block:: ipython

    In [1]: c = SSHCluster(scheduler_hostname="valardohaeris.local", worker_hostnames=["valardohaeris.local", "valarmorghulis.local"])

    In [2]: c.start()
    INFO inet.common.cluster Starting SSH cluster: scheduler=valardohaeris, workers=['valardohaeris', 'valarmorghulis'] ...
    INFO asyncssh Opening SSH connection to valardohaeris.local, port 22 (logging.py:92)
    INFO asyncssh [conn=0] Connected to SSH server at valardohaeris.local, port 22 (logging.py:92)
    ...

After the SSH cluster is started, open the http://localhost:8797 web page and see the live dashboard. The dashboard
displays among others what the cluster is doing.

It is easy to check if the cluster is operating correctly by running the following:

.. code-block:: ipython

    In [1]: c.run_gethostname(12)
    Out[1]: 'valardohaeris, valarmorghulis, valarmorghulis, valarmorghulis, valarmorghulis, valardohaeris, valarmorghulis, valardohaeris, valardohaeris, valarmorghulis, valardohaeris, valardohaeris'

The result should contain a permutation of the hostnames of all worker nodes similarly to the above.

The next step to use the SSH cluster to run simulations is to build a simulation project, preferably in both release
and debug mode, and distribute the binary files to all worker nodes:

.. code-block:: ipython

    In [1]: p = get_simulation_project("aloha")

    In [2]: build_project(simulation_project=p, mode="release")
    INFO inet.simulation.build Building aloha started (build.py:61)
    ...
    INFO inet.simulation.build Building aloha ended (build.py:67)

    In [3]: p.copy_binary_simulation_distribution_to_cluster(["valardohaeris.local", "valarmorghulis.local"])

The binary distribution files are incrementally copied using the :command:`rsync` command.

Running a set of simulations on the cluster is done with the same function with some additional parameters:

.. code-block:: ipython

    In [1]: run_simulations(mode="debug", filter="PureAlohaExperiment", scheduler="cluster", cluster=c)
    Out[1]: Multiple simulation results: DONE, summary: 42 DONE in 0:00:04.783647

The log output is not present when simulations are executed on a cluster.

Another way is to collect multiple simulation tasks and run them on the cluster, potentially multiple times:

.. TODO

    this only works if database is not initialized,
    complete/partial binary/source hashes are not computed
    using debug mode, release mode native compiled binaries may fail on some cluster nodes

.. code-block:: ipython

    In [1]: mt = get_simulation_tasks(simulation_project=p, mode="release", filter="PureAlohaExperiment", scheduler="cluster", cluster=c)

    In [2]: mt.run()
    Out[2]: Multiple simulation results: DONE, summary: 42 DONE in 0:00:04.109337

Exiting from the interactive Python session also automatically stops the SSH cluster.

Analyzing Results
=================

.. TODO

Testing Projects
================

Developing simulation models and creating simulations is a very time consuming, complicated, and error prone process.
Continuous automated testing is the simplest tool that can be utilized to increase the quality of the final solution.

Smoke Testing
-------------

The most basic tests, called smoke tests, simply check if simulations run without crashing and terminate properly. For
example, running the smoke tests for all simulations from the default simulation project:

.. code-block:: ipython

    In [1]: run_smoke_tests()
    [6/7] . -c TandemQueueExperiment -r 3 PASS
    [5/7] . -c TandemQueueExperiment -r 2 PASS
    ...
    [7/7] . -c TandemQueues PASS
    [2/7] . -c Fifo2 PASS
    Out[1]: Multiple smoke test results: PASS, summary: 7 PASS in 0:00:01.117562

Running smoke tests for a set of simulation configs from a specific simulation project:

.. code-block:: ipython

    In [1]: p = get_simulation_project("aloha")

    In [2]: r = run_smoke_tests(simulation_project=p, config_filter="PureAlohaExperiment")
    [13/42] . -c PureAlohaExperiment -r 12 PASS
    [11/42] . -c PureAlohaExperiment -r 10 PASS
    ...
    [30/42] . -c PureAlohaExperiment -r 29 PASS
    [29/42] . -c PureAlohaExperiment -r 28 PASS

    In [3]: r
    Out[3]: Multiple smoke test results: PASS, summary: 42 PASS in 0:00:00.807932

Repeating the execution of all smoke tests from the last result:

.. code-block:: ipython

    In [1]: r = r.rerun()

The result is the same as before. Of course, you can filter the results to re-run only the failed tests.

.. code-block:: ipython

    In [1]: r = r.get_failed_results().rerun()

Fingerprint Testing
-------------------


Detecting regressions during the development of simulation projects is a very time consuming task.

Running fingerprint tests is somewhat more complicated, because the test framework needs a database to store fingerprints.
If there are no fingerprints in the database yet, then they must be first calculated and inserted:

.. code-block:: ipython

    In [1]: update_correct_fingerprints(simulation_project=p, config_filter="PureAlohaExperiment",
                                        sim_time_limit="1s", database=default_database)
    [04/42] Updating fingerprint . -c PureAlohaExperiment -r 3 for 1s INSERT 856a-c13d/tplx
    [11/42] Updating fingerprint . -c PureAlohaExperiment -r 10 for 1s INSERT 835c-d8e8/tplx
    ...
    [28/42] Updating fingerprint . -c PureAlohaExperiment -r 27 for 1s INSERT 1b5b-3e28/tplx
    [09/42] Updating fingerprint . -c PureAlohaExperiment -r 8 for 1s INSERT 83fb-e5d5/tplx
    Out[1]:

    Details:
      . -c PureAlohaExperiment for 1s INSERT ec03-9cdf/tplx
      . -c PureAlohaExperiment -r 1 for 1s INSERT 27df-ce58/tplx
      ...
      . -c PureAlohaExperiment -r 40 for 1s INSERT 3fbc-cdb2/tplx
      . -c PureAlohaExperiment -r 41 for 1s INSERT f657-bebb/tplx

    Multiple update fingerprint results: INSERT, summary: 42 INSERT (unexpected) in 0:00:01.567479

When the fingerprints are already present in the database, then the fingerprint tests can be run:

.. code-block:: ipython

    In [1]: run_fingerprint_tests(simulation_project=p, config_filter="PureAlohaExperiment",
                                  sim_time_limit="1s", database=default_database)
    [02/42] Checking fingerprint . -c PureAlohaExperiment -r 1 for 1s PASS
    [06/42] Checking fingerprint . -c PureAlohaExperiment -r 5 for 1s PASS
    ...
    [16/42] Checking fingerprint . -c PureAlohaExperiment -r 15 for 1s PASS
    [08/42] Checking fingerprint . -c PureAlohaExperiment -r 7 for 1s PASS
    Out[1]: Multiple fingerprint test results: PASS, summary: 42 PASS in 0:00:01.129969

The PASS result means that the calculated fingerprint of the simulation matches the fingerprint stored in the database.

Command Line Tools
==================

Some of the tasks that can be carried out using the Python interpreter, can also be done directly from the command line.
The following list gives a brief overview of these command line tools:

 - :command:`inet_python_repl`: starts the INET Python interpreter
 - :command:`inet_build_project`: builds a simulation project using tasks
 - :command:`inet_run_simulations`: runs multiple simulations matching a filter criteria
 - :command:`inet_run_smoke_tests`: runs multiple smoke tests matching a filter criteria
 - :command:`inet_run_fingerprint_tests`: runs multiple fingerprint tests matching a filter criteria
 - :command:`inet_update_correct_fingerprints`: updates stored correct fingerprints matching a filter criteria

.. note::

     All command line tools print a detailed description of their options when run with the `-h` option.

Building the simulation project enclosing the current working directory:

.. code-block:: console

    levy@valardohaeris:~/workspace/omnetpp/samples/aloha$ inet_build_project
    [1/2] Compiling Host.cc DONE
    [2/2] Compiling Server.cc DONE
    [1/1] Linking libaloha.so DONE
    [1/1] Copying dynamic libraries DONE
    Multiple build task results: DONE, summary: 3 DONE in 0:00:00.455684

Of course, building the same project again skips all subtasks because everything is up to date.

.. code-block:: console

    levy@valardohaeris:~/workspace/omnetpp/samples/aloha$ inet_build_project
    Multiple build task results: SKIP, summary: 3 SKIP (expected) in 0:00:00.000395

Building a project outside the current working directory is also possible:

.. code-block:: console

    levy@valardohaeris:~/workspace/omnetpp$ inet_build_project -p tictoc
    [15/24] Compiling txc13.cc DONE
    [11/24] Compiling txc14.cc DONE
    ...
    [18/24] Compiling txc6.cc DONE
    [24/24] Compiling txc15.cc DONE
    [1/1] Linking libtictoc.so DONE
    [1/1] Copying dynamic libraries DONE
    Multiple build task results: DONE, summary: 4 TOTAL, 3 DONE, 1 SKIP (expected) in 0:00:00.104676

Running all simulations from the `fifo` sample project using the current working directory:

.. code-block:: console

    levy@valardohaeris:~/workspace/omnetpp/samples/fifo$ inet_run_simulations
    [3/7] . -c TandemQueueExperiment DONE
    [5/7] . -c TandemQueueExperiment -r 2 DONE
    ...
    [1/7] . -c Fifo1 DONE
    [2/7] . -c Fifo2 DONE
    Multiple simulation results: DONE, summary: 7 DONE in 0:00:15.856987

Running all simulation runs from the `PureAlohaExperiment` config for 1 second on a SSH cluster of two hosts in debug mode:

.. code-block:: console

    levy@valardohaeris:~/workspace/omnetpp/samples/aloha$ inet_run_simulations -m debug -t 1s --filter PureAlohaExperiment --hosts valarmorghulis.local,valardohaeris.local
    Multiple simulation results: DONE, summary: 42 DONE in 0:00:01.196147

Running the fingerprint tests from the `fifo` sample project using the default fingerprint database:

.. code-block:: console

    levy@valardohaeris:~/workspace/omnetpp/samples/fifo$ inet_run_fingerprint_tests -t 1s
    Multiple fingerprint test results: SKIP, summary: 7 SKIP (unexpected) in 0:00:00.004558

Not surprisingly all tests are skipped because the database doesn't have any correct fingerprints yet.
We first need to update the correct fingerprints in the database:

.. code-block:: console

    levy@valardohaeris:~/workspace/omnetpp/samples/fifo$ inet_update_correct_fingerprints -t 1s
    [2/7] Updating fingerprint . -c Fifo2 for 1s INSERT 6593-438a/tplx
    [3/7] Updating fingerprint . -c TandemQueueExperiment for 1s INSERT 4cbd-3dae/tplx
    ...
    [4/7] Updating fingerprint . -c TandemQueueExperiment -r 1 for 1s INSERT f27b-15fd/tplx
    [6/7] Updating fingerprint . -c TandemQueueExperiment -r 3 for 1s INSERT 4cbd-3dae/tplx

    Details:
      . -c Fifo1 for 1s INSERT 01de-529f/tplx
      . -c Fifo2 for 1s INSERT 6593-438a/tplx
      ...
      . -c TandemQueueExperiment -r 3 for 1s INSERT 4cbd-3dae/tplx
      . -c TandemQueues for 1s INSERT 4cbd-3dae/tplx

    Multiple update fingerprint results: INSERT, summary: 7 INSERT (unexpected) in 0:00:00.172821

Now, we can run all fingerprint tests comparing the fingerprints against the latest results:

.. code-block:: console

    levy@valardohaeris:~/workspace/omnetpp/samples/fifo$ inet_run_fingerprint_tests -t 1s
    [3/7] Checking fingerprint . -c TandemQueueExperiment for 1s PASS
    [5/7] Checking fingerprint . -c TandemQueueExperiment -r 2 for 1s PASS
    ...
    [7/7] Checking fingerprint . -c TandemQueues for 1s PASS
    [4/7] Checking fingerprint . -c TandemQueueExperiment -r 1 for 1s PASS
    Multiple fingerprint test results: PASS, summary: 7 PASS in 0:00:00.164720

Of course, trying to update the correct fingerprints again doesn't change the stored values:

.. code-block:: console

    levy@valardohaeris:~/workspace/omnetpp/samples/fifo$ inet_update_correct_fingerprints -t 1s
    [5/7] Updating fingerprint . -c TandemQueueExperiment -r 2 for 1s KEEP 4cbd-3dae/tplx
    [7/7] Updating fingerprint . -c TandemQueues for 1s KEEP 4cbd-3dae/tplx
    ...
    [4/7] Updating fingerprint . -c TandemQueueExperiment -r 1 for 1s KEEP f27b-15fd/tplx
    [2/7] Updating fingerprint . -c Fifo2 for 1s KEEP 6593-438a/tplx
    Multiple update fingerprint results: KEEP, summary: 7 KEEP in 0:00:00.218112

Tips and Tricks
===============

.. TODO

The real power of using the Python interpreter comes with having your own utilities. This includes importing additional
Python modules, adding specific functions tailored to your needs, adding state variables to quickly access what you use
often, etc. So it is highly advisable to start writing your own Python package where you can add what is required.
"""

from inet.documentation import *
from inet.scave import *
from inet.simulation import *
from inet.test import *

__all__ = [k for k,v in locals().items() if k[0] != "_" and v.__class__.__name__ != "module"]

