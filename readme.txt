c.
I have finished the tasks metioned in this assignment.
In my system, all processes are synchronous. 
The processes flow:
1.Three servers boot up and wait for receiving. edge server waits for TCP packets, two backend servers wait for UDP datagrams.
2.Client gets all operations from the input file.
3.Client packs every 10 operations as one packet and sends packets one by one.
4.Client waits for TCP packets.
5.Edge server gets those TCP packets and parses them into operations one by one and stores all the operations.
6.Edge server sends all operations using UDP to two backend servers one by one.
7.Edge server waits for receiving from the UDP socket.
8.Two backend servers start to receving all operations and operate each of them when it arrives.
9.Two backend servers return all results back to edge server.
10.Edge server gets the results,does sort job and sends them to client by serveral packets each of which consists of ten results.
11.Client gets the results and then outputs the results.

d.
My homework includes 4 files.They are client.cpp, edge.cpp,
server_and.cpp and server_or.cpp
client.cpp: collecting all operations, sending all packets to the edge server, getting results and showing them.
edge.cpp: getting operations from client, dividing operations into two part, sending operations to two backend servers, getting results from two servers and sending the results to the client.
server_and.cpp: getting operations from edge server, doing AND operations on each of them and send all the results to the edge server.
server_or.cpp: getting operations from edge server, doing OR operations on each of them and send all the results to the edge server.


e.
the format of file should be the same as the example in the pdf, and there should be no SPACE in the file.
It is ok to has '\n' at the end of message.
Capital letter and wrong letter is not allowed. There is no error check for the input.
One example:

and,1,1
or,1,1011
and,10,11


During TCP data transfer,after sending one packet which including ten operations, I set one second pause for receiver easier to receive and output a "..." to let people know there is a pause.

f.
client->edge server: one packet includes serveral operations like 'and,1,1or,1,1and,10,1'. The number of operations in a packet is less than 10.

edge server->backend server: one example is 'and,1,1#1$' .  #1 stands for the index   $ stands for the end of the message
If the index includes two digits, the format in the message will be a reversed one which is easy for me to operate this parameter.
For example, 'and,1,1#41$'. It is not meaning the No.41 but the No.14 operation.

two backend servers->edge server: one example is '1 and 1 = 1#1$'     #1 stands for the index     $ stands for the end of message

edge server->client: one example is '1 and 1 = 1$1 or 1 = 1$11 and 11 = 11$'    $stands for the end of one result;




