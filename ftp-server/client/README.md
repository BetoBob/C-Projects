# client

```
client.c
Hensley, Robert

Description
	A TCP client that requests files from a server and 
	outputs the files contents if successfully received.
	
	usage: ./client [ip]:[port] [optional: filename / command]

Specifications
	- use the command `index` to see the working directory
      of the server
    - only outputs files that exist on the server and that
      can be successfully opened
	
Examples
	$ ./client 129.65.128.82:4443 testfile
    This is a testfile.
    $ ./client 129.65.128.82:4443 log
    ...entire contents of log file should appear here...
    $ ./client 129.65.128.82:4443 doesnotexist
    $ ./client 129.65.128.82:4443 index
    server
    .dotfile
    emptyfile
    testfile
    bigfile
    log
    bad?filename
    notreadablefile
```

