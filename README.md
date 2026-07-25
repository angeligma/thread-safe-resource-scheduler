## Slug Dining Resource Scheduler

A synchronization library in C that manages concurrent access to a
capacity-limited shared resource — a dining hall — coordinating many
student threads entering/leaving against a cleaning service thread that
requires the hall to be completely empty and exclusive, all without busy
waiting. Includes a starvation-prevention extension so the cleaning service
is guaranteed a turn even under continuous student traffic.

**Stack:** C · POSIX threads (`pthread_mutex_t`, `pthread_cond_t`) ·
semaphores · Valgrind

### Overview

Models a real concurrency pattern that shows up constantly in systems
programming: many equivalent "reader-like" participants (students) can
share a resource simultaneously up to a fixed capacity, but a distinguished
"writer-like" participant (the cleaning service) needs the resource
entirely to itself, and neither side is allowed to spin-wait for their turn.
The library exposes a small blocking API — enter/leave for students, enter/
leave for cleaning — and callers never need to reason about locks directly.

### Highlights

- **Bounded concurrent access with a hard capacity limit** — any number of
  students can be inside the dining hall simultaneously up to a
  caller-specified capacity; once full, additional students block until a
  seat frees up, with no risk of overshooting the limit under concurrent
  entry attempts
- **Full mutual exclusion for cleaning** — the cleaning service can only
  begin once every student has left, and once cleaning has started, no new
  student is allowed in until it finishes — enforced even when students and
  the cleaning provider are racing to acquire access at the same moment
- **Zero busy-waiting** — every blocking condition (hall full, cleaning in
  progress, students still present) is implemented with condition
  variables/semaphores rather than polling loops, so waiting threads
  consume no CPU while blocked
- **Starvation-free cleaning (extra credit)** — under the base design, a
  steady stream of arriving students could keep the cleaning provider
  waiting indefinitely, since students are always free to enter as long as
  there's room. The extended version guarantees the cleaning provider a
  bounded wait by giving it priority once it announces its intent to
  clean — new students stop being admitted the moment cleaning is
  requested, rather than only once the hall happens to empty out on its own
- **Leak-free, crash-free under concurrency** — all synchronization state is
  properly initialized and torn down, verified free of memory leaks and
  segfaults under Valgrind across concurrent student/cleaning scenarios

### How it works

The dining hall's state — how many students are currently inside, its
capacity, and whether cleaning is active or pending — is tracked internally
and protected by a mutex so that every access to it is atomic, even when
many student and cleaning threads are calling into the library at once.

When a student tries to enter, the library checks whether there's available
capacity and whether cleaning is active or pending. If either condition
blocks entry, the calling thread waits on a condition variable rather than
spinning, and is only woken up once the state actually changes in a way
that might let it proceed. When a student leaves, the occupant count is
decremented and any threads waiting on that count — other students hoping
for a free seat, or the cleaning provider waiting for the hall to empty —
are signaled to re-check whether they can now proceed.

The cleaning provider's entry call blocks until the hall is completely
empty and no other cleaning is already underway, then marks the hall as
being cleaned so no student can enter in the meantime. Its leave call
clears that state and wakes any threads that had been waiting for cleaning
to finish.

For the starvation-free extension, the moment a cleaning request arrives,
the hall is marked as "cleaning pending" even before it's actually empty.
From that point on, new student entry attempts are blocked immediately
rather than being allowed to keep racing in ahead of the cleaner — so the
cleaning provider's wait is bounded by however long it takes the students
already inside to leave, rather than being at the mercy of an unbroken
stream of new arrivals.
