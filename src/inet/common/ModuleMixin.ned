//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common;

//
// Base module for all INET modules.
//
// This is a base type corresponding to the ModuleMixin<> c++ class.
// Never use this directly in any simulation. It servers only as the ultimate
// base type for all modules in INET.
//
simple ModuleMixin
{
    parameters:
        string displayStringTextFormat @mutable = default(""); // Determines the text that is written on top of the submodule, supports displaying pars, watches, and module-specific information
}
