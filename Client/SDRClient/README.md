#Client

The client is designed to run on a desktop machine compiled with the QT framework. This code can be easily compiled to run on any desktop OS with QT Creator. Only Windows and Linux have been tested.

Client is hard-coded to connect to a server with address 192.168.10.1, and provides an error message if unavaliable. Data is sent and recieved until the Client is closed and a disconnect is sent to the server. Only one client can be connected to a server instance at a time.
