
// ----------------------------------------------------------------------------
//
// HttpTools Project
//
// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable 
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------

package inet.httptools;

//
// httptController
//
// A controller object for OMNeT++ simulations which use the HttpTools browser and
// server components. A single controller must exist at the scenario level in each
// simulation.
//
// NEW: The module has been migrated to OMNeT++ 4.0 and the latest INET version.
//
// @version 1.0 
// @author  Kristjan V. Jonsson
//
simple httptController
{
    parameters:
        int logLevel = default(0);            // The log level: 2: Debug, 1: Info; 0: Errors and warnings only.
        string events = default("");          // Popularity modification events defintion file.
        string eventsSection = default("");   // Section within the popularity modification file, if one is defined.
        xml config;                           // XML configuration file.        
}

