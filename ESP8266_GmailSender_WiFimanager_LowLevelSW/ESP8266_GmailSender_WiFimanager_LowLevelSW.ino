
/*********************************************************************
 *   autor: Aleksandr Sidorov
 *   ver. 1.1. 16 - July - 2020
 *   device: NodeMCU v.3 
 *   Low Level Sensor
 *   GND - D4 - Open contact
 *   GND - D5 - Erase the memory settings
 *   
 *   - WiFiManager allows you to connect your ESP8266 to different Access Points (AP) without having to hard-code and upload new code to your board 
 *   - Sened E-mail from woodco.data@gmail.com when Level is Low
 *   - Erase memory if connect circuit GND and D5
 *********************************************************************/


#define USING_AXTLS
#include <ESP8266WiFi.h>
#include "Gsender.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
//#include <ArduinoOTA.h>


#pragma region Globals

uint8_t connection_state = 0;                    // Connected to WIFI or not
uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
#pragma endregion Globals

WiFiManager wifiManager;



// ---- SW ----------------------------------------------------------------------------------------------

int SWpin =   D4;         //SW
int SWValue = 0;

// ---- Reset Confi--------------------------------------------------------------------------------------

int EraseMemorypin =   D5;         // Reset config pin
int ResetValue = 0;

// ---- FlagMsg -----------------------------------------------------------------------------------------

bool FlagMsgSended = false;           //In futer version will be store in memory
long MinMsgSenderPeriod = 3600;       // The minimum time period between message [sec].
long InteragationInterval = 1000;     // interragaition interval [msec].
long previousMillis = 0;
long CounterMsgSenderPeriod = MinMsgSenderPeriod - 5;       // The minimum time period between message [sec].

//ESP8266WebServer server(80);

//const char* www_username = "admin";
//const char* www_password = "12345";

// ---- WiFiConnect -------------------------------------------------------------------------------------

uint8_t WiFiConnect()
{

    Serial.println("Connection: ESTABLISHED");
    wifiManager.autoConnect();
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Awaits()
{
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}

void setup()
{
    Serial.begin(115200);
    connection_state = WiFiConnect();
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect
     else
        wifiManager.setConfigPortalTimeout(60);
        
    pinMode(SWpin, INPUT_PULLUP);
    pinMode(EraseMemorypin, INPUT_PULLUP);

  /*  ArduinoOTA.begin();

  server.on("/", [](){
    if(!server.authenticate(www_username, www_password))
      return server.requestAuthentication();
    server.send(200, "text/plain", "Login OK");
  });
  server.begin();
  */
  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
 
 // The message about sucessful connection to the local network
  
 Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
 String subject = "The device ngrainger@gpwood.ie connected";
 if(gsender->Subject(subject)->Send("woodco.data@gmail.com", "The device ngrainger@gpwood.ie connected.")) {
      Serial.println("The message about sucessful connection to the local network sended.");
 }
 
}

void loop(){
  unsigned long currentMillis = millis(); 

  if(currentMillis < previousMillis) {previousMillis = currentMillis;}  // Reset timer about ones per 50 days
  
  if((currentMillis - previousMillis) > InteragationInterval) {
    previousMillis = currentMillis;
    SWValue = digitalRead(SWpin);  
    Serial.print("SW = ");    Serial.println(SWValue);


    if ((SWValue == 1) & (CounterMsgSenderPeriod >=MinMsgSenderPeriod ))
      FlagMsgSended = false;     
  
    // The message "Low level" sending
  
    if ((SWValue == 0) & !FlagMsgSended & (CounterMsgSenderPeriod >=MinMsgSenderPeriod )) {  
      Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
      String subject = "Low Level warning message.";
      if(gsender->Subject(subject)->Send("woodco.data@gmail.com", "Bulk storage low-level warning. Any problems, please call 062 74007")) {
          FlagMsgSended = true;
          CounterMsgSenderPeriod = 0;
          Serial.println("Message send.");
          if (gsender->Subject(subject)->Send("woodco.data@gmail.com", "Bulk storage low-level warning to ngrainger@gpwood.ie was sended."))
            Serial.println("Report to woodco.data@gmail.com was sended.");
      } else {
          Serial.print("Error sending message: ");
          Serial.println(gsender->getError());
          //FlagMsgSended = false;
      }   
    }  
    if (CounterMsgSenderPeriod < MinMsgSenderPeriod) CounterMsgSenderPeriod++;    
 
  }
       
  if (!digitalRead(EraseMemorypin))  {              // If D5 ON (Command to erase memory)
        if (WiFiConnect()) {
            Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
            String subject = "The device ngrainger@gpwood.ie memory erased.";
            if(gsender->Subject(subject)->Send("woodco.data@gmail.com", "The device ngrainger@gpwood.ie memory erased")) {
              Serial.println("E-mail message 'The device ngrainger@gpwood.ie memory erased' sended.");
            }
         }
  wifiManager.resetSettings(); //  erase all the stored information if D5 on
  Serial.print("digitalRead(EraseMemorypin)");    
  Serial.println(!digitalRead(EraseMemorypin));
  delay(1000);
  }
  // Web - server for diagnostic
  //ArduinoOTA.handle();
  //server.handleClient();
  
}
