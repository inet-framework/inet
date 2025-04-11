//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common;

//
// Base module for all INET simple modules
//
// - Implements displayStringTextFormat parameter which is used by refreshDisplay()
//   to display module-specific information in the display string
//
simple SimpleModule
{
    parameters:
        // prevent instantiating this module if a user forgot to add @class() in a derived module
        @class(ForbiddenSimpleModule); 
        string displayStringTextFormat @mutable = default("");
}
