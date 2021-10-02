# Problem 1 [120 pts]

## TODO

- block invalid params.
- check signal handler resetting.

Implement an application layer ping client/server using datagram sockets
(`SOCK_DGRAM`) that allows a client, `mypingcli`, to check if the ping server,
`mypingsrv`, is running on a remote host, as well as estimate RTT (round-trip
time). A socket is a type of file descriptor (note that Linux/UNIX distinguishes
7 file types of which sockets are one) which is primarily used for sending data
between processes running on different hosts. The hosts may run different
operating systems (e.g., Linux, Windows, macOS) on varied hardware platforms
(e.g., x86 and ARM CPUs). Our lab machines run Linux on x86 PCs. Datagram
sockets facilitate low overhead communication by not supporting reliability.
That is, a message sent from a process running on one host to a process running
on a remote host may not be received by the latter. It is up to the application
to deal with resultant consequences.

## 1.1 General background

A socket can be of several types. In this problem, you will use datagram
(`SOCK_DGRAM`) sockets that invoke the UDP (User Datagram Protocol) protocol
implemented inside operating systems such as Linux. We will discuss the inner
workings of UDP when we study transport protocols. In the same way that FIFOs
were utilized as an abstract communication primitive to achieve inter-process
communication in Problem 3, lab1, without understanding how FIFOs are actually
implemented, we will utilize datagram sockets as an abstract communication
primitive to facilitate communication between processes running on different
hosts. As noted in class, to identify a process running on an end system (e.g.,
PC, server, router, smartphone) under the governance of an operating system we
need to know how to reach the end system -- IP (Internet Protocol) address of
one its network interfaces -- and which process we wish to talk to on the end
system specified by a port number. A port number is a 16-bit non-negative
integer that is bound to the PID (process ID) of a process to serve as its alias
for network communication purposes.

Port numbers 0-1023, called well-known port numbers, cannot be used by app
programs. Port numbers 1024-49151, referred to as registered port numbers, are
usable by app processes. However, it is considered good practice to avoid using
them to the extent feasible. In many instances, networked client/server apps --
the same goes for peer-to-peer apps which are just symmetric client/server apps
where a host acts both as server and client -- are coded so that they do not
depend on specific port numbers to facilitate portability and robustness. We
will do the same when possible. The host on which a destination process runs is
identified by an IP address. Although IPv6 (version 6) with 128-bit addresses is
partially deployed and used, IPv4 (version 4) with 32-bit addresses remains the
dominant protocol of the global Internet today. Unless otherwise noted, we will
use IPv4 addresses. Although we loosely say that an IP address identifies a
host, more accurately, an IP address identifies a network interface on an end
system. Hosts that have multiple network interfaces (e.g., a smartphone has
WiFi, Bluetooth, cellular, among other interfaces) may have multiple IP
addresses, one per network interface. Such hosts are called multi-homed (vs.
single-homed). A network interface need not be configured with an IP address if
there is no need to speak IP.

Most network interfaces have unique 48-bit hardware addresses, also called MAC
(medium access control) addresses, with IP addresses serving as aliases in an
internetwork speaking IP. In socket programming, we use IP addresses, not MAC
addresses to facilitate communication between processes running on different
hosts. IP addresses are translated to MAC addresses by operating systems before
delivery over a wired/wireless link. We will study the inner working of IP when
discussing network layer protocols. To facilitate human readability of IP
addresses, a further layer of abstraction is implemented in the form of domain
names. For example, `www.cs.purdue.edu` is mapped to IPv4 address
`128.10.19.120` where the four decimal numbers specify the four byte values of a
32-bit address in human readable form. This translation operation is carried out
with the help of a distributed database system called DNS (Domain Name System).
Details of DNS, HTTP, SNMP, SSL, and other higher layer protocols are discussed
later in the course.

## 1.2 Implementation details

### User interface

The ping server, `mypingsrv`, invokes system call `socket()` to allocate a file
descriptor (i.e., socket descriptor) of type `SOCK_DGRAM`. A socket descriptor
is but a handle and must be further configured to specify who the communicating
parties are, and possibly other properties. After `socket()`, `bind()` is called
to bind the server to an IP address and an unused port number on its host. Use
the Linux command, `ifconfig -a`, to determine the IPv4 address following the
dotted decimal notation assigned to Ethernet interface `eth0` on our lab
machines (e.g., `128.10.25.213` on `pod3-3.cs.purdue.edu` in LWSN B148) and
provide it as command-line input to your server process along with a port number
to use:

```sh
% mypingsrv 128.10.25.213 55555
```

If the specified port number is already being used, `bind()` will fail. Run
`mypingsrv` again with a different port number.

The client, `mypingcli`, is executed on a different host with command line
arguments that specify the server's coordinate. In addition, the client
specifies an IP address of its network interface to use for network
communication. For example, `amber05.cs.purdue.edu` in HAAS G050 is configured
with Ethernet interface with IPv4 address `128.10.112.135`. Hence running

```sh
% mypingcli 128.10.11.135 128.10.25.213 55555
```

