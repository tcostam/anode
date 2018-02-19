/*
*
* Web Server & Web Client com modulo ESP8266
*
*/

#define DEBUG true

String ssid = "xxx";  // your network SSID (name)
String pass = "xxx"; // your network password

// http client info
String server = "x.x.x.x";
String host = "api.test.com";
String uri = "/v1/";
String data = "";

void setup()
{
  // initialize serial ports
  Serial.begin(115200);
  Serial1.begin(115200);

  // rst
  sendData("AT+RST", 2000, DEBUG);
  // Conecta a rede wireless
  sendData("AT+CWJAP=\"" + ssid + "\",\"" + pass + "\"", 2000, DEBUG);
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

void loop()
{
  // Verifica se o ESP8266 esta enviando dados
  if (Serial1.available())
  {
    if (Serial1.find("+IPD,"))
    {
      delay(300);
      int connectionId = Serial1.read() - 48;

      String webpage = "<head><meta http-equiv=""refresh"" content=""3"">";
      webpage += "</head><h1><u>ESP8266 - Web Server</u></h1><h2>Porta";
      webpage += "Digital 8: ";
      int a = digitalRead(8);
      webpage += a;
      webpage += "<h2>Porta Digital 9: ";
      int b = digitalRead(9);
      webpage += b;
      webpage += "</h2>";

      String cipSend = "AT+CIPSEND=";
      cipSend += connectionId;
      cipSend += ",";
      cipSend += webpage.length();

      sendData(cipSend, 1000, DEBUG);
      sendData(webpage, 1000, DEBUG);

      String closeCommand = "AT+CIPCLOSE=";
      closeCommand += connectionId; // append connection id

      sendData(closeCommand, 3000, DEBUG);
    }
  }
}

String sendData(String command, const int timeout, boolean debug)
{
  // Envio dos comandos AT para o modulo
  String response = "";
  Serial1.println(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (Serial1.available())
    {
      // The esp has data so display its output to the serial window
      char c = Serial1.read(); // read the next character.
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}

void httppost() {
  //start a TCP connection.
  Serial1.println("AT+CIPSTART=2,\"TCP\",\"" + server + "\",80");

  if(Serial1.find("OK")) {
    Serial.println("TCP connection ready");
  }

  delay(1000);

  String postRequest = "GET " + uri + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" +
                       "\r\n";

  String sendCmd = "AT+CIPSEND=2,"; //determine the number of caracters to be sent.
  Serial1.print(sendCmd);
  Serial1.println(postRequest.length());

  delay(500);

  if(Serial1.find(">")) {
    Serial.println("Sending..");
    Serial1.print(postRequest);

    if( Serial1.find("SEND OK")) {
      Serial.println("Packet sent");

      while (Serial1.available()) {
        String tmpResp = Serial1.readString();
        Serial.println(tmpResp);
      }

      // close the connection
      Serial1.println("AT+CIPCLOSE");
    }
  }
}

// espSerial is the serial port the esp8266 is connected to.
void aepSend(HardwareSerial &espSerial, String channel, String server, String port
            String timestamp, String source, String event, String message) {

  //start a TCP connection.
  espSerial.println("AT+CIPSTART=" + channel + ",\"TCP\",\"" + server + "\"," + port);

  if(espSerial.find("OK")) {
    Serial.println("TCP connection ready");
  }

  delay(300);

  // Send packet
  // 1970-01-01T00:00:00Z + source + event + length(int32) + message
  String packet = timestamp + "\r\n" +
                  source + "\r\n" +
                  event + "\r\n" +
                  message.length() + "\r\n" +
                  message;

  String sendCmd = "AT+CIPSEND=2,"; //determine the number of caracters to be sent.
  espSerial.print(sendCmd);
  espSerial.println(packet.length());

  delay(500);
}
