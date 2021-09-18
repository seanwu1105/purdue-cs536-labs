# Problem 3 [70 pts]

This is an extension of Problem 2 where instead of showing the output of the
command executed by the server on `stdout` of its associated window, the output
is sent to the client who submitted the request. The client then outputs the
response from the server on its `stdout` in its associated window. To facilitate
communication from server back to client, a client sets up its own FIFO before
sending a request to the server. The name of the FIFO is `cfifo<clientpid>`
where `<clientpid>` is the `pid` of the client. For example, if a client has
`pid` `22225` then its FIFO is named `client22225`. Since the server needs to
know the client's pid to open the client's FIFO and write the response, we will
change the format of the request message so that a client's message starts with
its pid followed by newline which is then followed the command and ends with
newline. For example, if the client with `pid` `22225` is requesting that
`ls -l -a` be executed by the server, the message would be the five characters
of string `22225` followed by `\n` followed by the eight characters of string
`ls -l -a` followed by `\n`.

In the above example, when a child process of the server process calls
`execvp()` to run the legacy app `/bin/ls` with command-line arguments `-l` and
`-a`, the output of `/bin/ls` which has been coded to output to stdout must be
redirected to the FIFO of the client process that sent the request. Use the
`dup2()` system call make file descriptor 1 (i.e., `stdout`) point to the
client's FIFO before calling `execvp()`. This ensures that when the legacy app
`ls` outputs to `stdout` the characters are written to the client's FIFO. After
submitting a request, a client blocks on its FIFO by calling `read()` to await
the server's response. After reading the server's response, the client writes it
to stdout which will output to the window associated with the client process.

Create subdirectory `v3/`, port and modify the code of Problem 2 so that the
server's response is sent to a client's FIFO. Update Makefile and README to
reflect the changes. Test and verify that the modified client/server app
implementing bidirectional communication works with multiple clients.

## Getting Started

Build `commandserver.bin` and `commandclient.bin` with the `make` command in
`/v3` directory.

```sh
make
```

Start the server first to create a FIFO `serverfifo.dat` automatically.

```sh
./commandserver.bin
```

After the server is running, create new clients in new terminals, which will
create a client FIFO (`cfifo<client_pid>`) automatically according to the `pid`
of client process.

```sh
./commandclient.bin
```

Now, you can enter commands in the new terminals running the `commandclient.bin`
executable. For example, type `ls -la` in one of the client terminal. You should
see the execution result of the corresponding command showing on the client
terminal.

To stop the server or clients, send `SIGINT` with <kbd>ctrl</kbd> + <kbd>c</kbd>
on Linux. Once the server has been stopped, clients cannot send commands. Doing
so will terminate the clients immediately.

To clean up for project rebuild on failed, use the following command with
`make`.

```sh
make clean
```

## Atomicity of `write()`

To avoid the potential for interleaving of characters belonging to two or more
requests from clients writing into FIFO, we limit the length of command in
client to `PIPE_BUF`.

## File as Client Input

You can use a file storing commands separated by `\n` as client input. For
example:

```sh
./commandclient.bin < ../test/commands.txt
```

## Blocking Server Behavior

Note that the server will be blocked when there is a task has not competed. You
can test this behavior with the `./test/long.py` file simulating a long-running
task.

For example,

```sh
# in client terminal A
python ../test/long.py # block server for 5 seconds
```

```sh
# in client terminal B
ps
ls
```

The server will execute the `ps` and `ls` commands after the
`python ./test/long.py` command has finished.

## Project Structure

### `simsh1.c`

The source of `commandserver.bin`. The reader of the FIFO (`serverfifo.dat`)
consumes commands from multiple writers of the FIFO. The server will write the
result of the commands back to the client FIFO (`cfifo<client_pid>`).
`commandserver.bin` can only have one instance at a time in one directory.

### `commandclient.c`

The source of `commandclient.bin`. The writer of the FIFO (`serverfifo.dat`)
feeds commands expected to be read by `commandserver.bin`. It would
automatically create the client FIFO (`cfifo<client_pid>`) and wait for the
command results from the server. Can have multiple instance at a time in one
directory.

### `fifo_info.h`

Store the sharing information about the FIFO.

### `parse_command.c`

Parse the user input command from string (`char*`) into argv (`char * const *`)
for `execvp`. Able to handle double quotes and escaped characters.
