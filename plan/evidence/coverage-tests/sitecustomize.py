# The measurement runs each job on the cores of a GitHub runner (taskset), but Python's cpu_count()
# reports all cores of the host. On a runner both numbers are the same, so make them the same here.
import multiprocessing
import os
_count = len(os.sched_getaffinity(0))
multiprocessing.cpu_count = lambda: _count
os.cpu_count = lambda: _count
