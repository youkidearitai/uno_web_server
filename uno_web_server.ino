#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// network configuration.  gateway and subnet are optional.

// the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//the IP address for the shield:
byte ip[] = { 192, 168, 0, 177 };
// the router's gateway address:
byte gateway[] = { 192, 168, 0, 1 };
// the subnet:
byte subnet[] = { 255, 255, 255, 0 };

// telnet defaults to port 23
EthernetServer server = EthernetServer(8080);

const int chipSelect = 4;

void setup() {
  // initialize the ethernet device
  Ethernet.begin(mac, ip, gateway, subnet);

  Serial.begin();

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1)
      ;
  }
  Serial.println("card initialized.");
  // start listening for clients
  server.begin();
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
    client.print("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>テストサーバー</title></head>");
    client.print("<body>");
    client.print("<h1>It works!!</h1>");
    File root = SD.open("/");
    client.print("<ul>");
    printDirectory(client, root, 0);
    root.close();
    client.print("</ul>");
    client.print("</body></html>");
  } else {
    File dataFile = SD.open(header_separators[1].substring(1));
    if (dataFile) {
      client.println("HTTP/1.1 200 OK");
      client.println("");
      while (dataFile.available()) {
        client.write(dataFile.read());
      }
      dataFile.close();
    } else {
      client.println("HTTP/1.1 404 Not Found");
      client.println("Content-Type: text/html; charset=UTF-8");
      client.println("");
      client.println("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>見つかりません</title></head><body><h1>Debug</h1><p>" + header_separators[1].substring(1) + "</p></body></html>");
    }
  }

  client.stop();
}

void printDirectory(EthernetClient client, File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    client.print("<li>");
    client.print(entry.name());
    if (entry.isDirectory()) {
      client.println("<ul>");
      printDirectory(client, entry, numTabs + 1);
      client.println("</ul>");
    } else {
      // files have sizes, directories do not
      client.println(entry.size(), DEC);
    }
    entry.close();
  }
}