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

// the reserved label values of RFC 3032 section 2.1
const uint32_t IPV4_EXPLICIT_NULL_LABEL = 0;
const uint32_t ROUTER_ALERT_LABEL = 1;
const uint32_t IPV6_EXPLICIT_NULL_LABEL = 2;
const uint32_t IMPLICIT_NULL_LABEL = 3;

const int LDP_KIND = 10;
const int HOW_KIND = 50;

} // namespace mpls_constants

} // namespace inet

#endif

