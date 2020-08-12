/*
 * Copyright 2013-2020 Software Radio Systems Limited
 *
 * This file is part of srsLTE.
 *
 * srsLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>
#include <thread>

#include <stdbool.h>

#include "srslte/phy/rf/rf.h"
#include "srslte/srslte.h"
#include "srsue/hdr/second_socket.h"

/* Example function to initialize ZMQ socket */
bool running = true;
static void*       zmq_ctx  = NULL;
static void*       zmq_sock_send = NULL;
static void*       zmq_sock_rec = NULL;
static const char* zmq_sock_send_args = "tcp://*:2003";
static const char* zmq_sock_rec_args = "tcp://*:2002";

int Second_socket::init()
{
  zmq_ctx = zmq_ctx_new();
  // Create socket
  zmq_sock_send = zmq_socket(zmq_ctx, ZMQ_PUB);
  if (!zmq_sock_send) {
    fprintf(stderr, "Error: creating transmitter socket\n");
  }

  // The transmitter starts first and creates the socket
  if (zmq_bind(zmq_sock_send, zmq_sock_send_args)) {
    fprintf(stderr, "Error: connecting transmitter socket: %s\n", zmq_strerror(zmq_errno()));
  }
  zmq_sock_rec = zmq_socket(zmq_ctx, ZMQ_PUB);
  if (!zmq_sock_rec) {
    fprintf(stderr, "Error: creating transmitter socket\n");
  }

  // The transmitter starts first and creates the socket
  if (zmq_bind(zmq_sock_rec, zmq_sock_rec_args)) {
    fprintf(stderr, "Error: connecting transmitter socket: %s\n", zmq_strerror(zmq_errno()));
  }

  Second_socket::receive();
  return 0;
}



/* Example function to write samples to ZMQ socket
int send(void** buffer, uint32_t buffer_len)
{
  // wait for request
  uint8_t dummy;
  zmq_recv(zmq_sock_rec, &dummy, sizeof(dummy), 0);
  return zmq_send(zmq_sock_send , buffer[0], buffer_len, 0);
}*/

int Second_socket::receive()
{
  void* buffer[2];
  uint32_t buflen      = 0; // in samples

  while (running) {
    // wait for request
    uint8_t dummy;
    zmq_recv(zmq_sock_rec, &dummy, sizeof(dummy), 0);
    zmq_recv(zmq_sock_rec , buffer, buflen, 0);
  };
  return 0;
}

int Second_socket::stop(){
  running = false;
  return 0;
}

/*int main(int argc, char** argv)
{
  void*    buffer[max_rx_antennas];
  int      n           = 0;
  uint32_t buflen      = 0; // in samples
  uint32_t sample_size = 8;

  // Sets signal handlers
  signal(SIGINT, int_handler);
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);
  sigprocmask(SIG_UNBLOCK, &sigset, NULL);

  // Parse args
  parse_args(argc, argv);

  // Initializes ZMQ
  if (init_zmq()) {
    ERROR("Initializing ZMQ\n");
    exit(-1);
  }

  if (init_radio(&buflen)) {
    ERROR("Initializing Radio\n");
    exit(-1);
  }

  // Initializes memory for input buffer
  bzero(buffer, sizeof(void*) * max_rx_antennas);
  for (int i = 0; i < nof_rx_antennas; i++) {
    buffer[i] = srslte_vec_cf_malloc(buflen);
    if (!buffer[i]) {
      perror("malloc");
      exit(-1);
    }
  }

  printf("Streaming samples...\n");
  uint32_t print_cnt = 0;
  while (keep_running) {
    n = rx_radio(buffer, buflen);
    if (n < 0) {
      ERROR("Error receiving samples\n");
      exit(-1);
    }
    if (srslte_verbose == SRSLTE_VERBOSE_INFO) {
      printf("Received %d samples from radio\n", n);
    }

    n = tx_zmq((void**)buffer, n * sample_size);

    if (n == -1) {
      print_cnt++;
      if (print_cnt == 1000) {
        printf("ZMQ socket not connected\n");
        print_cnt = 0;
      }
    } else {
      if (srslte_verbose == SRSLTE_VERBOSE_INFO) {
        printf("Transmitted %d bytes to ZMQ\n", n);
      }
    }
  }

  // Cleanup memory and close RF device
  for (int i = 0; i < nof_rx_antennas; i++) {
    if (buffer[i]) {
      free(buffer[i]);
    }
  }
  close_radio();

  printf("Exit Ok\n");
  exit(0);
}

/* Example function to initialize the Radio frontend. In this case, we use srsLTE RF API to open a device,
 * which automatically picks UHD, bladeRF, limeSDR, etc.
 
static srslte_rf_t radio   = {};
static char*       rf_args = "fastpath";
static float       rf_gain = 40.0, rf_freq = -1.0, rf_rate = 11.52e6;
static uint32_t    rf_recv_frame_size_ms = 1;

static int         init_radio(uint32_t* buffer_len)
{
  // Uses srsLTE RF API to open a device, could use other code here
  printf("Opening RF device...\n");
  if (srslte_rf_open_multi(&radio, rf_args, nof_rx_antennas)) {
    ERROR("Error opening rf\n");
    return -1;
  }

  printf("Set RX freq: %.2f MHz\n", srslte_rf_set_rx_freq(&radio, nof_rx_antennas, rf_freq) / 1000000);
  printf("Set RX gain: %.2f dB\n", srslte_rf_set_rx_gain(&radio, rf_gain));
  float srate = srslte_rf_set_rx_srate(&radio, rf_rate);
  if (srate != rf_rate) {
    ERROR("Error setting samplign frequency %.2f MHz\n", rf_rate * 1e-6);
    return -1;
  }

  if (buffer_len) {
    *buffer_len = srate * rf_recv_frame_size_ms * 1e-3;
  }

  printf("Set RX rate: %.2f MHz\n", srate * 1e-6);
  srslte_rf_start_rx_stream(&radio, false);
  return 0;
}

/* Example implementation to receive from Radio frontend. In this case we use srsLTE
 
static int rx_radio(void** buffer, uint32_t buf_len)
{
  return srslte_rf_recv_with_time_multi(&radio, buffer, buf_len, true, NULL, NULL);
}

static void close_radio()
{
  srslte_rf_close(&radio);
}

static void int_handler(int dummy)
{
  keep_running = false;
}
*/