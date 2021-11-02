# Problem 1 [180 pts]

## 1.1 Basic operation

We will implement a custom reliable file transfer protocol, `rrunner`
(roadrunner), aimed at improving performance compared to TCP based and `tftp`
file transfer apps under low loss network conditions. `tftp` is popular in LAN
environments for transporting system files. It uses simple stop-and-wait to
achieve reliability. Our objective is to increase throughput over `tftp` by
employing a form of sliding window to transmit multiple packets at a time,
however, without incurring the overhead in complexity and ACK traffic of
full-fledged sliding window protocols. The key element of `rrunner` is a fixed
window size _W_ in unit of UDP packets. The sender, i.e., file server
`rrunners`, transmits _W_ data packets -- unless end of file is reached -- where
each packet is marked by its sequence number _0_, _1_, ..., _W-1_. The actual
sequence number space is twice as large and discussed in Section 1.3. The
receiver, i.e., client `rrunnerc`, sends a UDP ACK packet only when all _W_
packets have been received. The server then sends the next _W_ data packets.
Before transmitting the data packets, the server sets a timer. If it expires
before an ACK is received, all _W_ data packets are retransmitted.

## 1.2 App interface

We will adopt the app interface specification, system parameters, and file I/O
component of the TCP based file transfer application `myftpc`/`myftps` of
Problem 2, lab3. The client is executed with command-line arguments

```sh
rrunnerc server-IP server-port filename secret-key blocksize windowsize
```

where `windowsize` specifies _W_. The other arguments are the same as for
`myftpc`. The file server is executed with command-line arguments

```sh
rrunners server-IP server-port secret-key blocksize windowsize timeout
```

where the `windowsize` must be the same value as in the client. Parameter
`blocksize` determines the unit of `read()`/`write()` when performing file I/O.
Since roadrunner uses UDP, `blocksize` specifies the number of bytes
sent/received by `sendto()`/`recvfrom()` system calls. The last argument,
timeout, specifies in unit of microseconds how long the server will wait before
_W_ data packets are retransmitted. Use the `setitimer()` system call of type
`ITIMER_REAL` to set a high resolution timer to timeout in a one-shot (i.e., not
periodic) fashion. Use `ping` or `myping` to estimate RTT based on which a
suitable timeout value is set. We do not want timeout to be too close to RTT
since statistical variability may trigger premature timeout causing unnecessary
retransmission. On the other hand, we do not want timeout to exceed RTT
significantly since it leads to wasted time and resultant decrease of
throughput. System parameters are typically specified in configuration files or
as program constants. We will continue to use command-line arguments for clarity
and configurability during testing and performance evaluation. We will adopt the
same filename and secret-key convention as Problem 2, lab3. However, we will
omit the server throwing a coin to determine if a request should be ignored.
That is, all received file transfer requests are honored. The client will still
need to set a timer (500 msec) to retransmit a request in case UDP fails to
deliver the client's request.

## 1.3 Implementation details

We will limit windowsize, i.e., _W_, to _0_-_63_. With blocksize 1024 (bytes)
and RTT 2 msec in an extended LAN environment, the maximum throughput achievable
by roadrunner is at least 1024 \* 64 \* 8 \* 500 = 262 Mbps. This ignores 28
bytes contributed by UDP and IPv4 headers, and at least 18 bytes contributed by
Ethernet from source/destination address and CRC fields. In the opposite
direction, the numbers ignore operating system, file I/O, and other software
overhead. During testing we must keep windowsize well below 64 since with RTT
smaller than 2 msec the file transfer app is easily capable to exceeding 1 Gbps.
Since our lab machines are a shared resource and not a dedicated testbed, we
will test _W_ values to compare against the performance of myftpc/myftps, not
the maximum throughput achievable by roadrunner.

