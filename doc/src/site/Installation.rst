===============
Installing INET
===============

Using the opp_env Package Manager
---------------------------------

The recommended way of installing the INET Framework, especially less recent versions, is `opp_env <http://omnetpp.org/opp_env>`__.
opp_env provides automated installation of over 70 models or model frameworks, with dependencies, including matching versions of OMNeT++.

An example session:

.. code:: console

   $ mkdir inet_workspace
   $ cd inet_workspace
   $ opp_env init
   $ opp_env install inet-latest  # or replace "latest" with a specific version
   $ opp_env shell

Note that this **installs OMNeT++ too**, so you don't need a pre-existing installation.

All-in-one variant:

.. code:: console

   $ opp_env shell --init -w inet_workspace --install inet-latest

Using the OMNeT++ IDE
---------------------

Recent versions of the OMNeT++ IDE can download and install INET (the latest stable version) for you.

Steps to follow?

1. Install OMNeT++
2. Open the Simulation IDE (``omnetpp``)
3. Go to the workbench (dismiss the Welcome screen). The first time you do this, a prompt will ask if you want to install INET.
4. Keep the boxes checked and proceed.

If you skipped this step (opened the workbench but skipped installing INET), nothing is lost:

1. Go to *Help -> Install Simulation Models*.
2. A dialog will appear with the available simulation models. Select INET and follow the prompts.

Both ways, the IDE will download, unpack, and automatically build INET from the sources.

Manual Installation
-------------------

If you downloaded an INET version other than the latest stable one, and you
cannot or don't want to use *opp_env*, you'll need to install it manually.

The procedure:

1. Install the matching version of OMNeT++.
2. Download the INET sources and unpack the archive.
3. Follow the instructions in the ``INSTALL`` (or ``INSTALL.md``) file in the archive.

You can read the ``INSTALL`` file that belongs to the current development version (git ``master`` branch)
`here <https://github.com/inet-framework/inet/blob/master/INSTALL.md>`__.
