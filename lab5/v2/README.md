# Bonus Problem (40 pts)

Add a new control law, method E, selected by value 2 of the method parameter in
the client's command-line arguments. The aim is improve upon the performance of
method D. Describe your control method and its rationale in `lab5.pdf`. Try to
be creative which starts with clearly stating the motivation. Even though an
idea may seem reasonable, whether it actually pans out is another matter.
Implement the method and evaluate its performance by comparing to the results of
Problem 1. Submit the extended code in `v2/`. If Problem 1 is tackled as a group
effort, the bonus problem must be solved as a group effort as well.

## Project Structure

Because of the motivation to improving the performance of method D, we decide to
implement the derivative term of the PID controller. The formula is derived as 
follows.

As the professor mentioned in class, the PID algorithm is
![](https://i.imgur.com/mO9svS2.png)
![](https://i.imgur.com/2KsEFvk.png)

We can differentiate both sides of the equation
![](https://i.imgur.com/xm4iYtv.png)

Assuming Kd is always larger or equal to zero, we can transpose the derivative term
and finally get the lambda t + 1
![](https://i.imgur.com/YyoroF4.png)
![](https://i.imgur.com/NDx69Rb.png)
![](https://i.imgur.com/3e0tTWE.png)

