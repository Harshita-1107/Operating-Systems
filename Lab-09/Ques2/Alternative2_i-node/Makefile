a.out: userprog.cpp inodehead.h inodeexe.a
	g++ -g userprog.cpp -L. -lsecfile

inodeexe.a: secfile.o
	ar -rcs inodeexe.a secfile.o

secfile.o: inodedefinations.cpp inodehead.h
	g++ -c inodedefinations.cpp

clean:
	rm secfile.o libalt1.a a.out
