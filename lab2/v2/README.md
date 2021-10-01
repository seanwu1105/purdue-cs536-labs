# Problem 2 [120 pts]

Modify Problem 3 of lab1 so that server and client run on different machines --
e.g., one on a machine in LWSN B148 and the other on a Linux PC in HAAS G050 --
and use stream sockets `SOCK_STREAM` to communicate in place of FIFOs.

## 2.1 General background

Stream sockets uses TCP (Transmission Control Protocol) which implements sliding
window to achieve reliable data communication. Sockets of type `SOCK_STREAM`
export a byte stream abstraction to app programmers where a sequence (or stream)
of bytes sent by a sender using `write()` is received as a sequence of bytes in
the same order and without "holes" by the receiver when calling `read()`. Thus
the operating system shields the app from having to deal with the consequences
of unreliable communication networks which applies to most real-world networks
today. In contrast to `SOCK_DGRAM` sockets implementing UDP where payload
carried by one packet is not part of a stream and data transport is unreliable,
`SOCK_STREAM` sockets maintain a notion of persistent state referred to as a
connection between sender and receiver which is needed to implement sliding
window. Other aspects of TCP such as congestion control that require a
connection between sender and receiver will be covered under transport
protocols.

`SOCK_STREAM` is inherently more overhead prone than `SOCK_DGRAM` which is
reflected in how a connection between sender and receiver is set up before
communication can commence using `write()` and `read()` system calls.
`SOCK_STREAM` sockets are well-suited for implementing concurrent client/server
apps including file and web servers. A server calls `socket()` to allocate a
`SOCK_STREAM` socket descriptor followed by `bind()` analogous to `SOCK_DGRAM`
in Problem 1. After `bind()`, the server calls `listen()` to mark the socket
descriptor as passive, meaning that a server waits on connection requests from
clients. The second argument of `listen()` specifies how many connection
requests are allowed pending. For our purposes, 5 will do. Following `listen()`,
the server calls `accept()` which blocks until a client connection request
arrives. When a client request arrives, `accept()` returns a new socket
descriptor that can be used to communicate with the client while the old socket
descriptor (the first argument of `socket()`) is left intact so that it can be
re-used to accept further connection requests. The new socket descriptor
returned by `accept()` is called a full association which is a 5-tuple

```txt
(SOCK_STREAM, server IP address, server port number, client IP address, client
port number)
```

that specifies the protocol type (`SOCK_STREAM` or TCP), server and client
coordinates. The original socket descriptor is called a half association since
the client IP and port number remain unspecified so that the descriptor can be
re-used to establish a new connection upon calling `accept()`.

On the client side, instead of calling `bind()` a client calls `connect()` with
the server's IP address and port number. The operating system will fill in the
client's IP address and port number with an an unused ephemeral number. If the
client is multi-homed and wants to use a specific network interface to
send/receive data, or wants to use a specific port number, then `bind()` can be
used to do so. By default, `bind()` is not needed on the client side due to the
actions of `connect()`. For typical clients, `connect()` is used which obviates
the need to call `bind()`. When `connect()` returns, a client can send its
request using `write()` analogous to Problem 3, lab1.

## 2.2 Implementation details

On the server side, a difference from Problem 3, lab1, is that client requests
do not share a common FIFO queue but are transmitted through separate full
association `SOCK_STREAM` sockets. That is, there exists a pairwise connection
between a specific client and the shared server. An additional functional
feature to add to the server is that upon receiving a client request, it tosses
a coin, and if it comes up heads, decides to ignore the request. That is,
without forking a child to delegate the task of executing the requested command,
the server goes back to blocking on `accept()` to await the next client request.
If the coin toss comes up tails, the server will first inspect the last three
bytes of the client's IPv4 address to check that it matches either `128.10.25.*`
or `128.10.112.*` which are addresses of our lab machines in LWSN B148 and HAAS
G050. That is, the fourth byte's decimal value should be 128, the third byte 10,
and the second byte either 25 or 112. If the match fails, the client request is
ignored. Problem 2 implements a remote command server which presents a security
vulnerability that must be guarded against. The above check restricts client
requests to originate from our lab machines. However, an attacker may forge its
source IP address, called spoofing. For additional protection, your server will
only allow the legacy apps `date` or `/bin/date` without command-line arguments
to be requested for execution. This is prevents a classmate who is unhappy about
having lost a tennis match from sending `rm` with suitable arguments to your
server. The fact that the source IP address is spoofed is immaterial since
damage will have been done. Make sure not call `execvp()` until you have
verified that the protection measures are functioning correctly.

On the client side, since the server is known to discard requests at the
application layer despite the data transport protocol implementing sliding
window in the operating system, a client sets a timer using `alarm()` before
transmitting a request. Set its value to 2 seconds. If a response arrives before
the 2 second timer expires, the `SIGALRM` signal is cancelled. Otherwise, a
signal handler registered for `SIGALRM` is asynchronously executed. The signal
handler closes the connection, establishes a new connection to the server, and
sets a fresh 2 second timer before retransmitting the request. This is repeated
at most 3 times. If the third attempt fails, the client gives up and outputs a
suitable message to stdout.

Implement your client/server app in a modular fashion and provide a Makefile in
`v2/` to compile and generate `rcommandserver.bin` and `rcommandclient.bin`.
Execute the client with command-line arguments

```
% rcommandclient.bin server-IP server-port
```

that specify the server's IP address and port number. Create README under `v2/`
that specifies the files and functions of your code, and a brief description of
their roles. Test your client/server app with multiple client processes and
verify correctness. Even with the protection measures in place, except when
testing do not keep the server running.

## Getting Started

Build `rcommandclient.bin` and `rcommandserver.bin` with the `make` command in
the `/v2` directory.

```sh
make
```

Start the server first to get ready for accepting remote commands.

```sh
./rcommandserver.bin <server-ip-address> <server-port-number>
```

After the server is running, start new clients in other terminals.

```sh
./rcommandclient.bin <server-ip-address> <server-port-number>
```

To stop a running pinging server or client, send `SIGINT` with <kbd>ctrl</kbd> +
<kbd>c</kbd> on Linux.

## Security Protection

Due to security reason, the clients can only send `date` and `/bin/date`
commands. All other commands will be ignored by the server.

Besides, the server will only accept the client from `128.10.25.*` or
`128.10.112.*`. If you want to test the server with clients in different IP
address, we can manually modify the `allowed_ips` array in `rcommandserver.c`.

Though the server has a simple security protection mentioned above. The code is
not audited or passes any real-world security test. Please use the
`rcommandserver` only for experiment inside a secure environment (e.g. docker).

## Project Structure

### `rcommandclient.c`

The source of `rcommandclient`. For security reasons, there are several
limitation mentioned above on the client side.

If a command is timeout, the client will attempt again up to 3 times. You can
modify the maximum attempt with `MAX_ATTEMPTS` definition in `rcommandclient.c`.

We do not handle any message longer than maximum read size because we only
accept `date` and `/bin/date` commands which are relatively short in command per
se and their return values.

### `rcommandserver.c`

The source of `rcommandserver`. It will ignore the remote command randomly to
simulate the behavior of `rcommandclient` on package lost or timeout.

### `parse_addrinfo_arg.c`

Parse the address information, including IP address and port number, from argv
and create a data with `struct addrinfo` type.
