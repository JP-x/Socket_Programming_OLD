#

#* Fall 2013

#*

#* First Name: Jonathan

#* Last Name: Padilla

#* Username:  jpadi004

#* email address: jpadi004@ucr.edu


all:  mtClient mtServer

mtClient: mtClient.c
	g++ -ggdb mtClient.c -lpthread -o mtClient

mtServer: mtServer.c
	g++ -ggdb mtServer.c -o mtServer


copy: mtClient
	cp -f mtClient ~/cs100/hw/hw9/testdir 
	cp -f mtClient ~/cs100/hw/hw9/testdir2 
	cp -f mtClient ~/cs100/hw/hw9/testdir3

clean:
	rm -rf *.out *.o mtClient mtServer
	rm -rf ~/cs100/hw/hw9/testdir/Thread*/*
	rmdir ~/cs100/hw/hw9/testdir/Thread*

clean2:
	rm -rf ~/cs100/hw/hw9/testdir/Thread*/*
	rmdir  ~/cs100/hw/hw9/testdir/hw2/
	rm -rf ~/cs100/hw/hw9/testdir2/hw2/*
	rmdir  ~/cs100/hw/hw9/testdir2/hw2/
	rm -rf ~/cs100/hw/hw9/testdir3/hw2/*
	rmdir  ~/cs100//hw/hw9/testdir3/hw2/
