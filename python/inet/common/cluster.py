import dask
import dask.distributed
import os
import socket
import time

from dask import delayed
from dask.distributed import Client, SSHCluster

#
# make sure that omnetpp and INET are compiled in release version
# make sure that your scheduler and worker hostnames can be resolved on all hosts
# make sure that all hosts can ssh login into all other hosts (even themselves) without any user interaction
# make sure that the Python versions are the same and all required library versions are the same on all nodes
# make sure that no firewall rule prevents the scheduler to connect the workers
#
# example usage:
#
# client = start_cluster()
# inet_project.copy_binary_simulation_distribution_to_cluster(client.cluster) # optional step
# multiple_tasks = get_simulation_tasks(mode="release", build=False, sim_time_limit="1s", filter="showcases/tsn/trafficshaping")
# run_multiple_tasks_on_cluster(multiple_tasks) # can be run multiple times
#
# open http://localhost:8797 to see the dashboard
#
def start_cluster(scheduler_hostname="valardohaeris", worker_hostnames=["valardohaeris", "valarmorghulis"]):
    cluster = dask.distributed.SSHCluster([scheduler_hostname, *worker_hostnames],
                                          remote_python="~/workspace/inet/bin/inet_ssh_cluster_python",
                                          scheduler_options={"dashboard_address": ":8797"})
    # registers the cluster as the default scheduler for future compute calls
    return Client(cluster)

# this is an example to test the cluster
def run_gethostname_on_cluster():
    delayed_hostnames = list(map(lambda i: delayed(socket.gethostname)(), range(0, 12)))
    delayed_result = delayed(lambda elements: ", ".join(elements))(delayed_hostnames)
    return delayed_result.compute()

# this can be used to run multiple tasks (simulations, tests, etc.)
def run_multiple_tasks_on_cluster(multiple_tasks):
    # TODO add this to MultipleTasks class to make this transparently available as an option 
    def run_task(task):
        return task.run()
    delayed_results = list(map(lambda task: delayed(run_task)(task), multiple_tasks.tasks))
    return multiple_tasks.multiple_task_results_class(results=dask.compute(*delayed_results), multiple_tasks=multiple_tasks)
