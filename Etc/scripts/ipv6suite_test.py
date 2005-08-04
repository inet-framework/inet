########################################################################
#
# Copyright (C) 2004 Johnny Lai
#
# This file is part of IPv6Suite
#
# IPv6Suite is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# IPv6Suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# File:   ipv6suite_test.py
# Author: Johnny Lai
# Date:   25 Jul 2004
#
# Contents:
#    IPv6SuiteInit, IPv6SuiteTest
#
########################################################################

########################################################################
#
#
# Check v3_test.py in qmtest libstdc++ testsuite for help on writing a Resource
# to build IPv6Suite. Also look at qm source distro
# qm-2.2/qm/test/classes/command.py for help on writing tests too.
#
# Sample run:
# qmtest run -c IPv6Suite.srcDir=~/src/IPv6Suite -c IPv6Suite.buildDir=/tmp/IPv6Suite ipv6suite
########################################################################


########################################################################
# Functions
########################################################################

def mkdirs(newdir, mode=0775):
    """ More use than the usual os.makedirs as it allows a directory to exist without giving error so I don't have to test if it exists etc.
    """
    try: os.makedirs(newdir, mode)
    except OSError, err:
        if err.errno != errno.EEXIST or not os.path.isdir(newdir):
            raise

def IdToDir(self):
    """ As QMTest default XML database only accepts filenames with one . it is better to create the hierarchical directories without the '.' in the dir name if the test/resource is in a subdirectory. self is either a descendent of Test or Resource.
    """

    return re.sub(r'\.', "/", self.GetId())

########################################################################
# Imports
########################################################################

import shutil
import fnmatch
import glob
import os
import os.path
import errno
import re
import qm
import sys
import string
import difflib
import filecmp
from qm.executable import RedirectedExecutable
from qm.test.test import Test
from qm.test.resource import Resource
from qm.test.result import Result
from qm.test.classes.command import ExecTestBase
from qm.test.classes.command import ShellCommandTest
from difflib import ndiff
#from compiler import CompilerExecutable



########################################################################
# classes
########################################################################

class IPv6SuiteInit(Resource):

    """ A particular build config that tests can depend on and use. 
"""

    arguments = [
        qm.fields.TextField(
        name="ipv6suite_build_options",
        title="CMake Options for IPv6Suite",
        description="Options passed to CMake invocation. These are usually build options "
        "that you set manually in ccmake. Currently we accept a space separated list of cmake BOOLEAN variables only.",
        default_value="USE_XMLWRAPP"
        ),

        qm.fields.TextField(
        name="make_options",
        title="Arguments to append to make command line",

        description="""Append options to the make command line to build
        IPv6Suite. Make occurs after cmake completes. Currently it is using
        the -j$RPM_BUILD_NCPUS so it is advisable to use BuildNedFirst as
        some kind of race condition exists when compiling _n.cc files
        generated from nedc. In future we won't even compile it since there
        is dynamic ned loading in opp3""",

        default_value="BuildNedFirst all"
        ),

#Superceded by context IPv6Suite.wipe_build_dir
#       qm.fields.BooleanField(
#       name="wipe_build_dir",
#       title="Delete Build Directory?",
#       description="""Controls whether the Build directory is deleted after dependent tests are run. Also controls whether tests are deleted during SetUp. Best to leave it at 1 unless you know what you are doing, since this guarantees a clean build environment.""",
#       default_value="true",
#       )
        ]
    
    def SetUp(self, context, result):
