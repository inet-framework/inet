INSTALLATION INSTRUCTIONS
=========================

The INET Framework can be compiled on any platform supported by OMNeT++.

PREREQUISITES

You should have a working OMNeT++ installation.

General
-------
1. Make sure your OMNeT++ installation works OK (e.g. try running the samples)
   and it is in the path (to test, try the command "which omnetpp", it should print
   the path of the executable). On Windows, open a console with the `mingwenv.cmd`
   command. The PATH and other variables will be automatically adjusted for you.
   Use this console to compile and run INET.

2. Extract the downloaded tarball into a directory of your choice (usually into your
   workspace directory, if you are using the IDE). NOTE: The built-in Windows
   archiver has bugs and cannot extract the file correctly. Use some other archiver
   or do it from command line (`tar xvfz inet-x.y.z-src.tgz`)


If you are building from command line:
--------------------------------------
3. Change to the INET directory and source the `setenv` script.

       $ source setenv

4. Make sure that any required Python modules are properly installed by executing
  `pip install -r python/requirements.txt`

5. Type `make makefiles`. This should generate the makefiles for you automatically.

6. Type `make` to build the inet executable (release version). Use `make MODE=debug`
   to build debug version.

7. You can run specific examples by changing into the example's directory and executing `inet`

If you are using the IDE:
-------------------------
3. Open the OMNeT++ IDE and choose the workspace where you have extracted the inet directory.
   The extracted directory must be a subdirectory of the workspace dir.

4. Import the project using: *File | Import | General | Existing projects* into *Workspace*.
   Then select the workspace dir as the root directory, and be sure NOT to check the
   "Copy projects into workspace" box. Click Finish.

5. Open the project (if already not open).
   Now you can build the project by pressing `CTRL-B` (*Project | Build all*)

6. To run an example from the IDE open the example's directory in the *Project Explorer* view,
   find the corresponding omnetpp.ini file. Right click on it and select *Run As / Simulation*.
   This should create a *Launch Configuration* for this example.


Note:
-----
- by default INET is creating a shared library (libINET.dll, libINET.so etc.)
  in the `src` directory. To use the shared library you can use the `inet`
  command to load it dynamically.
- If you add/remove files/directories later in the src directory, you MUST
  re-create your makefile. Run `make makefiles` again if you are building
  from the command line. (The IDE does it for you automatically)

Note to GIT users:
------------------

If you want to check out INET directly from the repository, we recommend using the

    $ git clone git@github.com:inet-framework/inet.git

To make the installation simple, the GIT repo contains all IDE configuration files.
If you make local changes in the IDE you may need to disable the change tracking on
those files, so GIT will not insist committing those changes back on your next commit.
You can use the _scripts/track-config-files-[on/off] scripts to enable/disable the 
change tracking.

To further ease the merging/rebasing operation the `.cproject` `.nedfolders` `.oppbuildspec` `.project`
files are configured to be resolved using the 'ours' merge strategy.  Depending on your
GIT version, you may need to enable the 'ours' merge driver for the project:

    $ git config merge.ours.driver true

VoIPTool feature
================

VoIPTool has only been tested on Linux. This does not mean it won't work on other
systems, but your mileage may vary on getting it up and running.


PREREQUISITES.

VoIPTool requires a "devel" package of the avcodec library (part of FFmpeg) 
to be installed on your system. On Ubuntu, this package can be installed with the 
following command:

    $ sudo apt-get install libavcodec-dev
    $ sudo apt-get install libavformat-dev

The package name and installation command may vary for other Linux systems.
