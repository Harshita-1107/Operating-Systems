import os
import subprocess
import sys

print("FIFO")
subprocess.call(["g++", "FIFO.cpp"]) 
for i in range(2,20,2): 
	subprocess.call(["./a.exe"], stdin = sys.stdin) 

print("LRU")
subprocess.call(["gcc", "lru.c"]) 
for i in range(2,20,2): 
	subprocess.call(["./a.exe"], stdin = sys.stdin) 

print("Optimal")
subprocess.call(["g++", "optimal.cpp"])
for i in range(2,20,2): 
	subprocess.call(["./a.exe",], stdin = sys.stdin)