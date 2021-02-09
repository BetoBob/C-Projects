# server

```
server.c
Hensley, Robert

Description
	A TCP server that sends files in it's working directory when
	a client sends a request for it.
	
	usage: ./server [port number]

Specifications
	- all client transactions are output on the terminal
	  and stored in a `log` text file
	- only has access to the files within the working directory
		- all dot files are illegal to access
	- critical section (when the server reads and writes to a client) 
	  uses mutex locking to avoid race conditions 
	- sending the server executable is an illegal operation for the client
	
Examples
	hostname -I 	# record ip address for client to use
	./server 4443 	# starts server on this port
    cat log			# view client / server transactions
```

