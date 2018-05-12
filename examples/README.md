Examples
--------

These are various example programs. All of them do performance measurements at
this point.

To run fiber_benchmarks, it's recommended to do this (after building):

    $ echo madvise | sudo tee /sys/kernel/mm/transparent_hugepage/enabled
    $ sudo cpupower frequency-set -g performance
    $ taskset -a -c 2 ./examples/fiber_benchmarks

Skip taskset if you only have one CPU core.

That will give you reasonably stable results that can be usefully compared
between the single-threaded and multi-threaded tests.

Whether to run with THP or not depends on what you want to measure.

Check the source code to figure out exactly what each test measures. Each
simulates one specific scenario (sometimes in an ideal setting that gives an
upper bound on possible performance only). Settings can be changed in the code.
