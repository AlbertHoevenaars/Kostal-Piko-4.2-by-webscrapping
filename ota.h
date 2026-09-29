#ifndef ota_h
#define ota_h

/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com
  Arduino IDE example: Examples > Arduino OTA > BasicOTA.ino
*********/


  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

//#define ota_password "Rdejg7640$*"

void setup_ota(void);

#endif