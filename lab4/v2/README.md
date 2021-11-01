# Problem 2 [60 pts]

## 2.1 General background

In Problem 1 we have used an insecure way -- sending a secret key in the clear
-- to authenticate a client. Such methods were the norm well into the 1990s
before the adoption of cryptographic primitives in everyday network protocols
such as supplanting telnet with ssh. Authentication is facilitated by basic
cryptographic primitives which can also be used for implementing confidentiality
(i.e., encryption) and integrity (i.e., message has not been modified by an
attacker). The underlying mechanisms are one and the same. Cryptographic
primitives used in network protocols rely on a message encoding function `E`, a
decoding function `D`, and two distinct keys, `e` and `d`, called public and
private keys, respectively. These systems are referred to as asymmetric or
public key cryptographic systems. Given a message `m` (a bit string), called
plaintext, that Alice wishes to send Bob, confidentiality that protects `m` from
eavesdroppers works as follows:

### Confidentiality:

1. Alice computes encrypted message, `s = E(m,e)`, using `m` and Bob's public
   key e. B's public key, as the name indicates, is assumed known to all parties
   who wish to send secret messages to B.
1. Bob receives `s` (e.g., payload of Ethernet frame, UDP or TCP packet), then
   computes `m = D(s,d)` which yields the original unencrypted message `m`.
1. It is assumed that without knowing B's private key `d`, computing `m` from
   `s` is computationally difficult.

`D` can be viewed as an inverse of `E`, and `D` is a "one-way function" in the
sense that encrypting -- going in one direction -- is computationally easy but
decrypting (i.e., inverting `E`) without knowing B's private key (which we
assume he safeguards) is computationally hard. That is, unless P = NP. For
authentication, the same primitives are employed but in reverse. That is, Bob
wishes to send a message (called certificate) `s` to Alice whereby Alice, upon
receiving `s`, can determine that Bob is the originator of `s`.

### Authentication:

1. Bob computes certificate, `s = D(m,d)`, where `m` is a plaintext message that
   says "I am Bob, born around 1905, my favorite color is blue, gettimeofday()
   is ... and I want you to send me the requested file" using his private key
   `d`.
1. Alice, upon receiving `s`, computes `m = E(s,e)`, which yields the original
   plaintext message which follows a strict format (e.g., name, birthday,
   favorite color, time stamp, etc.).
1. It is assumed that only B can generate a certificate `s` using his private
   key `d`, that when encrypted via `E` using B's public key `e` results in a
   meaningful plaintext message that follows a specific format.

Hence the two functions `E` and `D` commute in the sense that either order of
function composition yields the original (plaintext) message as they act as
inverses of each other. The popular RSA public key cryptosystem that relies on
the assumption that factoring large primes is computationally hard yields such
`E` and `D`.

## 2.2 Client authentication: public key infrastructure

