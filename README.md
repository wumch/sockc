# sockc

A socks5 relay server, works as downstream of [csocks](https://github.com/wumch/csocks).

Configuration options worker-count, io-threads are not currently supported.

# requirement
1. boost (>=1.42, boost-system, boost-filesystem, boost-program_options, boost-asio)  
2. libcrypto++  
3. [stage](https://github.com/wumch/stage)  
and cmake (>=2.8)  

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
