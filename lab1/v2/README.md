# Problem 2 [100 pts]

Create a subdirectory v2/ under lab1. Modify your code, simsh1.c, so that
instead of accepting client requests from stdin (i.e., human typing on keyboard
in v1/) the request is communicated from a client process. The server process
running commandserver.bin creates a FIFO (i.e., named pipe) to be used as a
communication channel where clients can submit requests. After creating a FIFO,
its name is saved in a text file v2/serverfifo.dat that a client can read to
find the name of the FIFO. A client running commandclient.bin starts after the
server process has started executing and created serverfifo.dat. If the client
cannot access serverfifo.dat, it terminates after printing a suitable error
message to stderr. When testing, run the server and client processes in separate
windows for ease of observation.

After finding the name of the server's FIFO, the client uses open() to open the
FIFO and sends its request using write(). The client's request is provided by a
human user through stdin. That is, the client is itself a server who prints a
prompt '> ' to stdout and accepts requests from stdin. The request is then
submitted to the server process via its FIFO. To support multiple client
processes, we will use a message format where a request ends with the newline
character. Hence we are using '\n' as a separator or delimiter. When using FIFO
to receive multiple client requests, the potential for interleaving of
characters belonging to two or more requests must be considered. Check up to how
many bytes in a write() system call Linux FIFOs guarantee interleaving will not
occur -- i.e., write() behaves atomically -- and implement your client so that
it rejects user requests that exceed the limit.

Implement your client/server app in a modular fashion and Provide a Makefile in
v2/ to compile and generate commandserver.bin and commandclient.bin. Create
README under v2/ that specifies the files and functions of your code, and a
brief description of their roles. Test your client/server app with multiple
client processes and verify correctness.
