//
// Copyright (C) 2009 Kristjan V. Jonsson, LDSS (kristjanvj@gmail.com)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
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


namespace inet::httptools;

//
// Server status update event
//
// Used by the controller to trigger popularity modifications for web sites.
//
// NEW: The definition has been migrated to OMNeT++ 4.0 and the latest INET version.
//
// @author  Kristjan V. Jonsson
//
packet HttpServerStatusUpdateMsg
{
    @omitGetVerb(true);
    string www;             // Name of the WWW site.
    simtime_t setTime;      // The event set time.
    int eventKind;          // The event kind, e.g. ss_special.
    double pvalue;          // The initial elevated probability.
    double pamortize;       // The amortization factor -- i.e. how much the pvalue is decreased on each hit.
}


