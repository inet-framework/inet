//
// Copyright (C) 2006-2015 Opensim Ltd
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

#include <vector>

#include <sys/capability.h>
namespace inet {

static const std::vector<cap_value_t> required_capabilities = {
        CAP_NET_ADMIN, // to change interface configuration (IP addresses, etc)
        CAP_NET_RAW, // to be able to use raw sockets
        CAP_SYS_ADMIN, // to be able to switch between network namespaces back and forth
        CAP_DAC_OVERRIDE // to be able to create new bind mounts (files in /var/run/netns) for newly created network namespaces
};

// source: https://unix.stackexchange.com/a/581945/329864
// in addition to adding the i flag, this adds the capabilities to the "ambient set"
void make_capabilities_inheritable(const std::vector<cap_value_t>& caps = required_capabilities);
// this does not really reset the i flag, instead, removes the capabilities from the "ambient set"
void make_capabilities_uninheritable(const std::vector<cap_value_t>& caps = required_capabilities);

int run_command(std::vector<const char *> args, bool wait_for_exit = true, bool hand_down_capabilities = false);
} // namespace inet

#endif // ifndef __INET_LINUXUTILS_H

