Cameron Bauman 1236693

I used the resource: https://www.geeksforgeeks.org/dynamic-array-in-c/ to learn more about the functionality and implementation of dynmically allocated arrays in C.
I used the resource: https://www.learn-c.org/en/Linked_lists as a review for traversing through linked lists. I also utilized the class slides to help with the headtail implementation

The functionality of all 3 programs works and valgrind test runs successfull on all 3 as well.

I learned more about memory allocation and how to implement it in this assignment. As well as the time comparision between methods of loading and storing data within these programs. The time required for arraydouble depends on the amount Records being processed but is not effecient in terms of managing space. The llheadonly takes much longer than llheadtail because it needs to chase through the entire list to append data rather than having a tail reference already at the end of the list. This clearly reflects the time for each programs execution in various tests.
