//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//


import inet.common.INETDefs;

namespace inet::httptools;

//
// Server status update event
//
// Used by the controller to trigger popularity modifications for web sites.
//
// @author  Kristjan V. Jonsson
//
message HttpServerStatusUpdateMsg
{
    string www;             // Name of the WWW site.
    simtime_t setTime;      // The event set time.
    int eventKind;          // The event kind, e.g. ss_special.
    double pvalue;          // The initial elevated probability.
    double pamortize;       // The amortization factor -- i.e. how much the pvalue is decreased on each hit.
}


