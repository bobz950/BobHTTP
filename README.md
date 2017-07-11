# BobHTTP
Coded by Robert Alianello.
A very simple multi-threaded HTTP server written in C++. This is simply a fun project I am working on in my free time.

Development and testing has been conducted on Windows with the intention for later cross platform support.

The project uses ASIO for its sockets. 

Current Features:
  Processing GET and POST requests.
  Parsing and mapping all parameters sent within request.
  Mapping request location to cooresponding file on server.
  
To build the project:
  Download ASIO for your platform: http://think-async.com/Asio/Download
  Place include/asio directory in root directory.
  Place include/asio.hpp in root directory.
  Compile and run.
