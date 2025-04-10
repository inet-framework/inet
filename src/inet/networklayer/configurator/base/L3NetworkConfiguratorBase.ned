//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.networklayer.configurator.base;

import inet.common.SimpleModule;
import inet.networklayer.configurator.contract.IL3NetworkConfigurator;

//
// Serves as a base module for layer 3 network configurators.
//
simple L3NetworkConfiguratorBase extends SimpleModule like IL3NetworkConfigurator
{
    parameters:
        @class(L3NetworkConfiguratorBase);
        double minLinkWeight = default(1E-3);
        bool configureIsolatedNetworksSeparatly = default(false);
}

