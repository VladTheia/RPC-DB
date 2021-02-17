build:
	rpcgen -C sensor.x
	gcc -c -std=c99 -o sensor_clnt.o sensor_clnt.c
	gcc -c -std=c99 -o sensor_svc.o sensor_svc.c
	gcc -c -std=c99 -o sensor_xdr.o sensor_xdr.c
	g++ -c -std=c++0x -o rpc_client.o rpc_client.cpp
	g++ -c -std=c++0x -o rpc_server.o rpc_server.cpp
	g++ -o client sensor_clnt.o rpc_client.o sensor_xdr.o
	g++ -o server sensor_svc.o rpc_server.o sensor_xdr.o

clean:
	rm -f client server sensor.h sensor_xdr.c sensor_svc.c sensor_clnt.c *.o *.rpcdb 