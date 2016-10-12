# ts3init Linux netfilter module
A Linux netfilter module to help filter ts3init floods on TeamSpeak 3 servers

TeamSpeak 3 servers are a popular (D)Dos target. They are harder to protect than other servers because the TeamSpeak 3 protocol is based on UDP. A popular method, that is hard to combat by hosters are called "init floods". This is where the attacker sends many connection request to the server, usually spoofing the source address to make it hard to block them. The TeamSpeak 3 protocol has an anti spoof check as the first stage of the connection, but the server program can not keep up with the flood of packets.

This set of plugins is designed to let the Linux kernel (or rather netfilter) handle the anti spoofing stages of a new TeamSpeak 3 connection.

## Protocol background and module description
When a TeamSpeak 3 client attempts to connect to a TeamSpeak 3 server, it sends out a "get cookie" packet. The server then replies with a "set cookie" packet. This packet has some secret information about the connection details. The client then response with a "get puzzle" packet. This packet includes the cookie that it got previously. The server now validates this cookie and if its correct it continues with the rest of the connection packets.

This software packages comes with two netfilter match extensions, and two netfilter target extensions which we will discus next. These extensions combined with some other netfilter modules, can take over the initial connection phase for a TeamSpeak 3 server. This makes it possible to prevent any packet that has not been verified to come from the ip address it claims as it's source, to reach the TeamSpeak 3 server.

## Match extensions
The first match extensions is called *ts3init_get_cookie*. It matches if the packet in question is a valid TeamSpeak 3 "get cookie" packet from the client. There are additional parameters that can be set:
```
$ iptables -m ts3init_get_cookie -h
<..>
ts3init_get_cookie match options:
  --min-client n The sending client needs to be at least version n.
  --check-time sec Check packet send time request. May be off by sec seconds.
```
The min-client parameter checks that the client version in the packet is at least the version specified. The check-time parameter compares the UTC timestamp in the client packet to the UTC time on the server. If they differ too much, the packet is not matched.

The second match extension is called *ts3init_get_puzzle*. It matches if the packet in question is a valid TeamSpeak 3 "get puzzle" packet from the client. There are additional parameters that can be set:
```
$ iptables -m ts3init_get_puzzle -h
<..>
ts3init_get_puzzle match options:
  --min-client n The sending client needs to be at least version n.
  --check-cookie seed Check the cookie. Assume it was generated with seed.
                      seed is a 60 byte random number in hex. A source
                      could be /dev/random.
```
The min-client parameter is the same as above. The check-cookie parameter matches if it matches the cookie that was generated in the netfilter target extension ts3init_set_cookie. To match the seed needs to be exactly the same of course. It is possible to check cookies that were generated on a different machine, provided that those machines have the same date and time, and the seem seed specified. In other words: The cookie is created in a deterministic way, depending only on the current time and the seed.

## Target extensions
The second target extension is called "TS3INIT_RESET". It takes no parameters. This target drops the packet and sends a "reset" packet back to the sender. The sender should always be the TeamSpeak 3 client. Starting with the TeamSpeak 3.1 client, the client will react to the reset packet by resending the "get cookie" to the server. Older clients do not handle this packet.

## How to use
The idea for which these extensions were developed was to create a few iptables rules that do the anti spoofing phase for a TeamSpeak server. This can be done as follows:
* [recommended] Disable connection tracking with the help of raw table
* Create ipset: ts3_authorized with a timeout of 30 seconds
* If a source ip address is in the ipset ts3_authorized, renew the entry in the set to update the timeout then accept the packet.
* Use ts3init_get_cookie matches to get connection requests and reply with TS3INIT_SET_COOKIE.
* Use ts3init_get_puzzle matches to get the cookie replies from the client. If they match, add the source ip address to the ts3_authorizing ipset and then reply with TS3INIT_RESET
* Drop all other packets

It is even possible to make a more detailed firewall. Perhaps connection tracking could be used instead of ipset.

## How to install
```
make
sudo make install
sudo depmod -a
sudo modprobe xt_ts3init
```

## Example iptables setup
