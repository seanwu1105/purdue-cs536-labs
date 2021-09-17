# Problem 1 [40 pts]

The directory

`/homes/park/pub/cs536/lab1` (symbolic link currently pointing to
/u/riker/u3/park/pub/cs536/lab1)

accessible from our lab machines in LWSN B148 and HAAS G050 contains `simsh.c`
which implements a prototypical concurrent server: a simple, minimalist shell. A
concurrent server receives and parses a client request -- in this case, from
`stdin` (by default keyboard) -- then delegates the actual execution of a
requested task to a worker process or thread. Many apps in the real-world,
including network software, follow the general structure of concurrent
client/server code which are multithreaded programs. They constitute an
important baseline for implementing network software.

Create a directory, `lab1/`, somewhere under your home directory, and a
subdirectory `v1/` therein. Copy `simsh.c` to `v1/`, compile, run and check that
you understand how it works. Then modify `simsh.c`, call it `simsh1.c`, that
accepts command-line arguments. That is, whereas `simsh.c` can handle `ls` and
`ps`, it cannot handle `ls -l -a` and `ps -gaux`. Your modified code,
`simsh1.c`, handles command-line arguments by parsing `ls -l -a` into string
tokens `ls`, `-l`, `-a` and passing them to `execvp()` as its second argument
which is an array of pointers to the parsed tokens. Make your code modular by
performing the parsing task in a separate function that is placed in its own
file. Provide a `Makefile` in `v1/` that compiles your code and generates a
binary executable `simsh1.bin`. Compile, test, and verify that your code works
correctly. Annotate your code so that a competent programmer can understand what
you are aiming to do.

## Getting Started

Build `simsh1.bin` with the `make` command in `/v1` directory.

```shell
make
```

Start the server.

```shell
./simsh1.bin
```

Now, you can enter commands in the terminal running the `simsh1.bin` executable.
For example, type `ls -la` in one of the client terminal. You should see the
execution result of the corresponding command showing on the terminal.

To stop the server, send `SIGINT` with <kbd>ctrl</kbd> + <kbd>c</kbd> on Linux.

To clean up for project rebuild, use the following command with `make`.

```shell
make clean
```

## Project Structure

### `simsh1.c`

The source of `simsh1.bin`.

### `parse_command.c`

Parse the user input command from string (`char*`) into argv (`char * const *`)
for `execvp`. Able to handle double quotes and escaped characters.