#        if not context.has_key("IPv6Suite.srcDir"):
            # By default we assume there is a compiler.
        srcDir = context["IPv6Suite.srcDir"]
        srcDir = os.path.expanduser(srcDir)
        context["IPv6Suite.srcDir"] = srcDir
        if not os.path.exists(srcDir):
            result.SetOutcome(result.ERROR,
                              "srcDir does not exist. Where will we get source"
                              "files from?")
            return
        
        
        buildDir = context["IPv6Suite.buildDir"]
        buildDir = os.path.abspath(buildDir)
        context["IPv6Suite.buildDir"] = buildDir
            
        buildDir = os.path.join(buildDir, IdToDir(self))
        
        self.wipe_build_dir = qm.parse_boolean(context["IPv6Suite.wipe_build_dir"])
        #Can't use bool since the boolean_value is a string of 'true|false' and since bool
        # thinks of any string as true except empty or none.
        #self.wipe_build_dir = bool(self.wipe_build_dir)
        
        #We want clean builds especially when previously failed ones may have broken generated code        
        if self.wipe_build_dir and os.path.exists(buildDir):
            shutil.rmtree(buildDir)

        mkdirs(buildDir)

        if self.wipe_build_dir:
            print "Wiping all files"
            
        context["IPv6Suite.myBuildDir"] = buildDir
        self.myBuildDir = buildDir
        #srcdir/test is where we store the related input files like ini/xml etc.
        #Well database of qmtest would be where we store these input files but what structure underneath?
        
        make_executable = RedirectedExecutable()
        #We actually want a gui to relace this value if possible?
        cmake_defines = string.split(self.ipv6suite_build_options)
        cmake_options = "-DOPP_USE_TK:BOOL=OFF"
##        "-DLIBRARY_OUTPUT_PATH:PATH=lib -DEXECUTABLE_OUTPUT_PATH:PATH=exe"
        for o in cmake_defines:
            cmake_options = "-D"+o+":BOOL=ON"+ " " + cmake_options

        if not os.path.exists(os.path.join(buildDir, "CMakeCache.txt")):
            cmake_command = ["cmake"] + string.split(cmake_options) + [srcDir]
            #print cmake_command
            status = make_executable.Run(cmake_command, dir=buildDir)
            if not os.WIFEXITED(status) or os.WEXITSTATUS(status) != 0:
                q_stdout = result.Quote(make_executable.stdout)
                q_stderr = result.Quote(make_executable.stderr)
                result.SetOutcome(result.ERROR,
                                  "Error in cmake Makefile generation",
                                  {"status": str(status),
                                   "stdout": q_stdout,
                                   "stderr": q_stderr,
                                   "command": " ".join(cmake_command),
                                   })
                return

        if os.environ.has_key("RPM_BUILD_NCPUS"):
            cpus = os.environ["RPM_BUILD_NCPUS"]
        else:
            cpus = "1"
        make_command =["make", "-j" + cpus] + string.split(self.make_options)
        status = make_executable.Run(make_command, dir=buildDir)
        if not os.WIFEXITED(status) or os.WEXITSTATUS(status) != 0:
                q_stdout = result.Quote(make_executable.stdout)
                q_stderr = result.Quote(make_executable.stderr)
                result.SetOutcome(result.ERROR,
                                  "Error building IPv6Suite",
                                  {"status": str(status),
                                   "stdout": q_stdout,
                                   "stderr": q_stderr,
                                   "command": " ".join(make_command),
                                   })
                return
        
    def CleanUp(self, result):
        if self.wipe_build_dir:
            shutil.rmtree( self.myBuildDir)
        #Should catch exception and return it as a result with annotation so we can see in HTML or console
        
