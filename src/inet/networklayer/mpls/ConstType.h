//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

