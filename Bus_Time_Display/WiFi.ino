#include <WiFi.h>
#include <WiFiMulti.h>
#include <Arduino_JSON.h>
#include "Secrets.h"

WiFiMulti WiFiMulti;

uint8_t current_time[] = {0,0,0};

void get_time() {
  const uint16_t port = 80;
  const char *host = "realtime.portauthority.org";
  WiFiClient client;

  if (!client.connect(host, port)) {
    bus_error = 1;
    return;
  }

  // Send the request to the server (with the secret api_key)
  client.print(
    "GET /bustime/api/v3/gettime?key=" + api_key + "&format=json HTTP/1.1\n"
    "Host: realtime.portauthority.org\n"
    "User-Agent: python-requests/2.31.0\n"
    "Connection: keep-alive\n\n"
  );

  // Wait with a 1 second timeout
  int maxloops = 0;
  while (!client.available() && maxloops < 1000) {
    maxloops++;
    delay(1);
  }

  // Check that we received a response
  if(client.available() > 0)
  {
    // Skip the response until the beginning of the JSON body
    client.readStringUntil('{');
    String json_str = "{";
    while(client.available() > 0) {
      String json_str_suffix = client.readString();
      json_str.concat(json_str_suffix);
    }
    client.stop();

    // Parse the JSON response
    JSONVar parsed_json = JSON.parse(json_str);
    if(JSON.typeof(parsed_json) != "undefined") {
      String time_str = (String)(const char*)parsed_json["bustime-response"]["tm"];
      current_time[0] = time_str.substring(9,11).toInt();
      current_time[1] = time_str.substring(12,14).toInt();
      current_time[2] = time_str.substring(15,17).toInt();
    } else {
      bus_error = 2;
    }
  }
  else
  {
    bus_error = 1;
  }
}

void get_predictions() {
  get_time();
  
  const uint16_t port = 80;
  const char *host = "realtime.portauthority.org";
  WiFiClient client;

  if (!client.connect(host, port)) {
    bus_error = 1;
    return;
  }

  // Send the request to the server (with the secret api_key)
  client.print(
    "GET /bustime/api/v3/getpredictions?top=8&rt=61A,61B,61C,61D&stpid=7117&tmres=s&rtpidatafeed=Port%20Authority%20Bus&key=" + api_key + "&format=json HTTP/1.1\n"
    "Host: realtime.portauthority.org\n"
    "User-Agent: python-requests/2.31.0\n"
    "Connection: keep-alive\n\n"
  );

  // Wait with a 1 second timeout
  int maxloops = 0;
  while (!client.available() && maxloops < 1000) {
    maxloops++;
    delay(1);
  }

  // Check that we received a response
  if(client.available() > 0)
  {
    // Skip the response until the beginning of the JSON body
    client.readStringUntil('{');
    String json_str = "{";
    while(client.available() > 0) {
      String json_str_suffix = client.readString();
      json_str.concat(json_str_suffix);
    }
    client.stop();

    // Parse the JSON response
    JSONVar parsed_json = JSON.parse(json_str);
    if(JSON.typeof(parsed_json) != "undefined") {
      for(int i = 0; i < 8; i++) {
        bus_names[i] = "";
        bus_times[i] = -1;
      }
      
      for(int i = 0; i < min(parsed_json["bustime-response"]["prd"].length(), 8); i++) {
        if(((int)parsed_json["bustime-response"]["prd"][i]["dyn"]) != 0) continue;
        // Set the bus name
        bus_names[i] = (const char*)(parsed_json["bustime-response"]["prd"][i]["rtdd"]);

        // Extract the prediction time as a string
        String time_str = (String)(const char*)parsed_json["bustime-response"]["prd"][i]["prdtm"];

        // Parse the prediction time into a number of seconds in the future
        int t_left = time_str.substring(9,11).toInt() - current_time[0]; // h
        if(t_left < 0) t_left += 24;
        t_left *= 60;
        t_left += time_str.substring(12,14).toInt() - current_time[1]; // m
        t_left *= 60;
        t_left += time_str.substring(15,17).toInt() - current_time[2]; // s

        // Convert to minutes and set the array
        t_left /= 60;
        bus_times[i] = t_left;
      }
      bus_error = 0;
    } else {
      bus_error = 2;
    }
  }
  else
  {
    bus_error = 1;
  }
}

void setup_wifi() {
  WiFiMulti.addAP(network_ssid, network_pass);
  
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
  }
}
