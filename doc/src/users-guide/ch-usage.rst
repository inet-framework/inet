.. _ug:cha:usage:

Using the INET Framework
========================

.. _ug:sec:usage:installation:

Installation
------------

There are several ways to install the INET Framework:

-  Let the OMNeT++ IDE download and install it for you. This is the
   easiest way. Just accept the offer to install INET in the dialog that
   comes up when you first start the IDE, or choose :menuselection:`Help --> Install
   Simulation Models` any time later.

-  From INET Framework web site, *http://inet.omnetpp.org*. The IDE
   always installs the last stable version compatible with your version
   of OMNeT++. If you need some other version, they are available for
   download from the web site. Installation instructions are also
   provided there.

-  From GitHub. If you have experience with *git*, clone the INET
   Framework project (``inet-framework/inet``), check out the
   revision of your choice, and follow the INSTALL file in the project
   root.

.. _ug:sec:usage:installing-inet-extensions:

Installing INET Extensions
--------------------------

If you plan to make use of INET extensions (e.g. Veins or SimuLTE),
follow the installation instructions provided with them.

In the absence of specific instructions, the following procedure usually
works:

-  First, check if the project root contains a file named
   :file:`.project`.

-  If it does, then the project can be imported into the IDE (use
   :menuselection:`File --> Import --> General --> Existing Project`
   into workspace). Make sure
   that the project is recognized as an OMNeT++ project (the :guilabel:`Project
   Properties` dialog contains a page titled *OMNeT++*), and it lists
   the INET project as dependency (check the :guilabel:`Project References` page
   in the :guilabel:`Project Properties` dialog).

-  If there is no :file:`.project` file, you can create an empty OMNeT++
   project using the :guilabel:`New OMNeT++ Project` wizard in
   :menuselection:`File --> New`, add the INET project as dependency
   using the :guilabel:`Project References` page in the :guilabel:`Project Properties`
   dialog, and copy the source files into the project.

.. _ug:sec:usage:getting-familiar-with-inet:

Getting Familiar with INET
--------------------------

The INET Framework builds upon OMNeT++, and uses the same concept:
modules that communicate by message passing. Hosts, routers, switches
and other network devices are represented by OMNeT++ compound modules.
These compound modules are assembled from simple modules that represent
protocols, applications, and other functional units. A network is again
an OMNeT++ compound module that contains host, router and other modules.

Modules are organized into a directory structure that roughly follows
OSI layers:

-  :file:`src/inet/applications/` – traffic generators and application
   models

-  :file:`src/inet/transportlayer/` – transport layer protocols

-  :file:`src/inet/networklayer/` – network layer protocols and
   accessories

-  :file:`src/inet/linklayer/` – link layer protocols and accessories

-  :file:`src/inet/physicallayer/` – physical layer models

-  :file:`src/inet/routing/` – routing protocols (internet and ad hoc)

-  :file:`src/inet/mobility/` – mobility models

-  :file:`src/inet/power/` – energy consumption modeling

-  :file:`src/inet/environment/` – model of the physical environment

-  :file:`src/inet/node/` – preassembled network node models

-  :file:`src/inet/visualizer/` – visualization components (2D and 3D)

-  :file:`src/inet/common/` – miscellaneous utility components

The OMNeT++ NED language uses hierarchical packages names. Packages
correspond to directories under :file:`src/`, so e.g. the
:file:`src/inet/transportlayer/tcp` directory corresponds to the
``inet.transportlayer.tcp`` NED package.

For modularity, the INET Framework has about 80 *project features*
(parts of the codebase that can be disabled as a unit) defined. Not all
project features are enabled in the default setup after installation.
You can review the list of available project features in the
:menuselection:`Project --> Project Features...` dialog in the IDE.
If you want to know more about project features, refer to the
*OMNeT++ User Guide*.
