# Bonus Problem (40 pts)

Add a new control law, method E, selected by value 2 of the method parameter in
the client's command-line arguments. The aim is improve upon the performance of
method D. Describe your control method and its rationale in `lab5.pdf`. Try to
be creative which starts with clearly stating the motivation. Even though an
idea may seem reasonable, whether it actually pans out is another matter.
Implement the method and evaluate its performance by comparing to the results of
Problem 1. Submit the extended code in `v2/`. If Problem 1 is tackled as a group
effort, the bonus problem must be solved as a group effort as well.

## Definition of Method E

To improving the method D, we decide to implement the derivative term of the PID
controller. The formula is derived as follows.

<!-- $$
\lambda(t)\ =\ K_pe(t)\ +\ K_i\int_{0}^{\tau}\ e(\tau)\ d\tau\ +\ K_d\dfrac{d}{dt}e(t)
$$ -->

<div align="center"><img style="background: white;" src="https://render.githubusercontent.com/render/math?math=%5Clambda(t)%5C%20%3D%5C%20K_pe(t)%5C%20%2B%5C%20K_i%5Cint_%7B0%7D%5E%7B%5Ctau%7D%5C%20e(%5Ctau)%5C%20d%5Ctau%5C%20%2B%5C%20K_d%5Cdfrac%7Bd%7D%7Bdt%7De(t)"></div>

We can differentiate both sides of the equation.

<!-- $$
\dfrac{d}{dt}\lambda(t)\ =\ -K_p(\lambda(t) - \gamma)\ +\ K_i(Q^* - Q(t))\ -\ K_d\dfrac{d}{dt}\lambda(t)
$$ -->

<div align="center"><img style="background: white;" src="https://render.githubusercontent.com/render/math?math=%5Cdfrac%7Bd%7D%7Bdt%7D%5Clambda(t)%5C%20%3D%5C%20-K_p(%5Clambda(t)%20-%20%5Cgamma)%5C%20%2B%5C%20K_i(Q%5E*%20-%20Q(t))%5C%20-%5C%20K_d%5Cdfrac%7Bd%7D%7Bdt%7D%5Clambda(t)"></div>

Assuming _Kd_ is always larger or equal to zero, we can transpose the derivative
term and finally get the _lambda(t + 1)_.

<!-- $$
\lambda(t + 1)\ =\ \lambda(t)\ +\ \dfrac{-K_p(\lambda(t) - \gamma)\ +\ K_i(Q^* - Q(t))}{(1\ +\ K_d)}
$$ -->

<div align="center"><img style="background: white;" src="https://render.githubusercontent.com/render/math?math=%5Clambda(t%20%2B%201)%5C%20%3D%5C%20%5Clambda(t)%5C%20%2B%5C%20%5Cdfrac%7B-K_p(%5Clambda(t)%20-%20%5Cgamma)%5C%20%2B%5C%20K_i(Q%5E*%20-%20Q(t))%7D%7B(1%5C%20%2B%5C%20K_d)%7D"></div>

## Analysis

After apply the "D" term of the PID formula, the plots indicate that the
transmission is more stable compared to the plots of method D. Though there are
several unstable events possibility caused by the network status at the time,
the overall congestion control performance is better than other methods.
