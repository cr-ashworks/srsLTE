#! /usr/bin/env python3

#
#   Hello World server in Python
#   Binds REP socket to tcp://*:5555
#   Expects b"Hello" from client, replies with b"World"
#

import time
import zmq

context = zmq.Context()
tx_socket = context.socket(zmq.PUSH)
tx_socket.bind("tcp://*:2004")

rx_socket = context.socket(zmq.PULL)
rx_socket.bind("tcp://*:2005")

while True:
    #  Wait for next request from client
    message = rx_socket.recv()
    print("Received request: %s" % message)

    #  Do some 'work'
    time.sleep(1)

    #  Send reply back to client
    tx_socket.send(b"World")
