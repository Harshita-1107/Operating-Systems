In this lab, we implemented various different page replacement algorithms:
1. First in First Out (FIFO)
2. Least Recently Used (LRU)
3. Optimal Page Replacement (OPT)

For the First in First Out Algorithm,
the operating system keeps track of all pages in the memory in a queue, 
the oldest page is at the front of the queue. 
When a page needs to be replaced page in the front of the queue is selected for removal. 

For Least Recently Used Algorithm,
In this algorithm, the page will be replaced which is least recently used. 


For Optimal Page Replacement Algorithm,
In this algorithm, pages are replaced which would not be used for the longest duration of time in the future. 

How to run:
    1. Open the directory where the code is located.
    2. Compile and run the execution of the C++ file using the commands -
             a) FIFO:
                  Command lines: g++ FIFO.cpp
                                 ./a.exe

             b) LRU:
                  Command lines: g++ LRU.cpp
                                 ./a.exe

             c) Optimal:
                  Command lines: g++ optimal.cpp
                                 ./a.exe

There exists a Python script that runs the simulation b times where b is the input taken from the user.

How to run the simulation: python script.py

- Belady's Anomaly:
When the frame size increases from 8 to 10 from the FIFO algorithm, the page defaults also increase. Hence this causes Belady's Anomaly. Below is the output shown for the same.

FIFO

Enter length of reference string: 
20

Enter reference string: 
7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1

Enter no. of frames: 
8

Page Replacement Process:      
7  -1  -1  -1  -1  -1  -1  -1  
7  0  -1  -1  -1  -1  -1  -1   
7  0  1  -1  -1  -1  -1  -1    
7  0  1  2  -1  -1  -1  -1  
7  0  1  2  0  -1  -1  -1
7  0  1  2  0  3  -1  -1
7  0  1  2  0  3  0  -1
7  0  1  2  0  3  0  4
7  0  1  2  0  3  0  4  8
7  0  1  2  0  3  0  4  8
7  0  1  2  0  3  0  4  8
7  0  1  2  0  3  0  4  8
7  0  1  2  0  3  0  4  8  
7  0  1  2  0  3  0  4  8
7  0  1  2  0  3  0  4  8
7  0  1  2  0  3  0  4  8
7  0  1  2  0  3  0  4  8
7  0  1  2  0  3  0  4  8
7  0  1  2  0  3  0  4  8
7  0  1  2  0  3  0  4  8

Number of Page Faults using FIFO: 8

Enter length of reference string: 
20

Enter reference string:
7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1

Enterno. of frames:
4

Page Replacement Process:
7  -1  -1  -1
7  0  -1  -1
7  0  1  -1
7  0  1  2
7  0  1  2  4
3  0  1  2  5
3  0  1  2  5
3  4  1  2  6
3  4  1  2  6
3  4  1  2  6
3  4  0  2  7  
3  4  0  2  7
3  4  0  2  7
3  4  0  1  8
2  4  0  1  9
2  4  0  1  9
2  4  0  1  9
2  7  0  1  10  
2  7  0  1  10
2  7  0  1  10

Number of Page Faults using FIFO: 10


For the rest two algorithms - LRU and optimal, there is no Belady's Anomaly.
