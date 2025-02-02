Write and test alternative implementations of condition variables

The purpose of the condvar repository is to host code which tests, benchmarks and profiles various implementations of condition variables.   The tests intentionally create massive lock contention in a turn-taking system based on [Dekker's Algorithm](https://en.wikipedia.org/wiki/Dekker%27s_algorithm).  The contention is large because there are only two kinds of turns, but there are 4 threads that want them.   Thus waking a random waiter will activate one which can't run 1/3 of the time.

The Makefile intentionally has compiler optimization disabled.   Turning on optimization would require memory barriers and/or atomic variables.  In fact, the recent ["Formal Verification"](https://accu.org/journals/overload/32/183/melinte/) by Aurelian Melinte in ACCU's *Overload* journal mentions that

> the Dekker algorithm as detailed below was proven to be unsafe on a machine without atomic reads and writes 

which implies that code without atomic reads and writes and consequent memory ordering is also likely unsafe.

A different *Overload* article, ["In an Atomic World"](https://accu.org/journals/overload/32/182/teodorescu/) by Lucian Radu Teodorescu inspired a large fraction of the code.

A primary but eventual goal is to test the priority inheritance behavior of these algorithms on a CONFIG\_PREEMPT\_RT Linux system.   See [Problems with condition variables](https://wiki.linuxfoundation.org/realtime/documentation/howto/applications/application_base) in the Linux Foundation's realtime wiki for the motivation.