The first byte of every data packet transmitted by `sendto()` will specify the
packet's sequence number _0_, ..., _W-1_. By the relationship between sequence
number space and maximum window size in sliding window protocols, we know that
sequence number space must be at least twice as large as maximum window size to
correctly handle retransmitted packets. Hence the actual sequence number space
is _0_, ..., _2\*W - 1_. Hence, sequence number _W_ represents the first data
packet of the next window of _W_ packets. For example, in the case of three
consecutive windows without retransmission, the sequence numbers of the 3\*W
data packets will be: _0_, ..., _W-1_; _W_, ..., _2\*W-1_; _0_, ..., _W-1_. When
the client receives all packets of a window, it sends a UDP ACK packet
containing a 1-byte payload with value _W-1_ or _2\*W-1_ depending on the
sequence number of the last packet received. Upon receiving an ACK, if the timer
has not expired then the timer is cancelled and the next _W_ data packets are
transmitted after setting a fresh timeout. If the timer has expired, then any
on-going retransmission is ceased and the newly set timer prior to
retransmission is cancelled. Due to the 1-byte sequence number, a data packet
transmitted by `sendto()` is _blocksize + 1_ bytes long. We will limit blocksize
to less than or equal _1471_ bytes so that the total payload of an Ethernet
frame carrying IPv4 and UDP packets does not exceed 1500 bytes. When comparing
roadrunner performance against that of `myftpc`/`myftps`, use _blocksize+1_ as
the latter's blocksize value.

The server, `rrunners`, needs to convey to its client that end of file has been
reached. This is done in one of two ways. First, if the size of the file being
transferred is not a multiple of blocksize bytes, the last data packet having
payload size less than _blocksize + 1_ implies that it is the last data packet.
In the case where file size is a multiple of blocksize, the server sends a
packet of size _blocksize + 2_ bytes where the last byte serves as a dummy
padding byte. If the _recvfrom()_ at the client returns _blocksize + 2_ bytes,
it strips off the last byte and knows that file transmission has ended. The
client then transmits _8_ duplicate ACK packets in the hope that one of them
will reach the server so that it can terminate. This is obviously a hack whose
theoretical roots we discussed in class. The server, upon (hopefully) receiving
an ACK of the last data packet, terminates. In general, the server `rrunners`
should follow a concurrent server structure where a child is forked to handle
the transfer of data. For this problem, you may use an iterative server
structure where a single client is handled.

## 1.4 Testing and performance evaluation

For `roadrunner` blocksize values _512_, _1024_, _1471_ bytes, test that
`windowsize = 1` (i.e., plain stop-and-wait similar to `tftp`) works correctly.
Compare throughput and completion time of large and small files benchmarked in
Problem 2, lab2, between `roadrunner` and `myftpc`/`myftps`. For blocksize
_1024_, increase windowsize gradually to verify correctness of `roadrunner` and
evaluate performance compared to `W = 1`. Stop increasing windowsize if
completion time exceeds 4 seconds. As noted in class, for large files typical
completion times should be in the 2-3 second range. Discuss your results in
`lab4.pdf`. Implement `roadrunner` in a modular fashion and provide a Makefile
in `v1/`. Include `README` under `v1/` that specifies the files and functions of
your code, and a brief description of their roles. Remove large files created
for testing after completion.

## Getting Started

Build `rrunners` and `rrunnerc` with the `make` command in the `/v1` directory.

```sh
make
```

Start the server first to get ready for accepting remote commands.

```sh
./rrunners <server-ip> <server-port> <secret-key> <blocksize> <windowsize> <timeout>
```

After the server is running, start the new client in another machine.

```sh
./rrunnerc <server-ip> <server-port> <filename> <secret-key> <blocksize>
```

To stop a running ftp server or client, send `SIGINT` with <kbd>ctrl</kbd> +
<kbd>c</kbd> on Linux.

## Create Dummy Test Files

Use `fallocate` to create test files.

```sh
fallocate -l 10M ./test
```

If the `fallocate` is not supported on the file system, use `truncate` instead.

```sh
truncate -s 10M ./test
```

## Project Structure

### `rrunnerc.c`

The source of `rrunnerc`.

If the input arguments are malformed (see spec in instruction mentioned above),
`rrunnerc` will reject the command.

### `rrunners.c`

The source of `rrunners`. The following conditions will ignore the incoming file
request:

- Secret key mismatch.
- The parameters in request are malformed (see spec in instruction mentioned
  above).
- Requested file does not exist.

### `arg_checkers.c`

Contains two checking functions to check if the arguments (filename and secret
key) follows the spec.

### `request_codec.c`

Provides functionality to encodes request message from filename and secret and
vice versa. Note that the total length of request is fixed to 10 bytes.

