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
#include <sys/capability.h>
#include <err.h>

// source: https://unix.stackexchange.com/a/581945/329864
void make_capabilities_inheritable()
{
    cap_t caps;

    /* --- this part is only needed if there is no i flag set on the binary capabilities  --- */

    caps = cap_get_proc();
    if (caps == NULL)
        throw "Failed to load capabilities";
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, NULL));
    cap_value_t cap_list[3];
    cap_list[0] = CAP_NET_ADMIN;
    cap_list[1] = CAP_NET_RAW;
    cap_list[2] = CAP_SYS_ADMIN;
    if (cap_set_flag(caps, CAP_INHERITABLE, 3, cap_list, CAP_SET) == -1)
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

    if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, CAP_NET_ADMIN, 0, 0) == -1)
        throw "Failed to pr_cap_ambient_raise!";
    if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, CAP_NET_RAW, 0, 0) == -1)
        throw "Failed to pr_cap_ambient_raise!";
    if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, CAP_SYS_ADMIN, 0, 0) == -1)
        throw "Failed to pr_cap_ambient_raise!";

    /* checking... (but the ambient flag is not visible...) */
    caps = cap_get_proc();
    if (caps == NULL)
        throw "Failed to load capabilities";
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, NULL));
}

void make_capabilities_uninheritable()
{
    if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, CAP_NET_ADMIN, 0, 0) == -1)
        throw "Failed to pr_cap_ambient_lower!";
    if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, CAP_NET_RAW, 0, 0) == -1)
        throw "Failed to pr_cap_ambient_lower!";
    if (prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_LOWER, CAP_SYS_ADMIN, 0, 0) == -1)
        throw "Failed to pr_cap_ambient_lower!";

    cap_t caps = cap_get_proc();
    if (caps == NULL)
        throw "Failed to load capabilities";
    printf("DEBUG: Loaded Capabilities: %s\n", cap_to_text(caps, NULL));
}



#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int runcmd(char *args[], bool wait_for_exit = true)
{
  pid_t child_pid;
  int child_status;

  child_pid = fork();
  if(child_pid == 0) {
    /* This is done by the child process. */

    execvp(args[0], args);

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

#include <string>

int oldNs = -1;
void set_net_ns(const char *networkNamespace)
{
    if (oldNs == -1)
        oldNs = open("/proc/self/ns/net", O_RDONLY);
    if (networkNamespace != nullptr && *networkNamespace != '\0') {
        std::string namespaceAsString = "/var/run/netns/";
        namespaceAsString += networkNamespace;
        int newNs = open(namespaceAsString.c_str(), O_RDONLY);
        if (newNs == -1)
            throw "Cannot open network namespace";
        if (setns(newNs, 0) == -1)
            throw "Cannot change network namespace";
        close(newNs);
    }
}

void reset_net_ns()
{
    setns(oldNs, 0);
}



int main(int ac, char **av){

    const char *args[] = { "ip", "addr", 0};
    runcmd( const_cast<char**>(args));
    printf("BOOP\n");
    set_net_ns("srv");
    const char *args_cons[] = { "konsole", "--hold", "-e", "ip", "addr", 0};
    runcmd( const_cast<char**>(args_cons), false);
    printf("BOOP\n");

    reset_net_ns();

    runcmd( const_cast<char**>(args));
    printf("BOOP\n");



    try {
        make_capabilities_inheritable();
    } catch (const char *e) {
        printf("error: %s", e);
    }

    //const char *args[] = {"ip", "tuntap", "add", "tap7", "mode", "tap", 0};

    // sudo ip link set up dev tap6

    //execvp(args[0], const_cast<char**>(args));
    //runcmd( const_cast<char**>(args));

    const char *args2[] = {"ip", "netns", "exec", "srv", "ip", "addr", 0};
    runcmd( const_cast<char**>(args2));
    printf("BOOP\n");

    try {
        make_capabilities_uninheritable();
    } catch (const char *e) {
        printf("error: %s", e);
    }
    runcmd( const_cast<char**>(args2));
    printf("BOOP\n");



        try {
        make_capabilities_inheritable();
    } catch (const char *e) {
        printf("error: %s", e);
    }

    runcmd( const_cast<char**>(args2));
    printf("BOOP\n");



}