class IPv6SuiteTest(ExecTestBase):
#class IPv6SuiteTest(ShellCommandTest):
    
    """ Test class for IPv6Suite regression testing. Each test case shall
have fields that are customised for a particular network scenario. Please look
at the description of each argument for this class for more detail.

There is a directory of files that are to be used as inputs to the simulation
inc. XML/ini.
"""
    arguments = [       
        qm.fields.TextField(
        name="scenario",
        title="Name of executable scenario to run",

        description="""scenario is searched for in context["IPv6Suite.myBuildDir"]/Examples
        recursively.

        This is a required value. If empty will trigger an exception
        """

        ),

        qm.fields.TextField(
        name="scenario_args",
        title="Arguments to pass to scenario",

        description="""These are the usual arguments that are passed to an
        omnetpp executable on the command line. E.g. -f omnetpp.ini and -r 3 etc.

        If empty will trigger an exception. It is best to specify the run
        explicitly otherwise will either run the very first one or the
        default-run in the ini file.  """,

        default_value="-r 1"

        ),

        qm.fields.TextField(
        name="description",
        title="Description for humans",

        description="""Provide a description for the test so fellow colleagues
        understand what it is you are testing.

        """,
        verbatim="true",
        multiline="true",
        default_value="Please provide a description for what this test is doing. REMOVE this template test"
        ),
        
        qm.fields.TextField(
        name="input_dir",
        title="Input files directory",

        description="""Directory containing all input files which allow test to
        run. The path is relative to self.GetDatabase().GetRoot()/input. Copied
        to os.path.join(context["IPv6Suite.myBuildDir"], run_directories,
        IdToDir(self)) togther with binaries so it is ready to run. After run
        finishes the whole directory is copied to
        self.GetDatabase().GetRoot()/test_output. Input dirs can be shared
        between tests since we use the run number to differentiate each test.

        If default value is null then the input_dir =
        os.path.join(self.GetDatabase().GetRoot(), "input", IdToDir(self)).

        
        TODO: create a resource for input files so that i.e. default.ini is a
        resource file. Then dependent runs will have the resources copied first
        before the files are copied from here saving duplication of files etc.
        """,
        default_value="" ),

        qm.fields.TextField(
        name="compare_dir",
        title="Expected output files directory",

        description="""Directory containing manually verified output files
        e.g. vec and/or sca files. The path is relative to
        self.GetDatabase().GetRoot()/expected. Each file in this directory will
        be compared against a file with the exact same name at the end of the
        test run. The results of failed diffs will be returned in results.

        If the value is empty then it is assumed to be
        os.path.join(self.GetDatabase().GetRoot(),"expected",IdToDir(self))
        """
        ),

        ]

# run script?
  
    def Run(self, context, result):
##         print self.GetDatabase().GetRoot()
##         print self.GetId()
##         print context["IPv6Suite.myBuildDir"]
##         print context["IPv6Suite.buildDir"]

        if self.scenario == "":
            return result.SetOutcome(result.ERROR, " scenario cannot be empty")

        buildDir = context["IPv6Suite.myBuildDir"]    
        if not os.path.isdir(buildDir):
            result.SetOutcome(result.ERROR, "Build Dir " + buildDir + " does not exist")
            return

        input_directories = "input"
        
        if self.input_dir == "":
            self.testName = IdToDir(self)
            self.input_dir = self.testName
            input_dir = os.path.join(self.GetDatabase().GetRoot(), input_directories, self.testName)
        else:
            input_dir = os.path.join(self.GetDatabase().GetRoot(), input_directories, self.input_dir)

        if not os.path.isdir(input_dir):
            return result.SetOutcome(result.ERROR, "input_dir " + input_dir + " does not exist")

        run_directories = "run"
        runDir = os.path.join(buildDir, run_directories, IdToDir(self))
        if os.path.exists(runDir) and not os.path.isdir(runDir):
            return result.SetOutcome(result.ERROR, " runDir " + runDir + " is not a directory ")

        if os.path.exists(runDir):
            shutil.rmtree(runDir)
        os.makedirs(runDir)

        ### Find executable in buildDir
        def findFile(absfile, path, names):
            if os.path.exists(self.scenario):
                names = []
                return
            for f in names:
                if os.path.isfile(os.path.join(path, f)) and absfile == f:
                    self.scenario = os.path.join(path, f)
                    names = []
                    break
        
        if not os.path.isfile(self.scenario):
            #Find it
            os.path.walk(os.path.join(buildDir, "Examples"), findFile, self.scenario)
##            os.path.walk(os.path.join(buildDir, "exe"), findFile, self.scenario)
            
        if not os.path.isfile(self.scenario):
            return result.SetOutcome(result.ERROR, " scenario " + self.scenario + " was NOT found ")

        shutil.copy2(self.scenario, runDir)

        def copyInputFiles(unused, path, names):
            for f in names:
                if not os.path.isfile(os.path.join(path, f)):
                    del names[names.index(f)] #prevent *.vec files from been copied too as compare_dir is subdir
	    #Need to reiterate again as deletion causes the iterator to point to one past the next one so we miss a file :(
	    #If there was more than one subdir then above loop would fail since the iterator problem would miss a directory and we'd end up in a subdir again. Need to read up further. For now assume only a subdir
            for f in names:
                if os.path.isfile(os.path.join(path, f)):
                    shutil.copy2(os.path.join(path, f), runDir)
                    
        os.path.walk(input_dir, copyInputFiles, "")

        def fixPaths(unused, path, names):
            for f in names:
                if os.path.isfile(os.path.join(path, f)):
                    #Fix ini files
                    if re.search(r'\.ini$|\.xml$',f):
                        os.system("perl -i -pwe \"s|../../Etc/||\" " + os.path.join(path, f))

        
        os.path.walk(runDir, fixPaths, "")

        dtdname = "netconf2.dtd"
        default_ini_name = "default.ini"
        dtdfile = os.path.join(runDir, dtdname)
        default_ini = os.path.join(runDir, default_ini_name)
        for f in [ default_ini, dtdfile]:
            if not os.path.exists(f):                
                shutil.copy2(os.path.join(context["IPv6Suite.srcDir"], "Etc", os.path.basename(f)), runDir)

        cwd = os.getcwd()
        os.chdir(runDir)
