# Problem 2 [100 pts]

Create a subdirectory `v2/` under lab1. Modify your code, `simsh1.c`, so that
instead of accepting client requests from stdin (i.e., human typing on keyboard
in `v1/`) the request is communicated from a client process. The server process
running commandserver.bin creates a FIFO (i.e., named pipe) to be used as a
communication channel where clients can submit requests. After creating a FIFO,
its name is saved in a text file `v2/serverfifo.dat` that a client can read to
find the name of the FIFO. A client running `commandclient.bin` starts after the
server process has started executing and created `serverfifo.dat`. If the client
cannot access `serverfifo.dat`, it terminates after printing a suitable error
message to `stderr`. When testing, run the server and client processes in
separate windows for ease of observation.

After finding the name of the server's FIFO, the client uses `open()` to open
the FIFO and sends its request using `write()`. The client's request is provided
by a human user through stdin. That is, the client is itself a server who prints
a prompt `> ` to `stdout` and accepts requests from `stdin`. The request is then
submitted to the server process via its FIFO. To support multiple client
processes, we will use a message format where a request ends with the newline
character. Hence we are using `\n` as a separator or delimiter. When using FIFO
to receive multiple client requests, the potential for interleaving of
characters belonging to two or more requests must be considered. Check up to how
many bytes in a `write()` system call Linux FIFOs guarantee interleaving will
not occur -- i.e., `write()` behaves atomically -- and implement your client so
that it rejects user requests that exceed the limit.

Implement your client/server app in a modular fashion and Provide a `Makefile`
in `v2/` to compile and generate `commandserver.bin` and `commandclient.bin`.
Create README under `v2/` that specifies the files and functions of your code,
and a brief description of their roles. Test your client/server app with
multiple client processes and verify correctness.

## Getting Started

Build `commandserver.bin` and `commandclient.bin` with the `make` command in
`/v2` directory.

```shell
make
```

Start the server first to create FIFO `serverfifo.dat` automatically.

```shell
./commandserver.bin
```

After the server is running, create new clients in new terminals.

```shell
./commandclient.bin
```

Now, you can enter commands in the new terminals running the `commandclient.bin`
executable. For example, type `ls -la` in one of the client terminal. You should
see the execution result of the corresponding command showing on the server
terminal.

To stop the server or clients, send `SIGINT` with <kbd>ctrl</kbd> + <kbd>c</kbd>
on Linux. Once the server has been stopped, clients cannot send commands. Doing
so will terminate the clients immediately.

To clean up for project rebuild, use the following command with `make`.

```shell
make clean
```

## Atomicity of `write()`

To avoid the potential for interleaving of characters belonging to two or more
requests from clients writing into FIFO, we limit the length of command in
client to `PIPE_BUF`.

## File as Client Input

You can use a file storing commands separated by `\n` as client input. For
example:

```shell
./commandclient.bin < ../test/commands.txt
```

## Blocking Server Behavior

Note that the server will be blocked when there is a task has not competed. You
can test this behavior with the `./test/long.py` file simulating a long-running
task.

For example,

```shell
# in client terminal A
python ../test/long.py # block server for 5 seconds
```

```shell
# in client terminal B
ps
ls
```

The server will execute the `ps` and `ls` commands after the
`python ./test/long.py` command has finished.

## Project Structure

### `simsh1.c`

The source of `commandserver.bin`. The reader of the FIFO (`serverfifo.dat`)
consumes commands from multiple writers of the FIFO. `commandserver.bin` can
only have one instance at a time in one directory.

### `commandclient.c`

The source of `commandclient.bin`. The writer of the FIFO (`serverfifo.dat`)
feeds commands expected to be read by `commandserver.bin`. Can have multiple
instance at a time in one directory.

### `fifo_info.h`

Store the sharing information about the FIFO.

### `parse_command.c`

Parse the user input command from string (`char*`) into argv (`char * const *`)
for `execvp`. Able to handle double quotes and escaped characters.
