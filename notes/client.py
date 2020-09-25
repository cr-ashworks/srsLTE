#! /usr/bin/env python3

#
#   Hello World client in Python
#   Connects REQ socket to tcp://localhost:5555
#   Sends "Hello" to server, expects "World" back
#

import zmq

context = zmq.Context()

#  Socket to talk to server
print("Connecting to hello world server…")
tx_socket = context.socket(zmq.PUSH)
tx_socket.connect("tcp://localhost:2005")

rx_socket = context.socket(zmq.PULL)
rx_socket.connect("tcp://localhost:2004")


#  Do 10 requests, waiting each time for a response
for request in range(10):
    print("Sending request %s …" % request)
    tx_socket.send(b"Hello")

    #  Get the reply.
    message = rx_socket.recv()
    print("Received reply %s [ %s ]" % (request, message))
