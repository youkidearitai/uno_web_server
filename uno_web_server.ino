#include <SPI.h>
#include <Ethernet.h>

// network configuration.  gateway and subnet are optional.

// the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAF, 0x13, 0xA0 };
//the IP address for the shield:
byte ip[] = { 192, 168, 0, 177 };
// the router's gateway address:
byte gateway[] = { 192, 168, 0, 1 };
// the subnet:
byte subnet[] = { 255, 255, 255, 0 };

// telnet defaults to port 23
EthernetServer server = EthernetServer(8080);

void setup() {
  // initialize the ethernet device
  Ethernet.begin(mac, ip, gateway, subnet);

  Serial.begin();
  // start listening for clients
  server.begin();
}

String getHTTPUrlFromHeader(String *str) {
}

void loop() {
  // if an incoming client connects, there will be bytes available to read:
  EthernetClient client = server.available();
  if (!client) {
    return;
  }

  String str = "";
  String header_separators[3];
  int index = 0;
  while (server.available()) {
    char one_byte = client.read();
    switch (one_byte) {
      case ' ':
        header_separators[index] = str;
        Serial.println(str);
        str = "";
        index++;
        if (index > 3) {
          client.println("HTTP/1.1 400 Bad Request");
          client.println("Content-Type: text/html; charset=UTF-8");
          client.println("");
          client.println("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>不正なリクエストです</title></head><body><h1>Invalid request!</h1></body></html>");
          client.stop();
          return;
        }
        break;
      case '\n':
        goto parse_end;
        break;
      default:
        str.concat(one_byte);
        break;
    }
  }
parse_end:;

  if (header_separators[1].equals("/")) {
    // Response header
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("");
    // Response body
    client.println("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>テストサーバー</title></head><body><h1>It works!!</h1></body></html>");
  } else {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("");
    client.println("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>見つかりません</title></head><body><h1>Debug</h1><p>" + header_separators[1] + "</p></body></html>");
  }

  client.stop();
}