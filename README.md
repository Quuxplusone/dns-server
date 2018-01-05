A toy authoritative DNS server, forked from https://github.com/tomasorti/dns-server

This server is just a toy:

* UDP only, no TCP
* Single-threaded server (for now)
* No support for POINTER-encoded names
* No attempt at proper name lookup
* No wildcards, no CNAME, no DNAME
* No AXFR
* No UPDATE

Its (single) "zone file" uses a restrictive subset of standard DNS syntax:

* No relative names
* No elided or implicit fields
* Only a handful of RR types
* No checking for common error modes
* Abort with an exception if you get the syntax wrong

Test with:

    ./dnsserver 9000 zone.txt &
    dig @127.0.0.1 -p 9000 www.google.com.
