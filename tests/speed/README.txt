This folder contains speed tests for the INET framework.

In general, it must be noted that it's notoriously difficult to do reliable
speed measurements. This file describes several common pitfalls that must be
avoided.

First of all, speed tests should measure CPU time instead of wall clock time.
The reason is that the elapsed wall clock time during the simulation depends
on many factors. For example, the elapsed wall clock time depends on what the
operating system is doing while the simulation is running.

Second, even though measuring CPU time is more reliable then measuring the wall
clock time, nevertheless the results still depend on several factors. They vary
with the used CPU hardware, the used compiler, various compiler optimization
options, and so on.

For even more accurate results it may be a good idea to run the same simulation
several times and use the mean runtime. Every now and then the measured runtime
may be quite different from the rest of the repetitions due to various reasons.
Therefore ruling out the outlier measurements produce more reliable results.

During the measurements it's advisable to disable all output including most of
the standard output and output file recording. Writing files may significantly
decrease performance and most of the time it's not the output file recording
performance what we are aiming for.

Finally, to get the most reliable results from your measurements it's strongly
suggested to disable all but one CPU. The reason is that the more CPU you are
using the more likely they will affect each other due to various shared hardware
resources. You can disable individual CPUs using the following command on linux:

echo 0 | sudo tee /sys/devices/system/cpu/cpu1/online

In the simplest case you can run a single speed test with the following command:
./speedtest -m TwoHosts

For more details run: ./speedtest -h