A central issue of using a public key cryptosystem for authentication and
confidentiality is bootstrapping, since Alice knowing Bob's public key is easier
said than done. For example, if an impostor C somehow convinces A that a key
that C created, say `e*`, is B's public key, all bets are off. So how do we
bootstrap a distributed system where the true public keys of all relevant
parties are accessible? Unfortunately, there is no solution to the bootstrapping
problem but for brute-force system initialization where designated "trusted"
parties, called certificate authority (CA), serve as arbiters of verifying
public keys. Operating systems are distributed with hardcoded public keys of CAs
so that a user of the operating system can make use of their services. For
example, if a secure communication session over UDP or TCP (e.g., user data sent
as UDP or TCP payload is encrypted before transmission) to a server is desired,
the server's address -- the symbolic domain name of an IP address, say,
www.cs.purdue.edu in place of 128.10.19.120 -- is transmitted to the CA
(encrypted with the CA's public key). The CA then responds with the public key
of the server as a signed certificate (i.e., step (a) of Authentication) which
the client can verify for authenticity by applying `E` with the CA's public key
(step (b) of Authentication). To reduce overhead, servers may obtain pre-signed
CA certificates which are communicated to clients directly.

When engaging in lengthy data exchanges (e.g., file server responding to client
requests), a public key infrastructure (PKI) is only used to establish mutual
authentication and share a secret private key, after which symmetric encryption
using the shared private key is used to achieve data confidentiality. This is
due to symmetric encryption being more efficient. A popular symmetric encryption
method is one-time pad which XORs data bits with a random sequence (obtained
from private key). To date, one-time pad is the only provably secure encryption
method assuming that the random sequence is indeed random and the private key is
known to the communicating parties only. In the real world, pseudo-random
sequences take the place of random sequences, hence one-time pads are only as
secure as the randomness of pseudo-random sequences generated from private keys.
As John von Neumann noted: "Anyone who considers arithmetical methods of
producing random digits is, of course, in a state of sin."

## 2.3 Client authentication: implementation

When building network applications using socket programming, TLS (Transport
Layer Security) and its precursor SSL (Secure Sockets Layer) using the OpenSSL
API is the default way to implement cryptographic protocols. Unlike TCP/UDP/IP
socket programming which is narrow in scope and supported by select system calls
of an operating system, SSL is more complex due to its generality stemming from
the diversity and richness of context dependent security protocols. SSL
programming is the subject of a course in network security and outside the scope
of our course. We will, however, implement a limited form of cryptographic
security by replacing the simple secret-key/password method of authenticating
the client in Problem 1 with a more secure method following the cryptographic
framework of 2.2.

We will assume that our file server maintains an access control list (ACL) of IP
addresses (in dotted decimal form) and associated public keys. A client sends a
request in the form of a certificate signed using function `D` and the client's
private key. The server, upon receiving a request, applies `E` with the client's
public key assuming its IP address is contained in the ACL. If the result
matches a certain format, the requested file is transmitted. Otherwise the
request is dropped. From a network programming perspective, the internals of E
and D are not relevant since they can be treated as black boxes. In place of
emulating RSA (or other cryptographic algorithms), we will define simplified
black boxes E and D as follows.

### Decoding function D:

The decoding function,
`unsigned int bbdecode(unsigned int x, unsigned int prikey)`, takes
`unsigned int x` and private key `prikey` as input and returns an `unsigned int`
value which will be used by the client to authenticate itself to the server.
`bbdecode()` works by performing bit-wise XOR of the 32 bits of `x` and
`prikey`. The value x will be the client IPv4 address viewed as a 32-bit
unsigned integer. The 4-byte unsigned integer returned by `bbdecode()` will be
sent in place of the 2-byte secret-key in the request packet to the server.
Hence the length of the request packet increases by 2 bytes.

### Encoding function E:

The encoding function,
`unsigned int bbencode(unsigned int y, unsigned int pubkey)`, takes
`unsigned int y` and public key `pubkey` as input. `pubkey` is the public key
associated with the IPv4 address of a client that sent a request. The server
looks up the public key `pubkey` associated with the client's IPv4 address. The
first four bytes of the request packet when viewed as `unsigned int` will be
used as `y` to compute `bbencode()` which takes the bit-wise XOR of `y` and
`pubkey`. If the value returned by `bbencode()` is the client's IPv4 address
viewed as an `unsigned int`, the server will consider the client authenticated.
Note that little endian/big endian conversion must be implemented to ensure that
`bbencode()` and `bbdecode()` computation yield correct results. Since XOR is
its own inverse, `E` and `D` will commute and satisfy our needs.

Using the certificate computed by `D`, `y`, is subject to replay attack by an
adversary who may record `y` through traffic sniffing, then reuse it to
authenticate itself as a valid client to download a file. This can be mitigated
by extending the certificate to include sequence numbers and timestamps that do
not remain static. We will omit this part in this exercise. Implement the
modified roadrunner and place the code in `v2/`. Create a file, `acl.dat`, that
contains entries comprised of IP address (in dotted decimal form) and public key
associated with the IP address (to be read in as unsigned integer). Use three
entries of lab machine IPv4 addresses for testing. Continue to use secret-key in
`rrunnerc`'s command-line argument as its private key value. Remove the
secret-key in `rrunners`'s command-line argument since it is not needed. Test
and verify that roadrunner with improved authentication support works correctly.
Check if there is any degradation of throughput due to performing
encryption/decryption
