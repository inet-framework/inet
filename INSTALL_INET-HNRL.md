# INET-HNRL-SPECIFIC INSTALLATION INSTRUCTIONS

## GETTING STARTED
You shoud first read the original [INSTALL](INSTALL) document for the original
[INET](http://inet.omnetpp.org) framework and make sure that you meet all the
prerequsites mentioned there.

You should also install [SQLite](http://www.sqlite.org) library and header files
to support writing scalar and vector outputs to SQLite databases.

Before taking the building steps described in the INSTALL document, you should
set paths for a root directory of OMNeT++ installation, a directory for SQLite
header file, and a directory for SQLite library file as described in the
following sections for command line and IDE installations.

### If you are building from command line:
- **OMNETPP_ROOT**: You should set this environment variable for the root
  directory of OMNeT++ installation (without a trailing slash (Unix/Linux) or
  backslash (Windows)); 'Makefile' is prepared based on this environment
  variable and wouldn't work properly without it. Note that this step is needed
  in order to set the directories for unexposed header files of the current
  version of OMNeT++ (4.1+) in the top-level "Makefile" of INET-HNRL until the
  next version of OMNeT++ exposes them.
- If you install SQLite in one of the standard places (e.g., '/usr' or
  '/usr/local'), you can skip this step. Otherwise, you should add
  '-ISQLITE_INC' and '-LSQLITE_LIB' options to opp_makemake in the 'Makefile',
  where 'SQLITE_INC' and 'SQLITE_LIB' denote directories for SQLite header and
  SQLite library, respectively.

### If you are using the IDE:
- **OMNETPP_ROOT**: You should set this linked resource variable for the root
  directory of OMNeT++ installation as follows:
  1. Open the OMNeT++ IDE.
  2. Go to "Window->Preferences->General->Workspace->Linked resources" and
     define the path variable.
- If you install SQLite in one of the standard places (e.g., '/usr' or
  '/usr/local'), you can skip step. Otherwise, you should add directories for
  SQLite header and SQLite library as follows:
  1. Open the OMNeT++ IDE.
  2. Click 'inet-hnrl' in the 'Project Explorer' panel and go to 'Properties' by
     right clicking.
  3. Go to "C/C++ General->Paths and Symbols" and add the said directories for
     SQLite in the 'Includes' and 'Library Paths' panels.
