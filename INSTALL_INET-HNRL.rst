INET-HNRL-SPECIFIC INSTALLATION INSTRUCTIONS
============================================

GETTING STARTED
---------------
You shoud first read INSTALL_ document for the original `INET
<http://inet.omnetpp.org>`_ framework and make sure that you meet all the
prerequsites mentioned there.

.. _INSTALL: https://github.com/kyeongsoo/inet-hnrl/blob/master/INSTALL

You should also install `SQLite <http://www.sqlite.org>`_ library and header
files to support writing scalar and vector outputs to SQLite databases.

Note that, before taking the building steps described in the INSTALL_ document,
you should set paths for (1) root directory of OMNeT++ installation, (2)
directory for the SQLite header file, and (3) directory for the SQLite library
file as follows:

If you are building from command line:
--------------------------------------
Set environment variables as follows:

- **OMNETPP_ROOT**: Root directory of OMNeT++ installation (without a trailing
  slash (Unix/Linux) or backslash (Windows)). This step is needed in order to
  set the directories for unexposed header files of the current version of
  OMNeT++ (4.1+) in the top-level "Makefile" of INET-HNRL until the next version
  of OMNeT++ exposes them.

- **SQLITE_INC**: Directory for SQLite header file.

- **SQLITE_LIB**: Directory for SQLite library file.

If you are using the IDE:
-------------------------
Set linked resource variables for the workspace as follows:

- Open the OMNeT++ IDE.

- Go to "Window->Preferences->General->Workspace->Linked resources" and define
  three path variables -- i.e., OMNETPP_ROOT, SQLITE_INC, SQLITE_LIB -- as for
  the environment variables.
