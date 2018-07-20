//***************************************************************************
//* file:        BaseMoBANNetwork.ned
//*
//* author:      Majid Nabi <m.nabi@tue.nl>
//*
//*              http://www.es.ele.tue.nl/nes
//*
//*
//* copyright:   (C) 2010 Electronic Systems group(ES),
//*              Eindhoven University of Technology (TU/e), the Netherlands.
//*
//*
//*              This program is free software; you can redistribute it
//*              and/or modify it under the terms of the GNU General Public
//*              License as published by the Free Software Foundation; either
//*              version 2 of the License, or (at your option) any later
//*              version.
//*              For further information see file COPYING
//*              in the top level directory
//***************************************************************************
//* part of:    MoBAN (Mobility Model for wireless Body Area Networks)
//* description:     The base network for running simulation examples of using the MoBAN mobility model.
//***************************************************************************
//* Citation of the following publication is appreciated if you use MoBAN for
//* a publication of your own.
//*
//* M. Nabi, M. Geilen, T. Basten. MoBAN: A Configurable Mobility Model for Wireless Body Area Networks.
//* In Proc. of the 4th Int'l Conf. on Simulation Tools and Techniques, SIMUTools 2011, Barcelona, Spain, 2011.
//*
//* BibTeX:
//*    @inproceedings{MoBAN,
//*     author = "M. Nabi and M. Geilen and T. Basten.",
//*     title = "{MoBAN}: A Configurable Mobility Model for Wireless Body Area Networks.",
//*        booktitle = "Proceedings of the 4th Int'l Conf. on Simulation Tools and Techniques.",
//*        series = {SIMUTools '11},
//*        isbn = {978-963-9799-41-7},
//*    year = {2011},
//*        location = {Barcelona, Spain},
//*    publisher = {ICST} }
//*
//**************************************************************************/

package inet.examples.mobility;

import inet.mobility.group.MoBanCoordinator;


//
// This network is exactly the same as baseNetwork existing among the examples of MiXiM, but the desired number of MoBAN coordinator modules have been added.
// Some nodes of type BaseNode use MoBanLocal module as their mobility module. Then those nodes will be considered as WBAN nodes.
// Those node have a parameter named "coordinatorIndex" that determines to which WBAN (coordinator) the node belongs.
//
network MoBANNetwork
{
    parameters:
        int numNodes; // total number of nodes in the network, either ordinary nodes or WBAN nodes
        int numMoBAN; // total number of MoBAN coordinators (WBAN instances)
        @display("bgb=1000,1000");
    submodules:
        node[numNodes]: MobileHost {
            parameters:
                @display("b=8,8,oval,red,,");
        }
        coordinator[numMoBAN]: MoBanCoordinator {
            parameters:
                @display("i=block/wheelbarrow;p=100,100;is=s");
        }
}
