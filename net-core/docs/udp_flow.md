UDP packetized data flow

A server binds its listening IP and port to its socket. Afterwards, it waits
for an incoming datagram. The client waits for its unbound socket to become
writeable. Afterwards it sends a datagram to the server address and port. The
server performs several checks on the received datagram and stores the source
address for later source checks. After successful checks, it sends back a
header with sequence and retry number equal to those used by the client. The
client performs a check on the acknowledge package and continues to send data
if needed. The server waits for further datagrams until the specified total of
data has been transmitted. From the second datagram in the sequence on, the
server will only accept datagrams coming from the same source address as the
first one.

```
                 server                               client


   |-->-|---> select (read)                    select (write) <----|-<--|
   |    |         |                                    |           |    |
   |    |-<-- timeout?                              timeout? -->---|    |
   |    | yes     | no                              no |       yes      |
   |    |         |             datagram               |                |
   |    |     recvfrom    <-----------------------   sendto             |
   |    |         |             65507 bytes            |                |
   |    |         |           header + data            |                |
   |    |    size check                                |                |
   |    |-<-- seq check                                |                |
   |   err  source check                               |                |
   |              |                                    |                |
   |    |---> select (write)                     select (read) <---|    |
   |    |         |                                    |           |    |
   |    |-<-- timeout?                              timeout? ------|    |
   |      yes     | no                              no |       yes      |
   |              |              datagram              |                |
   |           sendto     ------------------------>  recvfrom           |
   |              |              8 bytes               |                |
   |              |               header               |                |
   |              |                                    |                |
   |------<-- all received?                           all sent? -->-----|
       no         |                                    |        no
                  |  yes                           yes |
                done                                 done

```
