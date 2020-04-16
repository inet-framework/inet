//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.configurator;

//
// This module allows to configure network scenarios at layer 2.
// The ~Stp and ~Rstp related parameters such as link cost, port priority
// and the "is-edge" flag can be configured with XML files.
//
// This module is based on ~Ipv4NetworkConfigurator. The optional selector
// attributes: @hosts, @names, @towards, @among are also comes with this
// configurator and each of them behaves similarly to its ~Ipv4NetworkConfigurator equivalent.
// @ports selector added to configuring per-port parameters in Ieee8021dInterfaceData.
//
// The following example configures port 5 (if it exists) on all switches to cost 19 and
// priority 32768:
// <pre>
// <config>
//   <interface hosts='**' ports='5' cost='19' priority='32768'/>
// </config>
// <pre>
//
// For more information about the usage of the selector attributes see ~Ipv4NetworkConfigurator.
//
simple L2NetworkConfigurator
{
    parameters:
        @display("i=block/cogwheel");
        xml config = default(xml("<config><interface hosts='**' ports='**' cost='19' priority='128' edge='false'/></config>"));
}

