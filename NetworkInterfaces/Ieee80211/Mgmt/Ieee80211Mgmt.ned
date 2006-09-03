//
// Copyright (C) 2006 Andras Varga and Levente Mészáros
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
// This is not a concrete module type, but a prototype for all IEEE 802.11
// management module types. It exists to specify what gates a management
// module should have in order to be usable within Ieee80211Nic.
//
simple Ieee80211Mgmt
    gates:
        in: uppergateIn;
        out: uppergateOut;
        in: macIn;
        out: macOut;
endsimple
