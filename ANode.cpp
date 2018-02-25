#include "Arduino.h"
#include "ANode.h"

#define DEBUG true

// empty constructor
Event::Event() { }

Event::Event(String timestamp, String source, String name, String data)
{
  // constructor
  _timestamp = timestamp;
  _source = source;
  _name = name;
  _data = data;
}

String Event::getTimestamp()
{
  return _timestamp;
}

String Event::getSource()
{
  return _source;
}

String Event::getName()
{
  return _name;
}

String Event::getData()
{
  return _data;
}

// empty constructor
ANode::ANode() { }

ANode::ANode(HardwareSerial &espSerial, HardwareSerial &debugSerial, String sendChannel, String gatewayIp, String gatewayPort,
            String ssid, String pass)
{
  // constructor
  // espSerial is the serial port the esp8266 is connected to.
  _espSerial = &espSerial;
  _debugSerial = &debugSerial;
  _sendChannel = sendChannel;
  _gatewayIp = gatewayIp;
  _gatewayPort = gatewayPort;

  _ssid = ssid;
  _pass = pass;

  startServer();
}

// private methods

void ANode::startServer() {
  _debugSerial->println("STARTING...");
  // rst
  sendCommand("AT+RST", 2000, DEBUG);
  // Connect to wireless network
  sendCommand("AT+CWJAP=\"" + _ssid + "\",\"" + _pass + "\"", 2000, DEBUG);
  delay(10000);
  sendCommand("AT+CWMODE=1", 1000, DEBUG);
  // Shows IP Address
  sendCommand("AT+CIFSR", 1000, DEBUG);
  // Configure to multiple connections
  sendCommand("AT+CIPMUX=1", 1000, DEBUG);
  // Start web server at port 80
  sendCommand("AT+CIPSERVER=1,80", 1000, DEBUG);
  // if successful, return true
  return true;
}

void ANode::listenToEvent() {
  if (_espSerial->available())
  {
    if (_espSerial->find((char*)"+IPD,"))
    {
      delay(300);
      int connectionId = _espSerial->read() - 48;
      int connection = _espSerial->read();
      _debugSerial->println(connection);

      // String webpage = "<head><meta http-equiv=""refresh"" content=""3"">";
      // webpage += "</head><h1><u>ESP8266 - Web Server</u></h1><h2>Porta";
      // webpage += "Digital 8: </h2>";
      // webpage += "<h2>Porta Digital 9: ";
      // webpage += "</h2>";

      // String cipSend = "AT+CIPSEND=";
      // cipSend += connectionId;
      // cipSend += ",";
      // cipSend += webpage.length();

      // sendCommand(cipSend, 1000, DEBUG);
      // sendCommand(webpage, 1000, DEBUG);

      String closeCommand = "AT+CIPCLOSE=";
      closeCommand += connectionId; // append connection id
      sendCommand(closeCommand, 3000, DEBUG);
    }
  }

  return false;
}

void ANode::sendCommand(String command, const int timeout, boolean debug)
{
  // Send AT commands to the module
  String response = "";
  _espSerial->println(command);
  long int time = millis();
  while ((time + timeout) > millis())
  {
    while (_espSerial->available())
    {
      // The esp has data so display its output to the serial window
      char c = _espSerial->read(); // read the next character.
      response += c;
    }
  }
  if (debug)
  {
    _debugSerial->print(response);
  }
}

// public commands

bool ANode::sendEvent(Event &event, bool debug)
{
  _debugSerial->println("before tcp connection"); //xxx
  // start a TCP connection.
  _espSerial->println("AT+CIPSTART=" + _sendChannel + ",\"TCP\",\"" + _gatewayIp + "\"," + _gatewayPort);
  _debugSerial->println("after tcp connection"); //xxx

  if(_espSerial->find((char*)"OK") && debug && _debugSerial) { _debugSerial->println("TCP connection ready"); }
  delay(300);

  // Send packet
  // arduino event protocol (aep) format:
  // 1970-01-01T00:00:00Z\r\n
  // source\r\n
  // event_name\r\n
  // data_length(int32)\r\n
  // data
  String packet = event.getTimestamp() + "\r\n" +
                  event.getSource() + "\r\n" +
                  event.getName() + "\r\n" +
                  event.getData().length() + "\r\n" +
                  event.getData();

  String sendCmd = "AT+CIPSEND=2,"; // determine the number of caracters to be sent.
  _espSerial->print(sendCmd);
  _espSerial->println(packet.length());

  delay(500);

  if(_espSerial->find((char*)">")) {
    if (debug && _debugSerial) { _debugSerial->println("Sending event..."); }
    _espSerial->print(packet);

    if(_espSerial->find((char*)"SEND OK")) {
      if (debug && _debugSerial) { _debugSerial->println("Event sent"); }

      while (_espSerial->available()) {
        String tmpResp = _espSerial->readString();
        if (debug && _debugSerial) { _debugSerial->println(tmpResp); }
      }

      // close connection
      _espSerial->println("AT+CIPCLOSE");
      return true;
    }
  }

  return false;
}