##         executable = RedirectedExecutable()
##         status = executable.Run([os.path.join(runDir, os.path.basename(self.scenario)), "-r1"], dir=runDir)
## #        status = executable.Run(["pwd"], dir=runDir)
##         result["stdout"] = result.Quote(executable.stdout)
##         result["stderr"] = result.Quote(executable.stderr)
##         result["status"] = str(status)

##         print runDir + ' scenario ' + self.scenario
        args = [os.path.join(runDir, os.path.basename(self.scenario))] + string.split(self.scenario_args)
## #        self.RunProgram("./" + os.path.basename(self.scenario), args, context, result)
        self.RunProgram(args[0], args, context, result)
##        self.RunProgram('/tmp/IPv6Suite/build-init/ipv6suite/testnew/MIPv6Network', ['/tmp/IPv6Suite/build-init/ipv6suite/testnew/MIPv6Network', '-r1'], context, result)
## #        self.RunProgram("/bin/echo", ["/bin/echo", "hello ", " world"], context, result)
        if result.GetOutcome() == Result.PASS:
            self.CompareFiles(context, result, input_dir, runDir)            
        
        os.chdir(cwd)

    def CompareFiles(self, context, result, input_dir, runDir):
        
        expected_directories = "expected"
        
        if self.compare_dir == "":
            compare_dir = os.path.join(self.GetDatabase().GetRoot(), expected_directories, IdToDir(self))
        else:
            compare_dir = os.path.join(self.GetDatabase().GetRoot(), expected_directories, self.compare_dir)

        #Does this change test database?
        self.compare_dir = compare_dir

        try:
            generateExpectedResults = qm.parse_boolean(context["IPv6Suite.build"])
        except:
            generateExpectedResults = 0

        if generateExpectedResults:
            print "Generating expected results for test " + self.GetId() + " into " + compare_dir
            
        if not generateExpectedResults and not os.path.exists(compare_dir):
            return result.SetOutcome(result.ERROR, " compare_dir "+ self.compare_dir + " does not exist")


        self.differences = ""
        def diffResults(unused ,path, names):
            for f in names:
                if os.path.isfile(os.path.join(path, f)):
                    if not os.path.isfile(os.path.join(runDir, f)):
                        return result.Fail("Result file " + f + " was not produced!")
                    if not filecmp.cmp(os.path.join(path, f), os.path.join(runDir, f)):
                        self.differences += f + " "
                    continue
                    if re.match(r'2\.3', sys.version):
                        result[f] = result.Quote(''.join(
                            difflib.unified_diff(open(os.path.join(path, f)).read(),
                                                 open(os.path.join(runDir, f)).read())))
                    else:
                        diff = ndiff(open(os.path.join(path, f)).read(),
                                     open(os.path.join(runDir, f)).read())
                        result[f] = result.Quote(''.join(diff))

        #Compare output to expected and preverified results
        if os.path.exists(compare_dir):
            os.path.walk(compare_dir, diffResults, None)
            if self.differences:
                result["different results"] = result.Quote(self.differences)
                result.Fail(" Resulting output files differ: " + self.differences)

        #Copy result files to output dir for later perusal
        output_dir = os.path.join(self.GetDatabase().GetRoot(), "test_output", IdToDir(self))
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        def copyOutputFiles(dest, path, names):
            for f in names:
                file = os.path.join(path, f)
                if os.path.isfile(file):
                    if re.search(r'\.vec$|\.sca', f):
                        print " moving " + f + " to " + dest
                        #os.rename(file, os.path.join(dest, f)) #Does not work as it uses link so needs same filesystem
                        shutil.copy2(file, dest)
                        os.remove(file)

        if not generateExpectedResults:
            os.path.walk(runDir, copyOutputFiles, output_dir)
        else:
            os.path.walk(runDir, copyOutputFiles, compare_dir)


        
