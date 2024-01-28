# bf-cc

A brainfuck interpreter and run-time compiler written in C++ for Linux/x86-64.

The input program is translated into a minimalistic IR, optimized, and then
either interpreted, or compiled to x86-64 machine code and the executed.  The
generated machine code uses the
[Linux syscall API](https://github.com/torvalds/linux/blob/v4.17/arch/x86/entry/syscalls/syscall_64.tbl)
to perform the required I/O operations and is therefore not portable in any
way.  The optimizer is capable of performing some very basic optimizations and
could be enhanced further.

The runtime compiler uses simple, hard-coded instruction templates for each
OpCode of the IR.

## Command line interface

Usage: `bf-cc` [`-h`] [`-O(0|1|2|3)`] [`-mMEMORY_SIZE`] [`-e(keep|0|1)`] [`(-i|-c)`] PROGRAM

| Short option | Long option | Argument   | Description         |
|--------------|-------------|------------|---------------------|
| -O           | --optimize= | 0\|1\|2\|3 | Optimization level  |
| -m           | --memory=   | bytes      | Size of the heap    |
| -i           | --interp    |            | Use the interpreter |
| -c           | --compiler  |            | Use the compiler    |
| -e           | --eof=      | keep|0|-1  | EOF mode            |
| -h           | --help      |            | Display help        |

## License

Copyright (c) 2024 Christoph Göttschkes

Licensed under the [MIT](https://opensource.org/licenses/MIT) License.
See the `LICENSE` file for more info.
