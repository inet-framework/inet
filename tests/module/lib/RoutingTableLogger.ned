//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.test.moduletest.lib;

//
// An ISriptable module for dumping routing tables into a file.
//
// Use the following command in the script of the ScenarioManager:
//
//   <dump-routes module="..." nodes="..." dest="..."/>
//
// where
//   - @module is the full name of this logger module
//   - @nodes is the full name of the nodes whose routing tables to be dumped
//   - @dest is a list of address/prefix_length pairs for filtering the routes (optional)
//
simple RoutingTableLogger
{
    parameters:
        string outputFile = default("");
        @display("i=block/cogwheel_s");
}

