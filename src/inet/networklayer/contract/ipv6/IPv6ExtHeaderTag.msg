//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, see <http://www.gnu.org/licenses/>.
//


import inet.common.INETDefs;

cplusplus {{
#include "inet/networklayer/contract/ipv6/IPv6ExtHeaderTagBase.h"
}}

namespace inet;


class IPv6ExtHeaderTagBase extends cObject;

class IPv6ExtHeaderReq extends IPv6ExtHeaderTagBase
{
}

class IPv6ExtHeaderInd extends IPv6ExtHeaderTagBase
{
}
