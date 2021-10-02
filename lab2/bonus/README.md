# Bonus problem [20 pts]

Compare the RTT estimates from your application layer ping in Problem 1 against
those of `/bin/ping`. Perform tests between machines in the same lab (e.g.,
between pod machines in LWSN B148), and across labs (pod machine in LWSN B148
and amber machine in HAAS G050). What results do you find? Give your
interpretation of the findings in `lab2.pdf` and place it under `lab2/`.

The Bonus Problem is completely optional. It serves to provide additional
exercises to understand the material. Bonus problems help more readily reach the
45% contributed by lab component to the course grade.

## Data Collection

We have 4 conditions that need to be tested:

1. using `myping` in two computers in the same lab.
1. using `/bin/ping` in two computers in the same lab.
1. using `myping` in two computers in the different labs.
1. using `/bin/ping` in two computers in the different labs.

For each condition, we collect the following data regarding the estimated RTT in
ms.

| same lab, `myping` | same lab, `/bin/ping` | across lab, `myping` | across lab, `/bin/ping` |
| ------------------ | --------------------- | -------------------- | ----------------------- |
| 0.46               | 0.178                 | 0.553                | 0.304                   |
| 0.483              | 0.189                 | 0.553                | 0.378                   |
| 0.404              | 0.171                 | 0.636                | 0.385                   |
| 0.371              | 0.174                 | 0.524                | 0.402                   |
| 0.347              | 0.167                 | 0.64                 | 0.389                   |
| 0.314              | 0.168                 | 0.494                | 0.39                    |
| 0.331              | 0.189                 | 0.519                | 0.649                   |
| 0.308              | 0.171                 | 0.49                 | 0.384                   |
| 0.299              | 0.173                 | 0.485                | 0.656                   |
| 0.242              | 0.167                 | 0.512                | 0.383                   |
| 0.453              | 0.173                 | 0.704                | 0.408                   |
| 0.409              | 0.167                 | 0.641                | 0.387                   |
| 0.421              | 0.167                 | 0.528                | 0.656                   |
| 0.326              | 0.177                 | 0.519                | 0.391                   |
| 0.316              | 0.162                 | 0.552                | 0.399                   |
| 0.37               | 0.176                 | 0.488                | 0.383                   |
| 0.303              | 0.166                 | 0.497                | 0.651                   |
| 0.279              | 0.168                 | 0.487                | 0.638                   |
| 0.286              | 0.173                 | 0.478                | 0.317                   |
| 0.286              | 0.168                 | 0.519                | 0.381                   |

## Data Visualization

We visualize the collected data with a scatter chart.

![scatter chart](https://i.imgur.com/OobJDjZ.png)

## Interpretation and Analysis

We can see the averages RTT (in ms) for each case are:

| same lab, `myping` | same lab, `/bin/ping` | across lab, `myping` | across lab, `/bin/ping` |
| ------------------ | --------------------- | -------------------- | ----------------------- |
| 0.3504             | 0.1722                | 0.54095              | 0.44655                 |

From the observation and comparison, we see that in the same environment, the
RTT of `/bin/ping` is smaller than `myping`. We believe this is because
`/bin/ping` is a system call that directly sends ICMP `ECHO_REQUEST` packet. The
ICMP is in the Internet Layer. On the other hand, our handcrafted `myping`
program, which is in the application layer, and thus the overhead is higher than
`/bin/ping`.

Apart from that, the RTT difference ratio between `myping` and `/bin/ping` is
smaller when the two computers are in different labs.

- (same lab, `myping`)/(same lab, `/bin/ping`) = 2.035
- (across lab, `myping`)/(across lab, `/bin/ping`) = 1.211

We think that is because the overhead between `myping` and `/bin/ping` is fixed.
However, the RTT increases proportionally with the physical distance between two
devices. Hence, we can see the overhead impacts lower with larger spatial
distance.
