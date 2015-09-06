# sockc

A socks5 relay server, works as downstream of [csocks](https://github.com/wumch/csocks).

Configuration options worker-count, io-threads are not currently supported.

# requirement
boost (>=1.47, boost-system, boost-filesystem, boost-program_options, boost-asio)  
libcrypto++  
[stage](https://github.com/wumch/stage)  
cmake (>=2.8)  

# installation
$ git clone https://github.com/wumch/sockc.git sockc  
$ mkdir sockc-build  
$ cd sockc-build  
$ cmake ../sockc  
$ make  

# run
$ cd sockc-build  
$ cp -r ../sockc/etc ./  
$sudo touch /var/run/sockc.pid && sudo chown ${USER}:$(id -gn) !$  
$ ./sockc -h  