## Copied from ExecTestBase.RunProgram and modified so that stdout/stderr is always displayed
    def RunProgram(self, program, arguments, context, result):
        """Run the 'program'.

        'program' -- The path to the program to run.

        'arguments' -- A list of the arguments to the program.  This
        list must contain a first argument corresponding to 'argv[0]'.

        'context' -- A 'Context' giving run-time parameters to the
        test.

        'result' -- A 'Result' object.  The outcome will be
        'Result.PASS' when this method is called.  The 'result' may be
        modified by this method to indicate outcomes other than
        'Result.PASS' or to add annotations."""

        # Construct the environment.
        environment = self.MakeEnvironment(context)
        # Create the executable.
        if self.timeout >= 0:
            timeout = self.timeout
        else:
            # If no timeout was specified, we sill run this process in a
            # separate process group and kill the entire process group
            # when the child is done executing.  That means that
            # orphaned child processes created by the test will be
            # cleaned up.
            timeout = -2
        e = qm.executable.Filter(self.stdin, timeout)
        # Run it.
#        print arguments
#        print " path "+ program
        exit_status = e.Run(arguments, environment, path = program)

        # Get the output generated by the program regardless of how program finished
        stdout = e.stdout
        stderr = e.stderr
        # Record the results.

        #Stdout is too big we need to just discard it I guess or save it as stdout.out in output
        result["ExecTest.stdout"] = result.Quote(stdout)
        result["ExecTest.stderr"] = result.Quote(stderr)

        if not re.search(r'End run of OMNeT',e.stdout) and not re.search(r'Calling finish', e.stdout):
            return result.Fail("Simulation did not end properly")
            
        # If the process terminated normally, check the outputs.
        if sys.platform == "win32" or os.WIFEXITED(exit_status):
            # There are no causes of failure yet.
            causes = []
            # The target program terminated normally.  Extract the
            # exit code, if this test checks it.
            if self.exit_code is None:
                exit_code = None
            elif sys.platform == "win32":
                exit_code = exit_status
            else:
                exit_code = os.WEXITSTATUS(exit_status)

##             result["ExecTest.exit_code"] = str(exit_code)
##             # Check to see if the exit code matches.
##             if exit_code != self.exit_code:
##                 causes.append("exit_code")
##                 result["ExecTest.expected_exit_code"] \
##                     = str(self.exit_code)
##             # Check to see if the standard output matches.
##             if not self.__CompareText(stdout, self.stdout):
##                 causes.append("standard output")
##                 result["ExecTest.expected_stdout"] \
##                     = result.Quote(self.stdout)
##             # Check to see that the standard error matches.
##             if not self.__CompareText(stderr, self.stderr):
##                 causes.append("standard error")
##                 result["ExecTest.expected_stderr"] \
##                     = result.Quote(self.stderr)
##             # If anything went wrong, the test failed.
##             if causes:
##                 result.Fail("Unexpected %s." % string.join(causes, ", ")) 

        elif os.WIFSIGNALED(exit_status):
            # The target program terminated with a signal.  Construe
            # that as a test failure.
            signal_number = str(os.WTERMSIG(exit_status))
##             result.Fail("Program terminated by signal.")
            result["ExecTest.signal_number"] = signal_number
            # Get the output generated by the program.
            
        elif os.WIFSTOPPED(exit_status):
            # The target program was stopped.  Construe that as a
            # test failure.
            signal_number = str(os.WSTOPSIG(exit_status))
            result.Fail("Program stopped by signal.")
            result["ExecTest.signal_number"] = signal_number
        else:
            # The target program terminated abnormally in some other
            # manner.  (This shouldn't normally happen...)
            result.Fail("Program did not terminate normally.")

