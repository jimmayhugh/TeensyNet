/*  TeensyEthernet.h
    Version 0.01 0831/2014
    by Jim Mayhugh
*/
#ifndef TETH_H
#define TETH_H

// Ethernet Stuff

// #define STATIC_IP // uncomment to use a static IP Address

// The IP address will be dependent on your local network:
// buffers for receiving and sending data

char PacketBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char ReplyBuffer[UDP_TX_PACKET_MAX_SIZE];  // a string to send back


const uint8_t wizReset = 23;         // WIZ nic reset

unsigned int localPort = 2652;      // local port to listen on

// set up the static IP address if you want to use one
#ifdef STATIC_IP
IPAddress ip(192, 168, 1, 51);
#endif

#endif
