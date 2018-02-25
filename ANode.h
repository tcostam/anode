/*
*
* Web Server & Web Client library
* for arduino with ESP8266
*
*/

#ifndef ANode_h
#define ANode_h

#include "HardwareSerial.h"

class Event
{
  public:
    Event();
    Event(String timestamp, String source, String name, String data);
    String getTimestamp();
    String getSource();
    String getName();
    String getData();
  private:
    String _timestamp;
    String _source;
    String _name;
    String _data;
};

class ANode
{
public:
  ANode();
  ANode(HardwareSerial &espSerial, HardwareSerial &debugSerial, String sendChannel, String gatewayIp, String gatewayPort,
        String ssid, String port);
  bool sendEvent(Event &event, bool debug);
  void listenToEvent();
private:
  HardwareSerial *_espSerial; // esp8266 serial port
  HardwareSerial *_debugSerial; // debug serial port
  String _sendChannel;
  String _gatewayIp;
  String _gatewayPort;
  String _ssid;
  String _pass;
  void sendCommand(String command, const int timeout, boolean debug);
  void startServer();
};

#endif
