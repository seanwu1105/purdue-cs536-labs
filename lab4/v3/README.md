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
