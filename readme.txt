****************************
          Read Me
****************************
Major 2            CSCE 3600
Thompson           MWF 9:30A
****************************



****************************
        Group Members
****************************

Peter Neal
Alex Alfonso
Nicole Russo
Preston Mitchell

****************************
        Organization
****************************

- Peter Neal
C sockets and multi-threading, program flow, code implementation

- Alex Alfonso
Debugging, general code support, socket support, makefile

- Nicole Russo
Program logic model, debugging, general code support

- Preston Mitchell
Beautification, cleanup

****************************
      Design Overview
****************************

         +Server+
The server main() listens for incoming connections. Upon accepting a connection, it's file descriptor is passed to a connection handler.
The connection handler first determines the client number by comparing it's file descriptor against a global table of file descriptors, 
using the index of the table as the client number. At this time, it also assigns its file desctiptor to the table, allowing other threads to see that it's open.
The server must always remember to update the table back to default values when a client closes. The server then sends the client its number, unless 
it's been determined this is the third client, in which case it sends a "FULL" string. The client handles this and disconnects from its end.

The server then receives values from the client, blocking for the entire length of the buffer. Server first checks that recv == 0, and if so, handles things appropriately.
If not, add the number received from the client to a local (in thread) value, then return this value to the client.
Server then makes sure 2 clients are connected by referencing file descriptor table. If 2 clients are connected and the current, in thread total is within range,
place the local, in thread total into a global table of totals, using the client indexes. Server then determines the other client's file descriptor by referencing
global file descriptor table (the client that didn't just post a total is the other client, table will reflect this). Server constructs a buffer containing a command to
connect to a specified port. Server sends first client a confirmation to go into server mode. Global totals table and local total values are reset and action repeats.

        +Client+
The client main() ensures the correct amount of arguments exist, then assigns them to their appropriate variables. Client then creates a socket and attempts
to connect to the server using the parameters specified in argv. If connection is successful, client recv()s message from server containing its client number or a fail message.
If client recieves "full", exit. If not, begin. loop.

Client queries for data. If user enters zero, exit. Send data to server, wait for reply. Parse reply for any commands from the server. If no commands, print server reply,
store in an integer. If integer is in range, wait for confirmation from server to enter server mode. If confirmed, open a new socket on the port equal to the total, listen and accept incoming connection.
Recieve a total from the other client. Resume normal operation. If client does not recieve confirmation, resume normal operation. Clients do NOT keep track of their own totals, the server "reminds"
the clients of their totals. Clients then compare every time they are reminded. This allows the server to control what clients are doing, always.

If client recieves a command to connect to a specified port, client recieves last total from the server then disconnects. Using the ip specified at runtime, client creates a new socket and reconnects
to the IP on the port equal to the total. Client forwards its total from the server to the other client, then exits.

If the total is greater than 49151, the total rolls over.

****************************
   Complete Specification
****************************

We designed our program logic according to spec as best we could, building our model based on the in-class explanation as well as the posted expected output.
We designed and coded for each case discussed and shown in the example output. Much concerted effort went into making sure everyone understood the program logic the same way,
and so there was no ambiguity between us to what our code should accomplish and be able to accomplish.

That said, during the arbitration period, we wondered whether the client was to remain connected to the other client, and whether the other client was to go into a full-server mode.
In this case, we planned to have it run a slightly trimmed version of the same connection_handler the server was running, however after examining the expected output, we concluded 
the client disconnected immediately after forwarding its total.

****************************
        Known Bugs
****************************

When client A begins listening, a message to client B has already been sent to connect to client A.
Before client B recieves this message, it first sends a number. If This number is in the port range, the server will send client A the connect command, using the total client B just exited with.
If the client executed a function to enter server mode, this could fix it. Instead, we've written the functionality in-line, and so can't easily enter server mode from this state.
