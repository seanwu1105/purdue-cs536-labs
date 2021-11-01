# Problem 2 [120 pts]

Modify Problem 2, lab2, so that the remote command server becomes a file server.
The client, `myftpc`, is executed with command-line arguments

```sh
myftpc server-IP server-port filename secret-key blocksize
```

that specify server coordinates, name of the file to fetch, a secret key for
authentication, and a blocksize (in unit of bytes) that is used to write the
content of the transferred file. We will restrict file names to be less than or
equal to 8 ASCII characters which must be lower- or upper-case alphabet. The
secret key is an integer in the interval [0, 65535]. The client's request
consists of a 2 byte unsigned integer which encodes the secret key, and a
filename whose length is between 1-8 bytes. The client rejects command-line
input that does not meet the length specifications. The client takes a timestamp
before transmitting its request to the server, and a second timestamp after the
last byte of the file has been received and written to disk. The client outputs
to stdout the size of the file (in unit of bytes), completion time (second
timestamp minus first timestamp), and throughput (file size divided by
completion time).

The server, `myftps`, is executed with command-line arguments

```sh
myftps server-IP server-port secret-key blocksize
```

which specify the IP address and port number it should bind to, a secret key
used to authenticate client requests, and a blocksize (in unit of bytes) that is
used to read the content of the requested file. The server should not read more
than 10 bytes from the client request to prevent buffer overflow. If the secret
key communicated as the first two bytes of a request does not match the secret
key provided in its command-line arguments, the request is ignored. The same
goes if the filename fails to meet the naming convention. These checks are
performed in addition to source IP address filtering carried out in Problem 2,
lab2. The server will continue to toss a coin and ignore a request if it comes
up heads. If a client request passes the preceding checks, the server process
verifies that the requested file exists in the current working directory. If
not, the request is ignored by closing the connection. Otherwise, a child is
forked that reads the file content using `read()` in unit of blocksize bytes and
calls `write()` to transmit the bytes to the client. The child closes the
connection after the last byte has been transmitted.

Note that overall file transfer client/server performance is influenced by the
particulars of both network I/O and disk I/O. In general, for file servers
without specialized kernel support a rule of thumb is to perform disk I/O to
read/write files in unit of a file systems block size. Try blocksize values 512,
1024, 2048, 4096 for small (tens of KB) and large (tens of MB) files to evaluate
performance. Larger blocksize values reduce the number system calls which
improves disk I/O performance. In the case of write operations to network
sockets, considering the payload size of the underlying LAN technologies to
prevent fragmentation (we will discuss it under IP internetworking) can improve
performance. In this problem, set the count argument of `write()` and `read()`
to sockets to blocksize. When testing your file server app, make sure to verify
that the requested file has been transferred correctly, for example, by using
file checksums. Compare file transfer performance when server and client are
located on two machines in the same lab (e.g., LWSN B148) or across two
different labs (LWSN B148 and HAAS G050). Discuss your findings in `lab3.pdf`.
Implement your file server app in a modular fashion and provide a Makefile in
`v1/`. Include `README` under `v1/` that specifies the files and functions of
your code, and a brief description of their roles. Henceforth lack of
modularity, annotation, and clarity of code will incur 5% point penalty. Make
sure to remove large files created for testing after tests are completed.

## Getting Started

Build `myftps` and `myftpc` with the `make` command in the `/v1` directory.

```sh
make
```

Start the server first to get ready for accepting remote commands.

```sh
./myftps <server-ip> <server-port> <secret-key> <blocksize-byte>
```

After the server is running, start the new client in another machine.

