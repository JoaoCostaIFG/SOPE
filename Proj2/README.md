# SOPE Proj 2

This project was developed by _João Costa_ (me), [João Lucas](https://github.com/joaolucasmartins)
and [Ricardo Fontão](https://github.com/ricardofontao2000) for our Operating
Systems class (SOPE) at [MIEIC, FEUP](https://sigarra.up.pt/feup/en/cur_geral.cur_view?pv_curso_id=742)
(2019/2020).  
All the details of the assignment can be found [here](/objectives.pdf).

## Compilation instructions

To compile this project, you just need to call `make` inside the project's
**src** directory.  
If you wish to clean the files resulting from a compilation, you can just
call `make clean` inside that same directory.

## Usage instructions

To run the server:

```bash
Qn <-t nsecs> [-l nplaces] [-n nthreads] fifoname
```

To run the client generator:

```bash
Un <-t nsecs> fifoname
```

To run some simple automated tests use the **ttest.sh** script:

```bash
./ttest.sh <2 for second part of the project tests>
```

NOTE: the client generator (**Un**) will instantly exit, reporting a failure,
if the server isn't running when it attempts the first communication. With
this in mind, starting both the server (**Qn**) and the client generator (**Un**)
at the same time, might result in a failure depending on how the OS schedules
their tasks.

## Part 1 implementation details

### Starting the programs

To parse the command line arguments, we make use of **getopt.h**. Both programs
start by parsing the command line arguments and stop if any error is found.

When the server starts, it remains in a **"standby mode"**, waiting for the first
client to show up. To do this, the server creates the public FIFO and a
named semaphore initialized at **0** when it starts. The server then calls
a `sem_timedwait()` on that named semaphore. The clients call check the value
of the named semaphore when they start. If that value is 0, they call `sem_post()`
on that semaphore.  
This ensures the server remains in a "ready" state after starting and that it
can close itself after the given time elapses, even if no client ever connects.
Without the named semaphore, there would be the possibility of the server
hanging on the `open()` call indefinitely.

### Client timeout

At the beginning of the run, the client program calculates the number of requests
it can create with a **200 milisecond** delay between each packet, without exceeding the
maximum run time passed in the command line arguments. It then creates 1 thread
for every **200 miliseconds**.  
When a thread gets notified, by the server, that the bathroom is closed, it also
notifies the client generator **Un** so it can stop creating more threads, join
the remaining active threads and exit.

### Server timeout

The server starts in a **"standy mode"**, as was described earlier. Before processing
the first request, the server sets up an **alarm signal** with the timeout passed
in the command line arguments. The handler of this signal sets a global variable
(initialized as false) to true. This flag indicates if whether the bathroom is
closed.

When one of the server threads is handling an incoming request, it checks the
value of the variable and decides if the request should be accepted based on
its value.

The handling of requests (be it acceptance, denial or error reporting) by the
server only stops when every writer connected to the **public FIFO** closes
their end of the **FIFO**. This ensures that every incoming request
is handled and that the request generators will be warned if they should give up,
or, in other words, stop sending requests early.  
After every writer quits, and the running time limit (passed in the command line
arguments) has elapsed, the server performs its clean-up operations and exits.
These clean-up operations are: deleting the **public FIFO** and the
**named semaphore**, joining with its threads and clearing the allocated memory
before exiting.

In order to avoid busy waiting when all the **writers** have closed their end
of the **public FIFO**, the server makes `sem_timedwait()` call (if that
situation comes up). The server will leave the `timedwait()` if a client (**U**)
connects or the given maximum running time has elapsed. If the `sem_timedwait()`
times out, and the **public FIFO** isn't open for writing, the server quits
(after performing the clean-up operations described earlier).

### Problem of note

The code for this part had a race condition that prevented the **U** program
from stopping. It created the private FIFO after sending its request,
which made it possible for the server threads to try to open a non-existing
FIFO and, consequently, assume the client gave up. This caused some client
threads to hang indefinitely while attempting to open the private FIFO.

## Part 2 implementation details

### Limiting the number of bathroom spots

To limit the number of bathroom spots (in the bathroom), we made use of a
_semaphore_ and a _mutex_. This semaphore was initialized with the number
of bathroom spots.  
Each time a **Q** thread attempts to secure a bathroom spot, it first calls
`sem_wait()` on the _semaphore_ referred above. These threads also call
`sem_post()` when they don't need their bathroom spot anymore. This makes
it so the semaphore represents the number of available bathroom spots.
If there are available bathroom spots, the thread will lock a _mutex_ so
it can safely access the structure that stores the bathroom spots, pick
one that's free and update the structure to reflect that change,
unlocking the _mutex_ afterwards.  
It should be noted that the _mutex_ locking and the `sem_post()` calls
can hang when/while waiting for other threads to "be done with their
business".

### Limiting the number of active server threads

In order to limit the number of active threads, we instantiated a
_semaphore_ and initialized it with the maximum number of active threads.
Each time **Q** creates a thread, (via `pthread_create()`), it calls
`sem_wait()` on the semaphore before creating the thread. Just before
a created thread exits, it calls `sem_post()` on the _semaphore_. These
keep the number of active threads up to date and the **Q** hangs
before creating a thread if the number of active threads is at the
maximum.

### Other changes

We decided to delete the _named semaphore_ that helped control
the start and end of the **Q** run. The purpose of this semaphore
was to grantee that the server didn't hang indefinitely if no one
connected to the FIFO and assure correct stop times if multiple
users connected/disconnected.  
We believe these extra functionalities were important, but they
also made the code more complex. Since this is a school project
and the tests made by the teacher indicate there won't be any
kind of multiple users (**U**) tests, we decided to delete it.

### Cleanups/Fixes

Cleaned unused code from the project.  
Simplified/updated the Makefile.
Corrected **U** log file not showing the acquired bathroom spot.

## Licensing

The code on this repository is licensed under the **GPL3 license**
(see the LICENSE file).
