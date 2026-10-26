# CSCE 412 Load Balancer

This is an implementation of Project 4 for CSCE 421, a simulation of a load balancer with.

## Description

This project implements the load balancer as specified in Project 4 in CSCE 421. A simulation runs in the master load balancer which initializes its child load balancers, specified based on job type. Each load balancer holds web servers and a request queue, and as the simulation runs, the master load balancer routes requests to the load balancers which distributes them onto available servers and scales their servers based on their request queue sizes.

## Getting Started

### Dependencies

g++ - compilation

### Installing

Clone the repository.

### Executing program

```
make
./load-balancer
```

## Authors

Jason Lev

## Version History

* 1.0.0 - revision for class

## Acknowledgments

Robert Lightfoot - Professor

Amran Haroon - TA