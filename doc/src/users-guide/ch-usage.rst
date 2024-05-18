.. _ug:cha:usage:

Using the INET Framework
========================

.. _ug:sec:usage:installation:

Installation
------------

There are several ways to install the INET Framework:

- Allow the OMNeT++ IDE download and install it for you. This is the
  easiest way. Just accept the offer to install INET in the dialog that
  comes up when the IDE is started for the first time. Alternatively,
it can be installed by choosing :menuselection:`Help --> Install Simulation Models`
at any later time.

- The latest stable version of the INET Framework compatible with your
  version of OMNeT++ can be installed from the INET Framework website
  at *http://inet.omnetpp.org*. If a different version is required, it
  can also be downloaded from the website. Installation instructions are
  provided on the website.

- The INET Framework can be cloned from the GitHub repository. If you
  are familiar with *git*, clone the INET Framework project repository
  (``inet-framework/inet``) and check out the chosen revision. Then
follow the ``INSTALL`` file in the project root.

.. _ug:sec:usage:installing-inet-extensions:

Installing INET Extensions
--------------------------

If INET extensions (e.g. Veins or SimuLTE) are planned to be used, the
installation instructions provided with them should be followed.

Usually, the following procedure works if no specific instructions are
available:

- First, check if the project root contains a file named
  :file:`.project`.

- If the file exists, the project can be imported into the IDE by using
  :menuselection:`File --> Import --> General --> Existing Project`
  Make sure that the project is recognized as an OMNeT++ project
  the :guilabel:`Project Properties` dialog contains a page titled *OMNeT++*), and it lists
  the INET project as a dependency (check the :guilabel:`Project References` page
  in the :guilabel:`Project Properties` dialog).

- If there is no :file:`.project` file, create an empty OMNeT++ project
  using the :guilabel:`New OMNeT++ Project` wizard in
  :menuselection:`File --> New`. Then add the INET project as a dependency
  using the :guilabel:`Project References` page in the :guilabel:`Project Properties`
  dialog, and copy the source files into the project.

.. _ug:sec:usage:getting-familiar-with-inet:

Getting Familiar with INET
--------------------------

The INET Framework builds upon OMNeT++, and uses the same concept:
modules that communicate by message passing. Hosts, routers, switches,
and other network devices are represented by OMNeT++ compound modules.
These compound modules are assembled from simple modules that represent
protocols, applications, and other functional units. A network is again
an OMNeT++ compound module that contains host, router, and other modules.

Modules are organized into a directory structure that roughly follows
OSI layers:

- :file:`src/inet/applications/` – traffic generators and application
  models

- :file:`src/inet/transportlayer/` – transport layer protocols

- :file:`src/inet/networklayer/` – network layer protocols and
  accessories

- :file:`src/inet/linklayer/` – link layer protocols and accessories

- :file:`src/inet/physicallayer/` – physical layer models

- :file:`src/inet/routing/` – routing protocols (internet and ad hoc)

- :file:`src/inet/mobility/` – mobility models

- :file:`src/inet/power/` – energy consumption modeling

- :file:`src/inet/environment/` – model of the physical environment

- :file:`src/inet/node/` – preassembled network node models

- :file:`src/inet/visualizer/` – visualization components (2D and 3D)

- :file:`src/inet/common/` – miscellaneous utility components

The OMNeT++ NED language uses hierarchical package names. Packages
correspond to directories under :file:`src/`, so for example, the
:file:`src/inet/transportlayer/tcp` directory corresponds to the
``inet.transportlayer.tcp`` NED package.

For modularity, the INET Framework has about 80 *project features*,
which are parts of the codebase that can be disabled as a unit. After
installation, not all project features are enabled in the default setup.
The list of available project features can be reviewed in the
:menuselection:`Project --> Project Features...` dialog in the IDE. To
learn more about project features, refer to the *OMNeT++ User Guide*.
