Developed a client/server application for automatically receiving messages from subscribed
clients on different topics. The server saves the messages until the client which needs to receive
them becomes online.

Files:
    - server.c the code for the server

    - subscriber.c the code for the client
    
    -tcpclients.c helper file for struct with tcp clients, their messages, their state(online/offline) and 
    the socket they used for the connection 
    
    - topics.c helper file for 2 structs
        -strClients: the client id and it's subscription (to receive or not receive messages while offline)

        -srtTopics: the topics the clients want, the actual size and the max size of the vector.
    
    -payload.c struct for messages received from udp clients

Extra details:
    - Protaction against wrong commands
    - Ability for a client to change it's subscription type (ex: subscribe topic 0/1 -> subscribe topic 1/0 for changing the SF) 

