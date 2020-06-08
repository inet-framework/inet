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

#include "inet/common/LinuxUtils.h"

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
void make_capabilities_inheritable(const std::vector<cap_value_t>& capabilities)
{
    cap_t caps;

    caps = cap_get_proc();
    if (caps == NULL)
        throw "Failed to load capabilities";
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, NULL));
    if (cap_set_flag(caps, CAP_INHERITABLE, capabilities.size(), capabilities.data(), CAP_SET) == -1)
        throw "Failed to set inheritable";
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, NULL));
    if (cap_set_proc(caps) == -1)
        throw "Failed to set proc";
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, NULL));
    caps = cap_get_proc();
    if (caps == NULL)
        throw "Failed to load capabilities";
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, NULL));

    /* ------ */

    for (cap_value_t cap : capabilities)
        if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) == -1)
            throw "Failed to pr_cap_ambient_raise!";

    /* checking... (but the ambient flag is not visible...) */
    caps = cap_get_proc();
    if (caps == NULL)
        throw "Failed to load capabilities";
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, NULL));
}

// this does not really reset the i flag, instead, removes the capabilities from the "ambient set"
void make_capabilities_uninheritable(const std::vector<cap_value_t>& capabilities)
{
    for (cap_value_t cap : capabilities)
        if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, cap, 0, 0) == -1)
            throw "Failed to pr_cap_ambient_lower!";

    cap_t caps = cap_get_proc();
    if (caps == NULL)
        throw "Failed to load capabilities";
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, NULL));
}


int run_command(std::vector<const char *> args, bool wait_for_exit, bool hand_down_capabilities)
{
    ASSERT(args.size() >= 0 && args.front() != nullptr);
    std::unique_ptr<InheritableCapabilityContext> cntxt;

    if (hand_down_capabilities) {
        std::unique_ptr<InheritableCapabilityContext> c(new InheritableCapabilityContext);
        cntxt.swap(c);
    }

  pid_t child_pid;
  int child_status;

  child_pid = fork();
  if(child_pid == 0) {
    /* This is done by the child process. */
    std::vector<const char*> args_z = args;
    if (args.back() != nullptr)
        args_z.push_back(nullptr);
    execvp(args_z[0], const_cast<char**>(args_z.data()));

    /* If execvp returns, it must have failed. */

    printf("Unknown command\n");
    exit(0);
  }
  else {
    if (wait_for_exit) {
        /* This is run by the parent.  Wait for the child
            to terminate. */
        pid_t tpid = -1;
        do {
        tpid = wait(&child_status);
        if(tpid != child_pid)  {
            printf("terminated: %d", tpid);
        }
        } while(tpid != child_pid);

        return child_status;
    }
    else
        return -1;
  }
}


} // namespace inet

