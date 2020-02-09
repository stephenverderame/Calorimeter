#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#define RX D6 //3
#define TX D7 //1
#define HTTP_PORT 80
#define SRVR_PORT 8000
SoftwareSerial ard(RX, TX);
WiFiUDP udp;
WiFiServer server(HTTP_PORT);
long long lastUpdate = 0;
IPAddress ip;
bool hasPeer = false;
const char * htmlFile = R"V0G0N(<!DOCTYPE html>
<html>
<head>
  <title>Calorimeter Config</title>
  <style>
    body{
      text-align: center;
    }
    form{
      border-style:solid;
      border-color:gray;
      border-width: 2px;
      margin-left: 30%;
      margin-right: 30%;
      border-radius: 4px;
    }
  </style>
</head>
<body>
  <h3>Calorimeter Config</h3>
  <form method="POST">
    <br>
    <input type="text" name="ssid" placeholder="ssid"/>
    <br>
    <input type="text" name="pass" placeholder="password"/>
    <br>
    <input type="text" name="ipaddr" placeholder="server ip"/>
    <br><br>
    <input type="submit"/>
    <br><br>
  </form>
</body>
</html>)V0G0N";
String resp;
String symbolDecode(const String && s)
{
  String str;
  str.reserve(s.length() * 1.5);
  for(int i = 0; i < s.length(); ++i){
    if(s[i] == '%'){
      char hex[3];
      memcpy(hex, s.c_str() + i + 1, 2);
      hex[2] = '\0';
      char symbol = strtol(hex, NULL, 16);
      str += symbol;
      i += 2;
    }else{
      str += s[i];
    }
  }
  return str;
}
void setup() {
  ard.begin(9600);
  Serial.begin(9600);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("Calorimeter", "C@10r1eC0unt");
  Serial.println(WiFi.softAPIP());
  resp = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\nContent-Length: ";
  resp += strlen(htmlFile);
  resp += "\r\n\r\n";
  resp += htmlFile;
  resp += "\r\n";
  server.begin();
  udp.begin(SRVR_PORT);
  Serial.println("Setup!");
  WiFi.begin();
  int tm = 0;
  while(WiFi.status() != WL_CONNECTED && tm++ < 10) delay(150);
  EEPROM.begin(512);
  char saved = EEPROM.read(0);
  if(saved == 'T'){
    Serial.println("Saved data exists");
    ip = IPAddress(EEPROM.read(1), EEPROM.read(2), EEPROM.read(3), EEPROM.read(4));
    Serial.println(ip.toString());
    hasPeer = true;
  }
  

}

void loop() {
  WiFiClient connection = server.available();
  if(connection)
  {
    int timeout = 0;
    do{
      delay(100);
      ++timeout;
    }while(!connection.available() && timeout < 40);
    if(timeout >= 40) {
      Serial.println("Tcp timeout");
      return;
    }
    String buf;
    buf.reserve(500);
    while(connection.available())
      buf += (char)connection.read();
    String Ssid, Pass;
    if(buf.indexOf("POST") != -1)
    {
      int ssid = buf.indexOf("ssid=");
      int pass = buf.indexOf("pass=");
      int ippo = buf.indexOf("ipaddr=");
      if(ssid == -1 || pass == -1 || ippo == -1){
        Serial.println("Bad tcp post");
        return;
      }
      String ipa = buf.substring(ippo + 7, buf.indexOf("\r\n", pass));
      Ssid = symbolDecode(buf.substring(ssid + 5, buf.indexOf('&', ssid)));
      Pass = symbolDecode(buf.substring(pass + 5, buf.indexOf('&', pass)));
      hasPeer = true;
      int first = ipa.indexOf('.');
      int second = ipa.indexOf('.', first + 1);
      int third = ipa.indexOf('.', second + 1);
      ip = IPAddress(atoi(ipa.substring(0, first).c_str()), atoi(ipa.substring(first + 1, second).c_str()), atoi(ipa.substring(second + 1, third).c_str()), atoi(ipa.substring(third + 1).c_str()));
      Serial.printf("ssid: %s, pass: %s ip: %s\n", Ssid.c_str(), Pass.c_str(), ipa.c_str());
      Serial.println(ip.toString());
      WiFi.begin(Ssid, Pass);
      int timeout = 0;
      while(WiFi.status() != WL_CONNECTED && timeout < 40)
      {
        delay(100);
        ++timeout;
      }
      if(WiFi.status() == WL_CONNECTED) Serial.println("Connected to AP!");
      EEPROM.write(0, 'T');
      EEPROM.write(1, ip[0]);
      EEPROM.write(2, ip[1]);
      EEPROM.write(3, ip[2]);
      EEPROM.write(4, ip[3]);
      EEPROM.commit();
      
    }
    connection.print(resp);
  }
  if(ard.available())
  {
    union{
      char buf[8];
      struct{
        int16_t erroCode;
        int16_t co2Value;
        float wndValue;
      }data;
    }rd;
    int tm = 0;
    do{
      delay(50);
    }while(ard.available() < 8 && tm++ < 40);
    for(int i = 0; i < 8; ++i)
    {
      rd.buf[i] = ard.read();
    }
    Serial.print("Co2: "); Serial.println(rd.data.co2Value);
    Serial.print("Wind: "); Serial.println(rd.data.wndValue);
    udp.beginPacket(ip, SRVR_PORT);
    udp.write(rd.buf, 8);
    udp.endPacket();
  }

}
