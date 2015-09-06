# sockc

A socks4/5 relay server, works as downstream of [csocks](https://github.com/wumch/csocks).  
NOTE: Not fully compatible with the standard [SOCKS5](http://www.ietf.org/rfc/rfc1928.txt) protocol.

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
$ cp -r ../sockc/etc ./  # then you can edit etc/sockc.conf before run.  
$ sudo touch /var/run/sockc.pid && sudo chown ${USER}:$(id -gn) !$  
$ ./sockc -h  
