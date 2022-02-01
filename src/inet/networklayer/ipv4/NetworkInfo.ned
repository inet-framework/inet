//
// Copyright (C) 2007 Vojtech Janota
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
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

