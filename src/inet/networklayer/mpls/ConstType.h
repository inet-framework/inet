//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_CONSTTYPE_H
#define __INET_CONSTTYPE_H

#include "inet/common/INETDefs.h"

namespace inet {

enum messageKind {
    MPLS_KIND,
    LDP_KIND,
    SIGNAL_KIND
};

namespace mpls_constants {

const char libDataMarker[] = "In-lbl       In-intf     Out-lbl       Out-intf";
const char prtDataMarker[] = "Prefix            Pointer";

const char UnknownData[] = "UNDEFINED";
const char NoLabel[] = "Nolabel";
const char wildcast[] = "*";
const char empty[] = "";

const int ldp_port = 646;

const int LDP_KIND = 10;
const int HOW_KIND = 50;

} // namespace mpls_constants

} // namespace inet

#endif

