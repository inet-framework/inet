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

package inet.applications.httptools.configurator;

//
// Controller for simulations which use the HttpTools browser and
// server components. A single controller must exist at the scenario level in each
// simulation.
//
// The purpose of the HttpController object is to support the server and browser components.
// A single controller instance must exist in all scenarios in which the browser and server
// components are used. Server components register their URL, OMNeT++ module name and listening
// port with the controller upon initialization.
// The controller serves as a lookup service for OMNeT++ module names from the URLs used to
// identify the Web server instances. It also provides a random Web site lookup facility, used by
// the browser component in its random browsing mode. <i>Uniform</i> and <i>Zipf</i> distributions
// are currently supported for random server selection. The controller parameters are defined
// in a XML file, as previously discussed for the server and browser components.
// The controller supports <i>popularity modification events</i> for individual sites.
// Such events are defined in a script file, assigned to the controller by an initialization
// parameter, which can be used for modeling a variety of situations involving dynamically
// changing server popularity.
// A popularity modification event at a time T<sub>init</sub> causes a site to be added to a
// custom selection list.
// A random site request from a browser will then yield a hit from the custom list with
// p<sub>c</sub>=sum(p<sub>i</sub>), where p<sub>i</sub> is the enhanced selection probability
// of site <i>i</i> on the list. A particular site <i>i</i> is selected from the list
// with p=p<sub>i</sub>/p<sub>c</sub>. Conversely, a site is selected from the general population,
// using its assigned random distribution, with $p<sub>p</sub>=1-p<sub>c</sub>.
// The special probability of a site can be <i>amortized</i> by a constant <i>alpha</i> for
// each hit, eventually reducing it to zero, at which time it is removed from the special list.
//
// @author  Kristjan V. Jonsson
//
simple HttpController
{
    parameters:
        string events = default("");          // Popularity modification events defintion file.
        string eventsSection = default("");   // Section within the popularity modification file, if one is defined.
        xml config;                           // XML configuration file.
        @lifecycleSupport;
}

