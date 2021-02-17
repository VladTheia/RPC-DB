# RPC+DB

An RPC-based client-server architecture, that stores sensor data in a separate 
database for each user. The sensor data has the following format:

```cpp
struct sensor_data{
	int data_id;
	int no_values; // number of values
	float *values; // monitored values
}
```

## a. Server
The server exposes a set of operations:
 - add <id no_values values_arr> 
 - del <id>
 - update <id no_values values_arr>
 - read <id>
 - get_stat <id> - gets the statistics of one registry (min, max, average and
  median)
 - get_stat_all - gets the statistics for all the registries
 - login <user>
 - logout
 - load - creates a file called '<username>.rpcdb' and will write the users data
  in it
 - store - searches that users specific rpcdb file and will store all the data 
 inside from it, in the memory
Naturally, the server also keeps track of error handling, such as trying to 
login with a username that's already logged in, trying to logout when you're not
logged in, deleting a data with a non-existing id, etc.

## b. Client 
The client is a CLI tool for using all the procedures offered by the server. It 
create a connection, waits for a command and executes the desired remote procedure 
with the help of the client stub that is created after compiling the 'sensor.x' 
file.

## c. RPC specification file
In this application, the specification file is 'sensor.x'. It describes the data 
and methods that we are going to use.

# Usage
In one terminal
```bash
make
./server
```

And in other terminal
```bash
./client
```