#include "Arduino.h"
#include "ANode.h"

#define DEBUG true

Event::Event(String timestamp, String source, String name, String data)
{
  // constructor
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

ANode::ANode(HardwareSerial &espSerial, HardwareSerial &debugSerial, String sendChannel, String gatewayIp, String gatewayPort)
{
  // constructor
  // espSerial is the serial port the esp8266 is connected to.
  _espSerial = &espSerial;
  _debugSerial = &debugSerial;
  _sendChannel = sendChannel;
  _gatewayIp = gatewayIp;
  _gatewayPort = gatewayPort;

  // init board
  _espSerial->begin(115200);
  _debugSerial->begin(115200);

  // rst
  sendData("AT+RST", 2000, DEBUG);
  // Conecta a rede wireless
  sendData("AT+CWJAP=\"" + _ssid + "\",\"" + _pass + "\"", 2000, DEBUG);
  delay(10000);
  sendData("AT+CWMODE=1", 1000, DEBUG);
  // Mostra o endereco IP
  sendData("AT+CIFSR", 1000, DEBUG);
  // Configura para multiplas conexoes
  sendData("AT+CIPMUX=1", 1000, DEBUG);
  // Inicia o web server na porta 80
  sendData("AT+CIPSERVER=1,80", 1000, DEBUG);

  //  AT+CIPMUX?
  sendData("AT+CIPMUX?", 1000, DEBUG);
}

String ANode::sendData(String command, const int timeout, boolean debug)
{
  // Sending of AT commands to the module
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
  return response;
}

bool ANode::sendEvent(Event &event, bool debug)
{
  // start a TCP connection.
  _espSerial->println("AT+CIPSTART=" + _sendChannel + ",\"TCP\",\"" + _gatewayIp + "\"," + _gatewayPort);


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
