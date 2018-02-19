/*
*
* Web Server & Web Client library
* for arduino with ESP8266
*
*/

// Init node info --------------------------------------

// ESP8266 wiring
ANode.espSerial.begin(115200);

String ssid;
String pass;

// http client info
// String gatewayIp =  "x.x.x.x";
// String host = "api.example.com";
// String uri = "/v1/";
// String data = "";

// ------------------------------------------------------

struct Event {
  String timestamp;
  String source;
  String name;
  String data;

  // foo() : bar(3) {}   //look, a constructor
}

struct ANode {
  HardwareSerial espSerial; // esp8266 serial port
  HardwareSerial debugSerial // debug serial port

  String sendChannel;
  String gatewayIp;
  String gatewayPort;

  // espSerial is the serial port the esp8266 is connected to.
  // arduino event protocol (aep) format:
  // 1970-01-01T00:00:00Z\r\n
  // source\r\n
  // event_name\r\n
  // data_length(int32)\r\n
  // data
  bool sendEvent(Event &event, bool debug) {
    // start a TCP connection.
    espSerial.println("AT+CIPSTART=" + sendChannel + ",\"TCP\",\"" + gatewayIp + "\"," + gatewayPort);

    if(espSerial.find("OK") && debug && debugSerial) { debugSerial.println("TCP connection ready"); }
    delay(300);

    // Send packet
    // 1970-01-01T00:00:00Z + source + event + length(int32) + data
    String packet = event.timestamp + "\r\n" +
                    event.source + "\r\n" +
                    event.name + "\r\n" +
                    event.data.length() + "\r\n" +
                    event.data;

    String sendCmd = "AT+CIPSEND=2,"; // determine the number of caracters to be sent.
    espSerial.print(sendCmd);
    espSerial.println(packet.length());

    delay(500);

    if(espSerial.find(">")) {
      if (debug && debugSerial) { debugSerial.println("Sending event..."); }
      espSerial.print(packet);

      if(espSerial.find("SEND OK")) {
        if (debug && debugSerial) { debugSerial.println("Event sent"); }

        while (espSerial.available()) {
          String tmpResp = espSerial.readString();
          if (debug && debugSerial) { debugSerial.println(tmpResp); }
        }

        // close connection
        espSerial.println("AT+CIPCLOSE");
        return true;
      }
    }

    return false;
  }
}