### `socket_utils.c`

Provides shared functionality to build, create, bind and connect socket-related
data structure used by both client and server.

### `packet_codec.c`

Provides functionality to encode and decode packet from partitioned file data
(block data) and sequence number, and vice versa.

## Analysis

### `rrunner` vs `myftp`

We use the following commands for `rrunner` benchmark. We run the server on
`amber05.cs.purdue.edu` (`128.10.112.135`).

```sh
./rrunners 128.10.112.135 22222 123 <block-size> <window-size> 1000000
```

And we run the client on: `amber06.cs.purdue.edu` (`128.10.112.136`)

```sh
./rrunnerc 128.10.112.135 22222 testf 123 <block-size> <window-size>
```

We set the window size to 1 and 32 for comparison.

#### `myftp`

| blocksize | 64K bytes file throughput (bytes/ms) | 64K bytes file completion time (ms) | 64M bytes file throughput (bytes/ms) | 64M bytes file completion time (ms) |
| --------- | ------------------------------------ | ----------------------------------- | ------------------------------------ | ----------------------------------- |
| 512       | 480.2333165                          | 136.467                             | 715.7628418                          | 93758.519                           |
| 1024      | 798.6546102                          | 82.058                              | 1254.796654                          | 53481.864                           |
| 1471      | 1064.017015                          | 61.593                              | 1586.591925                          | 42297.495                           |

#### `rrunner`, `W=1`

| blocksize | 64K bytes file throughput (bytes/ms) | 64K bytes file completion time (ms) | 64M bytes file throughput (bytes/ms) | 64M bytes file completion time (ms) |
| --------- | ------------------------------------ | ----------------------------------- | ------------------------------------ | ----------------------------------- |
| 512       | 377.4571634                          | 173.625                             | 383.7349636                          | 174883.371                          |
| 1024      | 642.7870846                          | 101.956                             | 760.1632465                          | 88282.174                           |
| 1471      | 780.0697511                          | 84.013                              | 1148.341427                          | 58439.818                           |

#### `rrunner`, `W=32`

| blocksize | 64K bytes file throughput (bytes/ms) | 64K bytes file completion time (ms) | 64M bytes file throughput (bytes/ms) | 64M bytes file completion time (ms) |
| --------- | ------------------------------------ | ----------------------------------- | ------------------------------------ | ----------------------------------- |
| 512       | 2394.883976                          | 27.365                              | 124.2452503                          | 540132.229                          |
| 1024      | 5283.882932                          | 12.403                              | 223.6014495                          | 300127.142                          |
| 1471      | 5891.405969                          | 11.124                              | 278.8880449                          | 240630.121                          |

The network status is unstable and thus `rrunner` has worse performance than
`myftp` as multiple packets are dropped during the transmission. However, for
small file size (64K bytes), since the number of dropped packets has
significantly smaller, the performance is better than `myftp`.

There is another big factor for the completion time and throughput: network file
system. All lab machines use network file system to access the user data. Thus,
if the file system is busy or the connection is heavily loaded, the throughput
in the statistics above will decreases significantly.

### Performance with Different Window Sizes

We use the following commands for `rrunner` benchmark. We use the same machines
mentioned above.

For the server, we run:

```sh
./rrunners 128.10.112.135 22222 123 1024 <window-size> 1000000
```

For the client, we run:

```sh
./rrunnerc 128.10.112.135 22222 testf 123 1024 <window-size>
```

We transfer a 10 M bytes file with 1, 2, 4, 8, 12, 16, 20 window sizes.

| window size | throughput (bytes/s) | completion time (s) |
| ----------- | -------------------- | ------------------- |
| 1           | 273.9432852          | 3.738               |
| 2           | 421.7462932          | 2.428               |
| 4           | 595.0029053          | 1.721               |
| 8           | 912.6559715          | 1.122               |
| 12          | 292.0707359          | 3.506               |
| 16          | 222.6570994          | 4.599               |
| 20          | 166.1528476          | 6.163               |

As the window size increases, the penalty increases on UDP packets dropped.
Thus, the best window size is approximately 8 and the throughput could reach
912.655 bytes/s.

The load of network file system is still a big factor for above statistics. Any
network congestion would heavily impact the throughput and completion time.
