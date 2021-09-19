# Bonus problem [20 pts]

## Instruction

Use the ping application from one of our lab machines to measure the time it
takes for a packet to reach the destination and receive a response, called the
round-trip time (RTT). Do so for CS's web server `www.cs.purdue.edu` and IUPUI's
web server `www.iupui.edu` in Indianapolis. Select five additional destinations:
midwest, east coast, west coast, across the Atlantic, and across the Pacific.
Some web servers of universities may be configured to ignore ping messages but
it is not difficult to find many that will respond. For the 7 destinations, use
a map to roughly estimate their physical distance from Purdue. Use SOL to
calculate lower bounds of RTT for these destinations. Compare the lower bounds
against the estimates obtained through ping. For each of the destinations, give
your thoughts on what may be the main factors that result in the discrepancy.
For example, if your distance estimate from Purdue is based on straight line
distance, then the fact that communication lines follow indirect routes through
major communication hubs might be one such factor. Whether this is a major
factor you may determine by overestimating the physical distance from Purdue and
checking if SOL latency is significantly impacted. Submit your answer in a pdf
file, `lab1.pdf`, under `lab1/`.

The Bonus Problem is completely optional. It serves to provide additional
exercises to understand the material. Bonus problems help more readily reach the
45% contributed by lab component to the course grade.

## Answer

### Selected Destinations

| Destination        | URL                  | IP Address     | Estimated Physical Distance (km) | RTT Lower Bound (ms) | Estimated RTT (ms) |
| ------------------ | -------------------- | -------------- | -------------------------------- | -------------------- | ------------------ |
| Purdue             | www.cs.purdue.edu    | 128.10.19.120  | 1                                | 0.007                | 0.397              |
| IUPUI              | www.iupui.edu        | 129.79.123.148 | 140                              | 0.933                | 5.906              |
| Midwest            | bulletin.case.edu    | 12.2.169.179   | 409                              | 2.727                | 13.511             |
| East Coast         | cs.nyu.edu           | 216.165.22.203 | 1090                             | 7.267                | 25.779             |
| West Coast         | www.cs.ucla.edu      | 164.67.100.182 | 2861                             | 19.073               | 50.741             |
| Cross the Atlantic | www.manchester.ac.uk | 130.88.101.57  | 6203                             | 41.353               | 100.777            |
| Cross the Pacific  | upd.edu.ph           | 202.92.128.226 | 13267                            | 88.447               | 240.461            |

![IP geolocations map](https://i.imgur.com/3wMTVDJ.png)

### Discrepancy Explanation

| Destination        | [Estimated RTT / RTT Lower Bound] Ratio |
| ------------------ | --------------------------------------- |
| Purdue             | 56.71428571                             |
| IUPUI              | 6.330117899                             |
| Midwest            | 4.954528786                             |
| East Coast         | 3.547406082                             |
| West Coast         | 2.660357574                             |
| Cross the Atlantic | 2.436993688                             |
| Cross the Pacific  | 2.718701595                             |

From the ratio table, most of the discrepancy between the estimated RTT with
ping command and the lower bound of RTT will reduce if the physical distance
between two places increases. It is possible that when the physical distance is
relatively short, the RTT is mainly subject to the overhead of the devices I/O
and routing. Conversely, if the physical distance is relatively long, the speed
of transmission would affect the RTT more.

Overall, the lower bound of the RTT is always much smaller than the estimated
RTT. That is because, usually, the route between the source and destination
would not be a straight line. Also, the route will not be directly connected
generally. More routing means more overhead, possible queueing and package
losses, which greatly increases the RTT.

For the connection oversea, another factor of the impact on the RTT could be the
limited resource of international internet connectivity and the undersea cables.

To sum up, the estimated RTT will get closer to the lower bound if the length of
the route is longer. The factor includes the indirect routing, queueing and
device overhead.
