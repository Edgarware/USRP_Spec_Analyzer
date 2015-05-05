#Server

The server is designed to run on the ZYBO development board using an Arch Linux install, but it should run on any linux computer with a GNU Radio install.

Run command is `python2 sdr_server.py` Frequency scan range defaults to 400 Mhz to 4.4GHz. Server is currently coded with a default IP of 192.168.10.1 using port 9001, and assumes an SDR with an IP of 192.168.10.2. IP and port of server can be changed with the -i and -p options respectively. Additional options and defaults are listed if none are given.
