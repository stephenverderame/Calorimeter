#include <SoftwareSerial.h>
#define ESP_RX 2
#define ESP_TX 3
#define K30_RX 12
#define K30_TX 13
#define RV A0
#define TMP A1
SoftwareSerial espSerial(ESP_RX, ESP_TX);
SoftwareSerial k30Serial(K30_RX, K30_TX);
float getWindSpeed();
int16_t getCo2();
union{
  char buf[8];
  struct{
    int16_t err;
    int16_t co2;
    float wnd;
  }vars;
}data;
long lastCo2 = 0;
void setup() {
  Serial.begin(9600);
  k30Serial.begin(9600);
  espSerial.begin(9600);
  k30Serial.listen();

}

void loop() {
  data.vars.err = 0;
  if(millis() - lastCo2 >= 2000)
  {
    data.vars.co2 = getCo2();
    lastCo2 = millis();
  }
  data.vars.wnd = getWindSpeed();
  Serial.print("Co2: "); Serial.println(data.vars.co2);
  Serial.print("Wnd: "); Serial.println(data.vars.wnd);
  espSerial.write(data.buf, 8);
  delay(500);

}
float getWindSpeed()
{
  int rv = analogRead(RV);
  int tmp = analogRead(TMP);
  float wnd = rv * 0.0048828125;
  float zeroWnd = -0.0006*((float)tmp * (float)tmp) + 1.0727 * (float)tmp + 47.172;
  zeroWnd *= 0.0048828125;
  zeroWnd -= 0.2;
  float mph = pow((wnd - zeroWnd) / .2300, 2.7265);
  return mph / 2.237;
}
int16_t getCo2()
{
  unsigned char req[] = {0xFE, 0x44, 0x00, 0x08, 0x02, 0x9F, 0x25};
  unsigned char resp[7];
  memset(resp, 0, 7);
  int timeout = 0;
  k30Serial.write(req, 7);
  delay(50);
/*  while(!s.available()){
   s.write(req, 7);
   delay(50);
   if(++timeout > 100){
    return -1;
   }
  }*/
  timeout = 0;
  while(!k30Serial.available())
  {
    delay(20);
    if(++timeout > 100) return -2;
  }
  int32_t bytesRead = k30Serial.available();
  for(int i = 0; i < 7 && k30Serial.available(); ++i){
    resp[i] = k30Serial.read();
  }
  int16_t out = 0;
  out |= resp[3];
  out = out << 8;
  out |= resp[4];
  return out;
}
