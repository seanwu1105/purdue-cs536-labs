# Bonus problem [15 pts]

One of the performance factors ignored in Problem 2 is the influence of network
file systems which carry out network I/O combined with caching to enable access
to mounted file systems that may be remotely located. A method that reduces the
effect is to run client and server processes so that they access files on local
file systems of our lab machines, in particular, `/tmp`. For a large file and
the blocksize that yields the best performance in Problem 2, carry out
experiments across two lab machines and determine if there is a performance
difference. Discuss your finding in `lab3.pdf`. Remove files in `/tmp` after
tests are completed.

## Statistics

Environment:

- Blocksize: 1024 bytes
- Server on: `amber05.cs.purdue.edu` (`128.10.112.135`)
- Client on: `amber06.cs.purdue.edu` (`128.10.112.136`)

| file size (bytes) | completion time (ms) | throughput (bytes/ms) |
| ----------------- | -------------------- | --------------------- |
| 1K                | 1.495                | 684.95                |
| 2K                | 1.273                | 1608.798              |
| 4K                | 1.322                | 3098.336              |
| 8K                | 1.472                | 5565.217              |
| 16K               | 1.642                | 9978.076              |
| 32K               | 1.991                | 16458.061             |
| 64K               | 2.052                | 31937.622             |
| 128K              | 4.607                | 28450.619             |
| 256K              | 6.023                | 43523.825             |
| 512K              | 11.265               | 46541.323             |
| 1M                | 16.544               | 63381.044             |
| 2M                | 26.724               | 78474.48              |
| 4M                | 44.166               | 94966.807             |
| 8M                | 75.736               | 110761.17             |
| 16M               | 145.237              | 115516.129            |
| 32M               | 294.389              | 113979.911            |
| 64M               | 581.909              | 115325.358            |
| 128M              | 1164.106             | 115296.827            |

## Visualization

![same-lab-completion-time-local](https://i.imgur.com/bWhkzJb.png)

![same-lab-throughput-local](https://i.imgur.com/FLw9OjZ.png)

## Analysis

When the file size reach to 8M bytes, the throughput has approximately reached
the upper bound. The upper bound of the throughput increases 63 times compared
to the statistics when using network file system. Thus, the completion time
decreases significantly. For example, the completion time of a 128M bytes file
decreases from 73504.717 ms to 1164.106 ms, which is about 63 times faster.
Namely, performance of local file system is about 63 faster than the one of
network file system.
