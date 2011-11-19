//
// Copyright (C) 2010 Helene Lageber
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

enum BGPPathSegmentType
{
    AS_SET = 1;
    AS_SEQUENCE = 2;
}

//
// Represents a BGPv4 AS_PATH path attribute (RFC 4271 Section 4.3)
//
class BGPASPathSegment
{
    unsigned char type @enum(BGPPathSegmentType);
    unsigned char length;
    unsigned short asValue[];
}

