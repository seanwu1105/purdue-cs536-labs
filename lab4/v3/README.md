# Bonus problem [20 pts]

Encrypt the file content sent to the client by applying `E` at the server
byte-by-byte using the client's public key. The client decrypts the received
content by applying `D` with its private key byte-byte-byte. Since `E` and `D`
are defined to operate on unsigned integers, promote each byte of the requested
file from char to unsigned int before computing `E` and `D`. Use the least
significant byte of the computed unsigned int as the output of `E` and `D` for
encryption/decryption purposes. Place your code in `v3/`. Test and verify that
the modified roadrunner app of Problem 2 works correctly. Check if there is
degradation of throughput from performing encryption/decryption operations to
facilitate confidentiality.

## Getting Started

Build `rrunners` and `rrunnerc` with the `make` command in the `/v1` directory.

```sh
make
```

Before start the server, we need to make sure the client is in the whitelist.
Edit `acl.dat` in the following format.

```txt
<client-ip> <public-key>
<client-ip> <public-key>
...
```

Since we use XOR for simple encryption and decryption, the public key in the
list is the same as the private key used by the client. The following is the
sample `acl.dat` file.

```txt
192.168.1.6 123
192.168.1.11 688
192.168.1.5 77
```

Start the server first to get ready for accepting remote commands.

```sh
./rrunners <server-ip> <server-port> <blocksize> <windowsize> <timeout>
```

After the server is running, start the new client in another machine.

```sh
./rrunnerc <server-ip> <server-port> <filename> <private-key> <blocksize>
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

The structure and purpose of other files can be found in `v1/README.md`.

### `access_control.c`

Provide functionality to load the access control list (ACL) from `acl.dat` and
check if the given client's IP address has the access permission to request
file. Note that the `acl.dat` is loaded immediately into the memory after the
server starts.

### `bbcodec.c`

Provide toy encryption and decryption functions with XOR operator.

## Analysis

Still, the completion time of `v1` (without XOR authentication) and `v3` (with
XOR authentication) are almost identical since we only XOR operation against
file data and request command. The impact on performance is significantly
smaller than other factors, such as network status.

| window size | completion time (s) |
| ----------- | ------------------- |
| 1           | 3.935               |
| 2           | 2.606               |
| 4           | 1.783               |
