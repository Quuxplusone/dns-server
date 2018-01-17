A toy authoritative DNS server and DNS stub-resolver,
forked from https://github.com/tomasorti/dns-server

This authoritative server is just a toy:

* UDP only, no TCP
* Single-threaded server (for now)
* No attempt at proper name lookup
* No CNAME, no DNAME
* No AXFR
* No UPDATE

Its (single) "zone file" uses a restrictive subset of standard DNS syntax:

* No relative names
* No elided or implicit fields
* Only a handful of RR types
* No checking for common error modes
* Abort with an exception if you get the syntax wrong

The DNS stub resolver (a.k.a. client, a.k.a. `dig` clone) is also a toy.

* UDP only, no TCP
* Connect only to 127.0.0.1
* No relative names

Test with:

    ./dns-auth-server 9000 zone.txt &

    dig @127.0.0.1 -p 9000 www.google.com.

    ./dns-dig 9000 www.google.com. ANY

References:

* [RFC 1034 "Domain Names - Concepts and Facilities"](https://tools.ietf.org/html/rfc1034)
* [RFC 1035 "Domain Names - Implementation and Specification"](https://tools.ietf.org/html/rfc1035)
* [RFC 4592 "The Role of Wildcards in the Domain Name System"](https://tools.ietf.org/html/rfc4592)
* [RFC 6891 "Extension Mechanisms for DNS (EDNS(0))"](https://tools.ietf.org/html/rfc6891)
