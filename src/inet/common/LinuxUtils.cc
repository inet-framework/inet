//
// Copyright (C) 2020 Opensim Ltd
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

#include "inet/common/LinuxUtils.h"

#ifdef __linux__

#include "inet/common/InheritableCapabilityContext.h"

#include <memory>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/capability.h>
#include <err.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

namespace inet {

// source: https://unix.stackexchange.com/a/581945/329864
// in addition to adding the i flag, this adds the capabilities to the "ambient set"
void makeCapabilitiesInheritable(const std::vector<cap_value_t>& capabilities)
{
    cap_t caps = cap_get_proc();
    if (caps == nullptr)
        throw cRuntimeError("Failed to load capabilities");
    //printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, nullptr));

    if (cap_set_flag(caps, CAP_INHERITABLE, capabilities.size(), capabilities.data(), CAP_SET) == -1)
        throw cRuntimeError("Failed to set capabilities to be inheritable");
    //printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, nullptr));

    if (cap_set_proc(caps) == -1)
        throw cRuntimeError("Failed to set capabilities to process: %s", strerror(errno));

    for (cap_value_t cap : capabilities)
        if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) == -1)
            throw cRuntimeError("Failed to add capability '%s' to the ambient set: %s", cap_to_name(cap), strerror(errno));

    /*
    // checking... - but the ambient flag is not visible n the text anyways
    caps = cap_get_proc();
    if (caps == NULL)
        throw cRuntimeError("Failed to load capabilities");
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, nullptr));
    */
}

// this does not really reset the i flag, instead, removes the capabilities from the "ambient set"
void makeCapabilitiesUninheritable(const std::vector<cap_value_t>& capabilities)
{
    for (cap_value_t cap : capabilities)
        if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, cap, 0, 0) == -1)
            throw cRuntimeError("Failed to remove capability '%s' from the ambient set: %s", cap_to_name(cap), strerror(errno));
/*
    cap_t caps = cap_get_proc();
    if (caps == NULL)
        throw cRuntimeError("Failed to load capabilities");
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, NULL));
*/
}

int execCommand(const std::vector<const char*>& args, bool waitForExit, bool handDownCapabilities)
{
    ASSERT(args.size() >= 0 && args.front() != nullptr);

    // making use of RAII, but conditionally...
    std::unique_ptr<InheritableCapabilityContext> cntxt;
    if (handDownCapabilities) {
        std::unique_ptr<InheritableCapabilityContext> temp(new InheritableCapabilityContext);
        cntxt.swap(temp);
    }

    pid_t childPid = fork();

    if (childPid == 0) {
        // This is done by the child process.
        std::vector<const char*> args_z = args;
        args_z.push_back(nullptr);
        execvp(args_z[0], const_cast<char**>(args_z.data()));

        // If execvp returns, it must have failed.
        throw cRuntimeError("Failed to execute '%s': %s", args_z[0], strerror(errno));
    }
    else {
        // This is run by the parent.  Wait for the child to terminate if needed.
        if (waitForExit) {
            int childStatus;
            pid_t tpid = -1;

            do {
                tpid = wait(&childStatus);
            }
            while (tpid != childPid);

            return childStatus;
        }
        else
            return -1;
    }
}

} // namespace inet

#endif // __linux__
