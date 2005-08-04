//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

//
// Not the real ICMP codes; here it's set up so that codes above 128 are
// informational
//
enum ICMPType
{
    ICMP_DESTINATION_UNREACHABLE = 13;
    ICMP_REDIRECT = 5;
    ICMP_TIME_EXCEEDED = 11;
    ICMP_PARAMETER_PROBLEM = 12;

    ICMP_ECHO_REQUEST = 128;
    ICMP_ECHO_REPLY = 129;
    ICMP_TIMESTAMP_REQUEST = 130;
    ICMP_TIMESTAMP_REPLY = 131;
};

cplusplus {{
typedef int ICMPCode;
}}


//
// ICMP message class
//
message ICMPMessage
{
  fields:
    int type enum(ICMPType);
    int code;
};

