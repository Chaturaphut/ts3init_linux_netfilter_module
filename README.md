ts3init Linux netfilter module
==============================
A Linux netfilter module to help filter ts3init floods on TeamSpeak 3 servers

TeamSpeak 3 servers are a popular (D)Dos target. They are harder to protect than
other servers because the TeamSpeak 3 protocol is based on UDP. A popular
method, that is hard to combat by hosters are called *init floods*. This is
where the attacker sends many connection request to the server, usually spoofing
the source address to make it harder to block them. The TeamSpeak 3 protocol has
an anti spoof check as the first stage of the connection, but the server program
can not keep up with the flood of packets.

This set of plugins is designed to let the Linux kernel, or rather netfilter,
handle the anti spoofing stages of a new TeamSpeak 3 connection.

How to install
==============

```
make
sudo make install
sudo depmod -a
sudo modprobe xt_ts3init
```

Protocol background and module description
==========================================
When a TeamSpeak 3 client attempts to connect to a TeamSpeak 3 server, it sends
out a *get cookie* packet. The server then replies with a *set cookie* packet.
This packet has some secret information about the connection details. The client
then response with a *get puzzle* packet. This packet includes the cookie that
it got previously. The server now validates this cookie and if its correct 
continues with the rest of the connection packets.

This software packages comes with two netfilter match extensions, and three
netfilter target extensions which we will discuss next. Combined these
extensions and some other netfilter modules, can handle the initial connection
phase for a TeamSpeak 3 server. This prevents that any packet, with a spoofed
source ip address, to reach the TeamSpeak 3 server.

Match extensions
================

ts3init_get_cookie
--------------------
Matches if the packet in question is a valid TeamSpeak 3 *get cookie* packet
from the client.
There are additional parameters that can be set:
```
$ iptables -m ts3init_get_cookie -h
<..>
ts3init_get_cookie match options:
  --min-client n                The client needs to be at least version n.
  --check-time sec              Check packet send time request.
                                May be off by sec seconds.
```
* `min-client` checks that the client version in the packet is at least the
  version specified. 
* `check-time` compares the UTC timestamp in the client packet to the UTC time
  on the server. If they differ too much, the packet is not matched.

ts3init_get_puzzle
--------------------

Matches if the packet in question is a valid TeamSpeak 3 *get puzzle* packet
from the client. There are additional parameters that can be set:
```
$ iptables -m ts3init_get_puzzle -h
<..>
ts3init_get_puzzle match options:
  --min-client n               The sending client needs to be at least version.
  --check-cookie               Check that the cookie was generated by same seed.
  --random-seed <seed>         Seed is a 60 byte hex number.
                               A source could be /dev/random.
  --random-seed-file <file>    Read the seed from a file.
```
* `min-client` checks that the client version in the packet is at least the
  version specified. 
* `check-cookie` matches if it matches the cookie that was generated in the
  netfilter target extension `TS3INIT_SET_COOKIE`. To match the seed needs to be
  exactly the same of course. It is possible to check cookies that were
  generated on a different machine, provided that those machines have the same
  date and time, and the same seed specified. In other words: The cookie is
  created in a deterministic way, depending only on the current time and the
  seed. If check-cookie is specified, either random-seed or random-seed-file
  need to be specified too.

Target extensions
=================

TS3INIT_RESET
--------------- 

This target drops the packet and sends a *reset* packet back to the sender. The
sender should always be the TeamSpeak 3 client. Starting with the TeamSpeak 3.1
client, the client will react to the reset packet by resending the *get cookie*
to the server. Older clients do not handle this packet. It takes no parameters.

How to use
==========
The idea for which these extensions were developed was to create a few iptables
rules that do the anti spoofing phase for a TeamSpeak server. This can be done
as follows:
* [recommended] Disable connection tracking with the help of raw table
* Create ipset: ts3_authorized with a timeout of 30 seconds
* If a source ip address is in the ipset ts3_authorized, renew the entry in the
  set to update the timeout then accept the packet.
* Use `ts3init_get_cookie` matches to get connection requests and reply with
  `TS3INIT_SET_COOKIE`.
* Use `ts3init_get_puzzle` matches to get the cookie replies from the client.
  If they match, add the source ip address to the ts3_authorizing ipset and then
  reply with `TS3INIT_RESET`
* Drop all other packets

It is even possible to make a more detailed firewall. Perhaps connection
tracking could be used instead of ipset.

Example iptables setup
======================

