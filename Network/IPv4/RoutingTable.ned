//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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
// Keeps the table of all routing and interface information.
//
// Interfaces are dynamically registered. The routing table is read
// from a file (parameter routingTableFileName); the file can also fill in 
// or overwrite interface settings.
// The file format is documented <a href="irt.html">here</a>.
//
// It has no gates; all functionality can be accessed via member functions
// of the C++ module class. For detailed info please see the C++ documentation 
// of the class (Doxygen).
//
// @author: Jochen Reber, Andras Varga
//
simple RoutingTable
    parameters:
        routingTableFileName : string,
        IPForward : bool;
endsimple

