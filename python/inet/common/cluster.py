import dask
import dask.distributed
import time

from dask import delayed
from dask.distributed import Client, SSHCluster

def start_cluster():
    global client
    cluster = dask.distributed.SSHCluster(["192.168.0.200", "192.168.0.200", "192.168.0.52"], remote_python="~/workspace/inet/bin/inet_ssh_cluster_python", scheduler_options={"dashboard_address": ":8797"})
    client = Client(cluster)

def run_gethostname_on_cluster():
    import socket
    def concatenate(elements):
        return ", ".join(elements)
    delayed_hostnames = list(map(lambda i: delayed(socket.gethostname)(), range(0, 12)))
    delayed_result = delayed(concatenate)(delayed_hostnames)
    return delayed_result.compute()

def run_multiple_tasks_on_cluster(multiple_tasks):
    # TODO add this to MultipleTasks class to make this transparently available as an option 
    def run_task(task):
        return task.run()
    delayed_results = list(map(lambda task: delayed(run_task)(task), multiple_tasks.tasks))
    return multiple_tasks.multiple_task_results_class(results=dask.compute(*delayed_results), multiple_tasks=multiple_tasks)
