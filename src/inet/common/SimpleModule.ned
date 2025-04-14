//
// Copyright (C) 2025 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.common;

import inet.common.ModuleMixin;

//
// Base module for all INET simple modules.
//
simple SimpleModule extends ModuleMixin
{
    parameters:
        @class(SimpleModule);
}
