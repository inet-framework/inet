//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_LINUXUTILS_H
#define __INET_LINUXUTILS_H

#ifdef __linux__

#include <vector>
#include <sys/capability.h> // cap_value_t

namespace inet {

static const std::vector<cap_value_t> DEFAULT_REQUIRED_CAPABILITIES = {
    CAP_NET_ADMIN,   // to change host networking and interface configuration (IP addresses, routes, etc)
    CAP_NET_RAW,     // to use raw sockets
    CAP_SYS_ADMIN,   // to switch between network namespaces back and forth
    CAP_DAC_OVERRIDE // to create new bind mounts (files in /var/run/netns) for newly created network namespaces
};

void makeCapabilitiesInheritable(const std::vector<cap_value_t>& caps = DEFAULT_REQUIRED_CAPABILITIES);

void makeCapabilitiesUninheritable(const std::vector<cap_value_t>& caps = DEFAULT_REQUIRED_CAPABILITIES);

int execCommand(const std::vector<const char *>& args, int *pid = nullptr, bool waitForExit = true, bool handDownCapabilities = false);

} // namespace inet

#endif // __linux__
// #else error? warning?

#endif // ifndef __INET_LINUXUTILS_H

