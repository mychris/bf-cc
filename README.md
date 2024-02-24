# bf-cc

A brainfuck interpreter and run-time compiler written in C++. Currently, the
following platforms are supported:

- Linux on x86-64
- Windows 10 on x86-64

The input program is translated into a minimalistic IR, optimized, and then
either interpreted, or compiled to machine code and then executed.
The optimizer is capable of performing some very basic optimizations and
could be enhanced further.

The runtime compiler uses simple, hard-coded instruction templates for each
OpCode of the IR.

## Command line interface

Usage: `bf-cc [-h] [-O(0|1|2|3)] [-mMEMORY_SIZE] [-e(keep|0|1)] [(-i|-c)] PROGRAM`

| Short option | Long option | Argument    | Description         |
|--------------|-------------|-------------|---------------------|
| -O           | --optimize= | 0\|1\|2\|3  | Optimization level  |
| -m           | --memory=   | bytes       | Size of the heap    |
| -i           | --interp    |             | Use the interpreter |
| -c           | --compiler  |             | Use the compiler    |
| -e           | --eof=      | keep\|0\|-1 | EOF mode            |
| -h           | --help      |             | Display help        |

## Build instructions

The `Makefile` can be used for native builds and tests on Linux.  To cross compile
for Windows, use the [ziglang](https://ziglang.org/) toolchain
(see the `build.zig` file).

## IR

The IR consists of a doubly linked list of instructions.  Each instruction has
up to two operands.  Jump instruction jump to their corresponding labels.  Each
label can only jumped to from a single position.  JNZ are always backward jumps
and JZ are always forward jumps.  Each jump instruction contains a pointer to
its corresponding label, and the label contains a pointer to its jump.

Optimizations use an iterator over the instruction stream to inspect it, match
patterns, and manipulate the stream.  Because of the C++ iterator API, care must
be taken if the stream is manipulated while an iterator is alive.  The underlying
structure is a linked list, so removing something from the list might make the
iterator invalid.

## Optimizations

The optimizer has several passes, which can be controlled using the `-O` flag.
`-O0` disables all optimizations.

The optimization level `-O1` enables optimizations, which do not create any 
operations which are not already present in Brainfuck.  Operations might get 
an additional parameter, but no new operations are introduced.  With `-O2` 
some new operations, like setting the current cell to a specific value, are 
introduced and operations get more operands. The highest optimization level
`-O3` adds more loop optimizations.

If something goes wrong, first try to disable optimizations.

## Interpreter

The interpreter is not optimized at all. It simply runs over the instruction 
stream and switches over the opcode of the instruction to perform it.

## Runtime compiler

The runtime compiler uses hardcoded templates for each instruction.  It does not
create any reusable procedures.  There are many assumptions for the type of the
operands of each instruction, which should generally hold for brainfuck programs.

If something goes wrong, first try the interpreter.

## EOF for read operations

The `-e` flag can be used to change the behavior on EOF.  I have seen many 
programs which rely on `-e0`, but I think `-ekeep` is a more sane default, since
the program can simply set the cell to `0` before the read, if desired.  Using a
hardcoded value for EOF might make it harder for certain applications.

## TODOs

Like always, too many.

## License

Copyright (c) 2023-2024 Christoph Göttschkes

Licensed under the [MIT](https://opensource.org/licenses/MIT) License.
See the `LICENSE` file for more info.
