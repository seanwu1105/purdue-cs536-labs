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

Use `fallocate` to create test files.

```sh
fallocate -l 10M ./test
```

If the `fallocate` is not supported, use `truncate` instead.

```sh
truncate -s 10M ./test
```
