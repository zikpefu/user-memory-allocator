Zachary Ikpefua
# Project 3: Memory Allocator 

KNOWN PROBLEMS:
There is a problem that is causing my code to seg fault when I try to deference my freelist, this means somewhere within my code
I am not properly referencing the address or adding the pointers correctly. 
```
FUNCTIONAL TEST 5:  -- random barrage of mallocs, reallocs, and frees.
LD_PRELOAD=./libmyalloc.so ./func5
{'exception': False, 'code': 139, 'stderr': 'Segmentation fault\n', 'stdout': '', 'elapsed': 0.004734992980957031, 'call': 'LD_PRELOAD=./libmyalloc.so ./func5', 'timeout': False}
        ERROR: test crashed. (Segmentation fault)
        ERROR: test failed.
```
DESIGN:
allocator - user created malloc, calloc, realloc, and free functions. More information for each function is in the c file.


# Goals
This program recieved a 70/90 for a functional score and I would like to improve on this. After evaulating the test above, the 
efficency needs to be addressed to total out the 90/90 score
