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