on the client host specifies that the client should use `128.10.11.135` as its
IP address and communicate with a ping server at `128.10.25.213:55555`. Instead
of specifying what port number to use to bind the client process, specify `0` as
the port number which delegates the task of finding an unused port number to the
operating system.

### Operation: client

The client, `mypingcli`, after creating a `SOCK_DGRAM` socket and binding to an
IP address and port number, creates a UDP packet containing a 5 byte payload
which it sends to the server using the `sendto()` system call. The first 4 bytes
contain an integer -- message identifier (MID) or sequence number -- that
identifies the client request. The value of the fifth byte is a control message
that commands the server what to do. If its value is 0 it means that the server
should respond immediately by sending a UDP packet to the client with a 4-byte
payload whose content is the MID value contained in the client request. If its
value is a positive integer _D_ between 1 and 5, it means that the server should
delay sending a response by _D_ seconds. If the value of the fifth byte equals
99, then the server should terminate. Before sending a ping request, the client
calls `gettimeofday()` to record the time stamp just before the packet is sent.
Upon receiving a response from the server, the client calls `gettimeofday()` to
take a second timestamp. Using the MID value in the response packet, the client
calculates RTT by taking the difference of the corresponding send and receive
timestamps which are output to stdout in a unit of milliseconds (msec).

Upon starting up, the client reads from a configuration file, `pingparam.dat`,
four integer values: _N_, _T_, _D_, _S_. _N_ is an integer between 1 and 7 that
specifies how many packets the client should send to the receiver. If _N_ > 1,
then _T_ lying between 1 and 5 specifies how many seconds the client should wait
before sending the next request. _D_ is the 1-byte command to be sent to the
server, and _S_ is the sequence number (i.e., MID value) of the first packet.
The 4-byte MID value of any additional ping packets is incremented by 1. For the
last request packet, the client sets an alarm to expire after 10 seconds using
`alarm()`. If a response does not arrive and the alarm expires, the client
ceases waiting and terminates.

### Operation: server

After binding to an IP address and port number, the server, `mypingsrv`, calls
`recvfrom()` and blocks on client requests. When a request arrives, the server
reads the 5-byte payload and inspects the fifth byte to determine what action is
requested. If the command is 0, the server creates a packet containing the
4-byte MID payload and sends it to the client. That is, the server process
behaves as an iterative server and performs the task itself. After responding to
the client, the server goes back to blocking on `recvfrom()`. If the command is
99 then it terminates by calling `exit(1)`. If the command is invalid, then the
client request is ignored and the server calls `recvfrom()` to wait for the next
request. If the command is an integer value between 1 and 5, then the server
process delegates sending a response to the client to a worker process by
forking a child. The child process uses `sleep()` to delay sending a response to
the client by 1-5 seconds.

Implement your ping app in a modular fashion and provide a `Makefile` in `v1/`
to compile and generate `mypingsrv` and `mypingcli`. Create `README` under `v1/`
that specifies the files and functions of your code and a brief description of
their roles. When sending and receiving 5-byte messages, note that the x86 Linux
PCs in our labs use little-endian byte ordering whereas Ethernet uses big-endian
byte ordering. Test your ping app to verify correctness.

## Getting Started

Build `mypingsrv` and `mypingcli` with the `make` command in the `/v1`
directory.

```sh
make
```

Start the server first to get ready for pinging.

```sh
./mypingsrv <server-ip-address> <server-port-number>
```

After the server is running, start new clients in new terminals, which will
start the pinging process.

```sh
./mypingcli <client-ip-address> <server-ip-address> <server-port-number>
```

You can config the pinging process by setting parameters in `pingparam.dat`. See
implementation details for client operations above for the configuration.

To stop a running pinging server or client, send `SIGINT` with <kbd>ctrl</kbd> +
<kbd>c</kbd> on Linux. Once the server has been stopped, clients would not
receive any pinging feedback.

To clean up for project rebuild on failed, use the following command.

```sh
make clean
```

## Project Structure

### `mypingcli.c`

The source of `mypingcli`. For each pinging, it will send the pinging signal to
the assigned `mypingsrv` with UDP and then wait for the result from `mypingsrv`
with UDP. If no result is received before the timeout, `mypingcli` move forward
to the next pinging and ignore all previous results.

### `mypingsrv.c`

The source of `mypingsrv`. After starting running with the assigned IP address
and port number, `mypingsrv` waits for a UDP package. After executing the
predefined behavior according to the pinging message, `mypingsrv` will send back
the message to `mypingcli`.

### `read_config.c`

Provides functionalities to read and parse configurations in `pingparam.dat`.
The parsed configurations will be stored in a `struct Config` data type
variable. There will be 4 integers, separated by space, in the `pingparam.dat`
file: (_N_ _T_ _D_ _S_).

### `message_codec.c`

Handles the message encoding and decoding. The message in the pinging process
can be created by a 4-bytes message ID (MID) and a 1-byte delay parameter. This
source file provides the functionalities to convert a 5-bytes message to a MID
and delay parameter and vice versa.
