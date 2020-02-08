#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#define RX D6 //3
#define TX D7 //1
#define SCL 5
#define SDA 4
#define HTTP_PORT 80
#define SRVR_PORT 8000
//SoftwareSerial k30(RX, TX);
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
//  k30.begin(9600);
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
  if(millis() - lastUpdate > 3000 && hasPeer)
  {
    union{
      int32_t co2;
      char data[4];
    }packet;
    packet.co2 = 0;
    packet.co2 = 2;//readCo2(k30);
    Serial.print("Co2: "); Serial.println(packet.co2);
    udp.beginPacket(ip, SRVR_PORT);
    udp.write(packet.data, 4);
    udp.endPacket();
    lastUpdate = millis();
  }

}

int32_t readCo2(SoftwareSerial & s)
{
  unsigned char req[] = {0xFE, 0x44, 0x00, 0x08, 0x02, 0x9F, 0x25};
  unsigned char resp[7];
  memset(resp, 0, 7);
  int timeout = 0;
  s.write(req, 7);
  delay(50);
/*  while(!s.available()){
   s.write(req, 7);
   delay(50);
   if(++timeout > 100){
    return -1;
   }
  }*/
  timeout = 0;
  while(!s.available())
  {
    delay(20);
    if(++timeout > 100) return -2;
  }
  int32_t bytesRead = s.available();
  for(int i = 0; i < 7 && s.available(); ++i){
    resp[i] = s.read();
  }
  Serial.print("Read: "); Serial.print(bytesRead); Serial.println(" bytes");
  int32_t out = 0;
  out |= resp[3];
  out = out << 8;
  out |= resp[4];
//  out = (int32_t)resp[3] * 0xFF + (int32_t)resp[4];
  return out;
}
/*
int16_t readCo2I2c()
{
  int16_t out = 0;
  Wire.beginTransmission(0x68);
  Wire.write(0x22);
  Wire.write(0x00);
  Wire.write(0x08);
  Wire.write(0x2A);
  int ret = Wire.endTransmission();
  if(ret) Serial.println(ret);
  delay(10);
  int bytes = Wire.requestFrom(0x68, 4);
  Serial.print("Read: "); Serial.print(bytes); Serial.println(" bytes");
  byte buf[4];
  int loaded = 0;
  for(int i = 0; i < 4 && Wire.available(); ++i)
  {
    buf[i] = Wire.read();
    ++loaded;
  }
  Serial.print(loaded); Serial.println(" bytes loaded");
  out |= buf[1] & 0xFF;
  out = out << 8;
  out |= buf[2] & 0xFF;
  byte sum = 0;
  sum += buf[0] + buf[1] + buf[2];
  if(sum != buf[3]) Serial.println("Checksum fail!");
 return out;
}
*/
