//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.configurator.base;

import inet.networklayer.configurator.contract.IL3NetworkConfigurator;

//
// This module serves as a base module for layer 3 network configurators.
//
simple L3NetworkConfiguratorBase like IL3NetworkConfigurator
{
    parameters:
        double minLinkWeight = default(1E-3);
        bool configureIsolatedNetworksSeparatly = default(false);
}

