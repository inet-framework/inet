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

If you are building from command line:
--------------------------------------
Set an environment variable "OMNETPP_ROOT" to specify the root directory of
OMNeT++ installation (without a trailing slash (Unix/Linux) or backslash
(Windows)). This step is needed in order to set the directories for unexposed
header files of the current version of OMNeT++ (4.1+) in the top-level
"Makefile" of INET-HNRL until the next version of OMNeT++ exposes them.

If you are using the IDE:
-------------------------


