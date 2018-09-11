//
// Copyright (C) 2007 Vojtech Janota
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.networklayer.ipv4;

//
// ~NetworkInfo does not send or receive messages and has no parameters
// either. Instead it is invoked from ~ScenarioManager scripts. E.g.:
//
// <pre><nohtml>
// <scenario>
//   <at t="2">
//     <routing module="NetworkInfo" target="LSR2" file="LSR2_002.txt"/>
//   </at>
// </scenario>
// </nohtml></pre>
//
// It is currently only used in the Quagga/INET validation scenarios, and supports following
// commands:
//
// - <code>routing</code>: dumps routing table of the <tt>target</tt> node into
//   specified <tt>file</tt>. Optionally <tt>mode</tt> attribute can be set to "a"
//   to have the output appended (default is to truncate target file if such exists).
//   Setting attribute <tt>compat</tt> to value "linux" modifies the
//   output to look like `route -n | sort -r` output on linux...
//
simple NetworkInfo
{
}

