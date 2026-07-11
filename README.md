# sludgepipe

This is a C telemetry decoder designed to be as badly written as humanly possible while still barely working. The design must make every single mistake on the checklist, and be absolutely unreadable to anyone but myself. No AI use allowed, it's a learning project :)

#### Bad design

- [ ] one main.c file mixing every responsibility
- [ ] massive switch statement with copy-pasted logic that is subtly out of sync
- [ ] everything is a global variable
- [ ] configuration is editing a bunch of macros
- [ ] no named constants
- [ ] make every function a void
- [ ] exit(1) inside a helper function

#### Memory & Buffers
- [ ] not bounds-checking an input into a buffer
- [ ] ring buffer with an off-by-one error
- [ ] fields extracted by casting haphazardly
- [ ] sprintf, strcpy, strcat
- [ ] malloc on every packet that leaks when there's an error
- [ ] use after free

#### Undefined behaviour & arithmetic

- [ ] timestamp that overflows
- [ ] bit shifting a signed integer
- [ ] bit shifting by greater than the width of the type
- [ ] using an uninitialised struct as a testing input for "jitter"
- [ ] packing a float using union type-punning in one place and int* cast in another
- [ ] using %d for uint32_t, %ld for size_t
- [ ] baking in little-endianness silently

#### Performance

- [ ] parse inputs one byte at a time with fgetc()
- [ ] CSV output rebuilt for the entire file every time a packet is received
- [ ] calculate a rolling median by qsorting the entire history every time
- [ ] O(N) comparison when an index lookup would have worked
- [ ] Passing a struct by value through several layers

#### Concurrency

- [ ] making a handshake somehow work by tuning a sleep
- [ ] adding a thread that makes things slower instead of faster
- [ ] false sharing
- [ ] volatile int as a global shutdown flag
- [ ] race condition that causes a deadlock on an error path

#### Tooling

- [ ] build.sh is two lines
- [ ] debugging with commented out printfs
- [ ] no tests

## Goal

Once the checklist is complete, the goal will be to write a test suite, characterise the behaviour, and systematically fix all the problems through a series of atomic refactors with no breaking changes.

Tools to learn to use during this phase:

- gcc flags
- ASan
- UBSan
- TSan
- valgrind
- clang-tidy
