SPEED TESTS FOR INET
====================

This folder contains speed tests for the INET Framework using the examples.

In general, it must be noted that it's notoriously difficult to do repeatable
speed measurements. This file describes several common pitfalls that should be
avoided. Before optimizing, it's essential to have a speed test that produces
repeatable results. Otherwise it's unclear if any particular optimization is
right or wrong.

The intended audience of this document are the maintainers of the INET Framework.

1. Fix the Environment

Make sure that the measurements take place in the same hardware and software
environment. Start with creating a set of reference measurements, then change
as little as possible, and compare the results. Changing more than one thing
at a time can easily lead to the wrong conclusions.

2. Measure CPU Time

Measure CPU time instead of wall clock time. The elapsed wall clock time depends
on many factors some of which are difficult if not impossible to control. For
example, the elapsed wall clock time depends on what the operating system is
doing while the simulation is running.

3. Measure Directly

Measure only the target part and leave out any other tasks if possible. It's
very tempting to measure using the standard time utility of linux. Unfortunately
it's results are not so repeatable because it includes the runtime of the whole
process. Modern linux also includes the so perf tools that supports measuring
many interesting things done by the operating system. Either way, if the goal is
measuring how long the simulation takes, then the best thing to do is to put in
a few lines of code around that part such as this:

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
     ...
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
    int64_t time = ((int64_t)end.tv_sec * 1000000000L + end.tv_nsec) - ((int64_t)start.tv_sec * 1000000000L + start.tv_nsec);
    out << "Runtime: " << time / 1.0E+9 << "\n";

4. Calculate Mean

Repeat the same simulation several times, and calculate the mean runtime accross
repetitions. Every now and then the measured runtime may be quite different
from the rest of the repetitions due to various reasons. Therefore ruling out
the outlier measurements produce more repetable results.

5. Disable All Output

Disable or reduce all unnecessary output including most of standard output and
also output file recording. Writing files significantly decreases performance,
and most of the time it's not the output file recording performance what must
be measured and optimized.

6. Configure Compiler Optimization

Select the right compiler optimization, because optimization flags drastically
affect runtime performance. For example, the default -O0 optimization setting
for GCC is several times slower than the -O3 setting. It's important to note
that compiling using GCC for debugging (i.e. with debug symbols) must be done
with -Og optimization flag as suggested by the GCC user manual.

7. Increase Process Priority

Increase process priority to the maximum allowed to minimize context switches.
Even though measuring CPU time is more repetable than measuring wall clock time,
nevertheless the results still depend on several factors. The most important
factor is the number of context switches to other threads and processes run by
the operating system.

8. Disable Multiple CPUs

Disable all but one CPUs, because the more CPUs used the more likely they will
affect each other due to various shared hardware resources. Using a single CPU
ensures that no such effect takes place.

9. Disable CPU Frequency Scaling

Disable CPU frequency scaling used by modern operating systems. This feature is
used to reduce power consumption (mostly in portable devices), and it makes the
measurements less repeatable.

Making sure the cpu's frequency doesn't scale during the speedtest:

$ sudo pstate-frequency -S --turbo off # turn off frequency scaling
$ sudo pstate-frequency -S --min 100   # set frequency of cpu cores to 100% (base frequency)
$ sudo pstate-frequency -S --max 100

$ pstate-frequency -G # to check if turbo is off and the percentages are properly set to around 100% (the cpu frequencies reported here can be misleading)
$ cpufreq-info        # check the actual cpu core frequencies, should be around the base frequency

10. Disable Thermal Throttling

Ensure thermal throttling does not alter CPU performance. Monitor CPU temperatures
and use sufficient cooling.

11. Minimize Interrupts

Minimize hardware and software interrupts during measurements to reduce unexpected
performance impacts.

12. Change CPU Affinity

Set CPU affinity to bind the process to a specific CPU. This ensures the process runs only on designated CPUs. Use the taskset command:

$ taskset -c 0 ./inet_profile -u Cmdenv

13. Isolate CPU(s)

Isolate specific CPUs using kernel parameters to dedicate them exclusively to
the measurement process. Modify the GRUB configuration:

    GRUB_CMDLINE_LINUX="isolcpus=14,15 nohz_full=14,15 rcu_nocbs=14,15"

14. Disable Power Management

Disable all CPU power management features to ensure consistent performance.

15. Use Real-Time Kernel

Consider using a real-time kernel for higher priority and more deterministic task scheduling.

16. Be aware of clock granularity

Understand the resolution of the timing functions used, as insufficient granularity
can lead to inaccurate measurements.

17. User time vs system time

Differentiate between user time (time spent in user-mode processes) and system
time (time spent in kernel-mode processes). This distinction helps interpret
performance results.

18. Wall-clock time vs CPU time

Wall-clock time includes all elapsed time, whereas CPU time only measures the
time actively used by the process.

19. Know What is Measured

Measure CPU time, CPU cycles or CPU instructions to analyze performance at the
hardware level. Example APIs include:

    _rdtsc
     clock()
     std::chrono
     clock_gettime(CLOCK_PROCESS_CPUTIME_ID
     struct rusage r;
     getrusage(RUSAGE_SELF, &r);

20. CPU Cache

Be mindful of cache effects on performance. Warm-up runs can help mitigate
cache-related variability. A context switch, to handle an interrupt or to run
another task, clears the CPU caches, so most likely increases CPU time.

21. Useful Command-Line Tools

$ time
$ cpufreq-info
$ lscpu
$ perf # this can display context switches, CPU cycles, CPU instructions, CPU migrations
$ taskset
$ mpstat

22. Important mapped files

Use the following for relevant configuration and status information:

$ cat /proc/cmdline
$ cat /proc/interrupts
$ cat /proc/self/status | grep Cpus_allowed_list
$ cat /proc/sys/kernel/perf_event_paranoid
$ cat /sys/devices/system/cpu/smt/control
$ cat /sys/devices/system/cpu/intel_pstate/no_turbo
$ cat /sys/devices/system/cpu/cpu14/cpufreq/scaling_max_freq
$ cat /sys/devices/system/cpu/cpu14/cpufreq/scaling_cur_freq
$ cat /sys/devices/system/cpu/cpu15/cpufreq/scaling_governor
$ cat /proc/irq/*/smp_affinity

23. Configure OMNeT++ Features

Disable Qtenv, Tkenv, Osg, OsgEarth, Parsim and other unnecessary optional OMNeT++
features. Some of these features link with several additional libraries that
may affect performance. For example, linking with pthread library might decrease
the performance of std::shared_ptr by using locking to provide atomic operations
for the reference counter.

24. Configure INET Features

Disable VOIP, visualization, and other unnecessary optional INET features.
Some of these features link with several additional libraries that may affect
performance.

25. Disable Self Checking

Disable all self checking code that would unnecessarily affect the measurement
results. Even if these would be included in the final product, they don't change
the output of the program during the measurement. For example, the INET packet
API contains two sets of self checking operations, see Chunk.h for more details.

26. Running Tests

In the simplest case, you can run all speed tests with the following command:

$ inet_run_speed_tests

For more details on command line options, run:

$ inet_run_speed_tests -h
