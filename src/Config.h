#pragma once

// Max amount of requests to send to a single device if it does not respond to the first packets
const int MAX_REQUEST_PACKET = 6;
// Time in ms between the requests
const int DELAY_BETWEEN_PACKETS = 1200;

// The frequency that the lora module is sending to and receiving from
const int LORA_FREQUENCY = 868E6; // Old: 433E6 

// The port on which the webserver will be listening
const int WEBSERVER_PORT = 80;

// How long to wait and recheck if the connection to the wifi could be established
// in ms
const int WIFI_CONNECTION_DELAY = 2000; //ms

// How long the wait min and max when an request got send to the master
// Used to get a random between and to desync the differnet nodes to avaid collisions
const int MIN_RANDOM_DELAY = 500; //ms
const int MAX_RANDOM_DELAY = 5000; //ms

// How long to wait at least between request-rounds
// This way it can be ensured that no request gets send more frequently as the here specified time
const long REQUEST_ROUND_DELAY = 1000 * 60; //ms