```sh
./myftpc <server-ip> <server-port> <filename> <secret-key> <blocksize-byte>
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

## Security Protection

The server will only accept the client from `128.10.25.*` or `128.10.112.*`. If
you want to test the server with clients in a different IP address, we can
manually modify the `allowed_ips` array in `myftps.c`.

Though the server has the simple security protection mentioned above. The code
is not audited or passes any real-world security test. Please use the `myftps`
only for the experiment inside a secure environment (e.g. docker).

## Project Structure

### `myftpc.c`

The source of `myftpc`. For security reasons, there are several limitation
mentioned above on the client-side.

If the input arguments are malformed (see spec in instruction mentioned above),
`myftpc` will reject the command. Also, if the connection is lost after
connected, the client will terminate immediately.

### `myftps.c`

The source of `myftps`. It will ignore the remote command randomly to simulate
the behavior of `myftpc` on package lost or timeout. The following conditions
will disconnect the TCP connection immediately:

- Secret key mismatch.
- The parameters in request are malformed (see spec in instruction mentioned
  above).
- Client address is not in white list (see the allow list in instruction
  mentioned above).
- Requested file does not exist.
- Randomly (50% chance) ignore the request.

### `arg_checkers.c`

Contains two checking functions to check if the arguments (filename and secret
key) follows the spec.

### `request_codec.c`

Provides functionality to encodes request message from filename and secret and
vice versa. Note that the total length of request is fixed to 10 bytes.

### `socket_utils.c`

Provides shared functionality to build, create, bind and connect socket-related
data structure used by both client and server.

## Analysis

### Same Lab vs Different Labs

The following is the statistics of file transmission between 2 machines in same
lab.

Environment:

- Blocksize: 1024 bytes
- Same lab
  - Server on: `amber05.cs.purdue.edu` (`128.10.12.135`)
  - Client on: `amber06.cs.purdue.edu` (`128.10.112.136`)
- Different labs
  - Server on: `amber05.cs.purdue.edu` (`128.10.12.135`)
  - Client on: `pod3-3.cs.purdue.edu` (`128.10.25.213`)

| file size (bytes) | same-lab completion time (ms) | same-lab throughput (bytes/ms) | different-labs completion time (ms) | different-labs throughput (bytes/ms) |
| ----------------- | ----------------------------- | ------------------------------ | ----------------------------------- | ------------------------------------ |
| 1K                | 2.195                         | 466.515                        | 4.235                               | 241.795                              |
| 2K                | 4.519                         | 453.198                        | 4.927                               | 415.669                              |
| 4K                | 3.983                         | 1028.371                       | 7.366                               | 556.068                              |
| 8K                | 6.53                          | 1254.518                       | 11.95                               | 685.523                              |
| 16K               | 11.836                        | 1384.251                       | 20.284                              | 807.73                               |
| 32K               | 20.75                         | 1579.181                       | 36.328                              | 902.004                              |
| 64K               | 40.93                         | 1601.173                       | 72.201                              | 907.688                              |
| 128K              | 75.714                        | 1731.146                       | 131.6                               | 995.988                              |
| 256K              | 149.943                       | 1748.291                       | 258.006                             | 1016.038                             |
| 512K              | 291.836                       | 1760.324                       | 564.554                             | 928.676                              |
| 1M                | 591.683                       | 1772.192                       | 961.443                             | 1090.627                             |
| 2M                | 1164.455                      | 1800.973                       | 1489.877                            | 1407.601                             |
| 4M                | 2182.411                      | 1921.867                       | 3794.393                            | 1105.395                             |
| 8M                | 4378.048                      | 1916.061                       | 7578.114                            | 1106.952                             |
| 16M               | 9550.879                      | 1756.615                       | 14313.703                           | 1172.109                             |
| 32M               | 17888.158                     | 1875.79                        | 26028.311                           | 1289.151                             |
| 64M               | 38215.547                     | 1756.062                       | 50110.191                           | 1339.226                             |
| 128M              | 73504.717                     | 1852.974                       | 106147.926                          | 1264.44                              |

The following is the visualization of the "same-lab" statistics. Note that the
completion time chart is in log scale.

![same-lab-completion-time](https://i.imgur.com/4xFJych.png)

![same-lab-throughput](https://i.imgur.com/eBb6tI2.png)

The following is the visualization of the "different-lab" statistics. Note that
the completion time chart is in log scale.

![diff-lab-completion-time](https://i.imgur.com/Dz4Cvmp.png)

![diff-lab-throughput](https://i.imgur.com/cMEkOpU.png)

From the statistics, we can see that the overall completion time of the
"same-lab" is smaller than the one of "different-lab". This is expected as the
physical distance is longer for 2 machines in different labs than in the same
lab. The throughput is also higher in the "same-lab" case as less router or
other core infrastructures is needed to transfer the data between 2 machines in
same lab in comparison to 2 machines in 2 different labs. The same reason causes
the fluctuation of throughput in "different-labs" case as more infrastructures
involved, which causes higher nondeterministic overhead. Finally, the throughput
for small files is relatively low as the overtime dominate the completion time
for these cases.

There is another big factor for the completion time and throughput: network file
system. All lab machines use network file system to access the user data. Thus,
if the file system is busy or the connection is heavily loaded, the throughput
in the statistics above will decreases significantly.

### Blocksize

The following is the statistics of file transmission in different blocksizes and
file sizes.

Environment:

- Server on: `amber05.cs.purdue.edu` (`128.10.112.135`)
- Client on: `amber06.cs.purdue.edu` (`128.10.112.136`)

| blocksize (bytes) | 64K bytes file throughput (bytes/ms) | 64M bytes file throughput (bytes/ms) |
| ----------------- | ------------------------------------ | ------------------------------------ |
| 512               | 567.063                              | 699.483                              |
| 1024              | 1101.390                             | 1403.790                             |
| 2048              | 1897.724                             | 2617.399                             |
| 4096              | 2763.600                             | 4200.077                             |

The following is the visualization of above data.

![64k-blocksize-throughput](https://i.imgur.com/ggIKJsb.png)
![64m-blocksize-throughput](https://i.imgur.com/BE9Etar.png)

We can see that the throughput is heavily affected by the blocksize. The
blocksize decides how many times a series of system calls need to be invoked.
Thus, high blocksize can greatly reduce the overhead of socket I/O and disk I/O.

The load of network file system is still a big factor for above statistics. Any
network congestion would heavily impact the throughput.
