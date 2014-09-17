//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
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

#endif // ifndef __INET_CONSTTYPE_H

