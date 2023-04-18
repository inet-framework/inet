"""
Provides functionality for creating and managing SSH clusters.

Please note that undocumented features are not supposed to be used by the user.
"""

import dask
import dask.distributed
import logging
import os
import socket
import time

from inet.common.util import *

_logger = logging.getLogger(__name__)

class SSHCluster:
    """
    Represents an SSH cluster. An SSH cluster is basically a set of network nodes, most often connected to a single LAN,
    all of which can login into each other using SSH passwordless login. The SSH cluster utilizes all network nodes with
    automatic and transparent load balancing as if it were a single computer.

    There are a couple of pitfalls that can prevent the SSH cluster from working correctly. If you encounter problems
    with the SSH cluster, make sure the following conditions are met:

     - scheduler and worker node hostnames can be resolved to IP addresses on all nodes
     - all nodes can SSH login into all other nodes (even themselves!) without any user interaction (no password is required)
     - the Python version and all required Python library versions are the same on all nodes
     - no firewall rule prevents the scheduler to connect to the workers
     - simulation projects are compiled both in release and debug versions
    """

    def __init__(self, scheduler_hostname, worker_hostnames, nix_shell=None):
        """
        Initializes a new SSH cluster object.

        Parameters:
            scheduler_hostname (string):
                The hostname of the scheduler node.

            worker_hostnames (list):
                The hostnames of the worker nodes.

            nix_shell (string or None):
                A Nix shell name that is used to execute tasks on the SSH cluster for repeatable results.
        """
        self.scheduler_hostname = scheduler_hostname
        self.worker_hostnames = worker_hostnames
        self.nix_shell = nix_shell

    def start(self):
        """
        Starts the SSH cluster and the dashboard web page.
        """
        remote_python = (f"NIX_SHELL={self.nix_shell} " if self.nix_shell else "") + "~/omnetpp-distribution/inet/bin/inet_ssh_cluster_python"
        _logger.info(f"Starting SSH cluster: scheduler={self.scheduler_hostname}, workers={self.worker_hostnames}, remote_python={remote_python}, dashboard=http://localhost:8797")
        cluster = dask.distributed.SSHCluster([self.scheduler_hostname, *self.worker_hostnames],
                                              remote_python=remote_python,
                                              scheduler_options={"dashboard_address": ":8797"})
        # registers the cluster as the default scheduler for future compute calls
        dask.distributed.Client(cluster)

    def run_gethostname(self, num_tasks=12):
        """
        Collects the result of calling :py:func:`socket.gethostname` multiple times on the SSH cluster. This function is
        primarily useful to test the correct operation of the SSH cluster. The result should contain a permutation of the
        hostnames of all worker nodes.

        Parameters:
            num_tasks (int):
                The number of times :py:func:`socket.gethostname` is called.

        Returns (List):
            The hostname of worker nodes where :py:func:`socket.gethostname` was called.
        """
        delayed_hostnames = list(map(lambda i: dask.delayed(socket.gethostname)(), range(0, num_tasks)))
        delayed_result = dask.delayed(lambda elements: ", ".join(elements))(delayed_hostnames)
        return delayed_result.compute()
