//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, Dortmund University of Technology (TU Dortmund), Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.networklayer.mipv6;

import inet.common.SimpleModule;

//
// ~Mipv6 Data structure for the HA and CN(s) only.
//
// @see  ~Mipv6.
//
simple BindingCache extends SimpleModule
{
    parameters:
        @class(BindingCache);
        @display("i=old/harddisk");
}

