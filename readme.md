# EE450 Project - Socket Programming

## Full Name
Jingcheng Wen  

## Student ID
9721996420  

## Introduction (What I have Done in the Assignment)
This socket programming assignment is to implement a RSVP application for client/patient to schedule appointment from 3 hospitals. The scheduler will choose the best option based on shortest distance and availability of hospitals. I implemented the dijkstra algorithm to find the shortest path, and used socket system calls for host communications. This application is written in C/C++.  

## File Structure

### `network.hpp`, `network.cpp`
Utility functions for network  
Basically just wrap the socket system calls, and mostly reused coded from Beej's Guide to Network Programming.

### `hospital.hpp`, `hospital.cpp` 
* `Hospital` class  
Using object-oriented design to abstract hospital. The class stores the capacity, location, occupancy and map information, and provide functions to find the shortest path and calculate score for a given location. 
* Other common functions for hospitalA/B/C

### `hospitalA.cpp`, `hospitalB.cpp`, `hospitalC.cpp`
Main function for Hospital A/B/C  
Bootup -> Send initial capacity and occupancy to scheduler -> Loop in routine:  
* Wait to receive location information of client from scheduler
* Find the shortest path and calculate the score
* Send the result to scheduler  

### `scheduler.cpp`
Main function for Scheduler  
Bootup -> Wait for initial information from each Hospital -> Loop in routine:  
* Wait to receive location information from client
* Send location information of client to all available Hospitals
* Wait to receive distance and score infomation from all available Hospitals
* Make assignment decision according rules
* Send result back to Client
* Send result to the assignd hospital if decides to assign

### `client.cpp`
Main function for Client  
* Send location information to Scheduler
* Wait for assignment result

## Message Format
### Phase 1 Boot-up
* hospital -> scheduler: the initial hospital info: [\<capacity> \<occupancy>]  

### Phase 2 Forward
* client -> scheduler: the location info of client: [\<location\>]  
* scheduler -> hospital: the location info of client: [\<location>\]  

### Phase 3 Scoring
* hospital -> scheduler: the calculated shortest distance and score: [\<distance> \<score>]  
If the location of client is not on the map, then distance is "-1".  
If the location is the same as the hospital, then distance is "0".

### Phase 4 Reply
* scheduler -> client: the assignment decision: [\<decision\>]  
0: hospitalA, 1: hospitalB, 2: hospitalC  
-1: location not in map  
-2: location the same as hospital  
-3: all hospitals are already fully occupied  
* scheduler -> hospital [\<decision\>]
1: assigned, 0: not assigned

## Idiosyncrasy
Notice my implementation assume each of the message can be delivered error-free. It depends on the exact order of each message.  
Make sure it is executed following the order: Scheduler -> Hospital A -> Hospital B -> Hospital C -> Client

## Code Reuse
Only in `network.cpp`, most of the code reused from Beej's Guide to Network Programming. Other codes are writted by me with reference below.

## Reference

### Socket Programming
* Code in `network.cpp` heavily referred to [Beej's Guide to Network Programming](https://www.beej.us/guide/bgnet/html/)
* [Socket Programming in C/C++ - GeeksforGeeks](https://www.geeksforgeeks.org/socket-programming-cc/?ref=leftbar-rightbar)
* Do not forget to initialize `addrlen` in `accept()` and `fromlen` in `recvfrom()`: [linux - recvfrom returns invalid argument when *from* is passed - Stack Overflow](https://stackoverflow.com/questions/2999639/recvfrom-returns-invalid-argument-when-from-is-passed)

### Shortest Path (Dijkstra)
* [Traversing a map (or unordered_map) in C++ STL - GeeksforGeeks](https://www.geeksforgeeks.org/traversing-a-map-or-unordered_map-in-cpp-stl/)
* [C++ Priority Queue With Comparator - neutrofoton](https://neutrofoton.github.io/blog/2016/12/29/c-plus-plus-priority-queue-with-comparator/)
* [Priority queue of pairs in C++ with ordering by first and second element - GeeksforGeeks](https://www.geeksforgeeks.org/priority-queue-of-pairs-in-c-with-ordering-by-first-and-second-element/)

### Debug
* Using gdb: See CS402 discussion1 
* Using vscode: See CS402 warmup1 FAQ [VScodeForC.pdf (usc.edu)](http://merlot.usc.edu/cs402-s21/VScodeForC.pdf)
* Find zombie process and kill: [Mac/Linux and Golang : Fix bind: address already in use error (socketloop.com)](https://www.socketloop.com/tutorials/mac-linux-and-golang-fix-bind-address-already-in-use-error)

### Miscellaneous
* `stringstream` in c++: [c++ - How do you clear a stringstream variable? - Stack Overflow](https://stackoverflow.com/questions/20731/how-do-you-clear-a-stringstream-variable)
* [c++ - How can I get the max (or min) value in a vector? - Stack Overflow](https://stackoverflow.com/questions/9874802/how-can-i-get-the-max-or-min-value-in-a-vector)