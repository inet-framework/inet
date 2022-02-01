//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

package inet.routing.pim.tables;

//
// The PimInterfaceTable module contains the PIM specific configuration
// parameters and state of the interfaces on which PIM is enabled.
//
// PIM routers must contain an instance of this module.
//
// The configuration file must contain <interface> elements under
// the document root with the following attributes:
//
// - @mode:          optional attribute, possible values are "dense" and "sparse".
//                   If missing then PIM is not used on that interface, otherwise
//                   it operates in the specified mode.
// - @state-refresh: optional boolean attribute, default is "false".
//                   If it is "true", then the router will send SR messages
//                   on this interface.
//
// Additionally each <interface> element may contain
// @hosts, @names, @towards, @among attributes to select the
// interfaces which the PIM parameters are applied to (see ~Ipv4NetworkConfigurator).
//
simple PimInterfaceTable
{
    parameters:
        @display("i=block/table");
        string interfaceTableModule;
        xml pimConfig = default(xml("<config/>"));
}

