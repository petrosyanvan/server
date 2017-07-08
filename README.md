# TCP Server

The server is opening a connection and listens for incoming requests, once a client is connected, the server forks a bash process which is using socket file descriptor as it's own input/output/error streams

## Getting Started

Run "make" in Debug directory to compile code. You must give a port address as an argument when running program. 
Example:
```
./server 8000
```

## Authors

* **Van Petrosyan** 
