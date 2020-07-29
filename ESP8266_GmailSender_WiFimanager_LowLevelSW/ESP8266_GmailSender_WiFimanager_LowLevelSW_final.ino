
/*********************************************************************
 *   autor: Aleksandr Sidorov
 *   ver. 1.3. 28 - July - 2020
 *   device: NodeMCU v.3 
 *   Low-Level Sensor
 *   GND - D5 - Normal open contact (: Tank full = contact open; Tank empty = 
 *   GND - D1 - Erase the memory settings
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

int SWpin =   D5;         //SW to LowLevel sensor

int SWValue ;   // SW_Value = 0 if tunk full (contact close)
                // SW_Value = 1 if tunk empty (contact open)

// ---- Reset Confi--------------------------------------------------------------------------------------

int EraseMemorypin =   D1;         // Reset config pin
int ResetValue = 0;

// ---- FlagMsg -----------------------------------------------------------------------------------------

bool FlagMsgSended = false;           //In futer version will be store in memory
bool FlagMsgTestSended = false;           //In futer version will be store in memory

long MinMsgSenderPeriod = 3600;       // The minimum time period between message [interrageationInterval].
long InteragationInterval = 30000;     // interragaition interval [msec].
long previousMillis = 0;
long CounterMsgSenderPeriod = MinMsgSenderPeriod - 5;       // The minimum time period between message [sec].

// ---- WiFiConnect -------------------------------------------------------------------------------------

uint8_t WiFiConnect()
{

    Serial.println("Connection: ESTABLISHED");
    wifiManager.autoConnect();
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void(* resetFunc) (void) = 0; // объявляем функцию reset

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
    SWValue = 0;
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
 String subject = "From Test: The device ngrainger@gpwood.ie connected";
 if(gsender->Subject(subject)->Send("woodco.data@gmail.com", "The device ngrainger@gpwood.ie connected.")) {
      Serial.println("The message about sucessful connection to the local network sended.");
 }
 
}

void loop(){
  unsigned long currentMillis = millis(); 
  SWValue = digitalRead(SWpin); 
  if(currentMillis < previousMillis) {previousMillis = currentMillis;}  // Reset timer about ones per 50 days to avoid overflow
  
  if((currentMillis - previousMillis) > InteragationInterval) {         // The timer 
    previousMillis = currentMillis;
 
    Serial.print("SW = ");    Serial.println(SWValue);


    if ((!SWValue) & (CounterMsgSenderPeriod >=MinMsgSenderPeriod )){
      FlagMsgSended = false;     
      FlagMsgTestSended = false;  
    } 
    // The message "Low level" sending
  
 //   if ((SWValue) & !FlagMsgTestSended & !(CounterMsgSenderPeriod >=MinMsgSenderPeriod ))  {
 //     Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
 //     String subject = "From Test: Test Low Level warning message.";
 //     if (gsender->Subject(subject)->Send("woodco.data@gmail.com", "Bulk storage low-level warning to ngrainger@gpwood.ie was sended.")){
 //           Serial.println("Fast report ");
 //           FlagMsgTestSended = true;
 //           delay(1000);
 //         }
 //   }
    
    
    if ((SWValue) & !FlagMsgSended & (CounterMsgSenderPeriod >=MinMsgSenderPeriod )) {   // SWValue 1 ->0
      Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
      String subject = "From Test: Low Level warning message.";
      if(gsender->Subject(subject)->Send("ngrainger@gpwood.ie", "Bulk storage low-level warning. Any problems, please call 062 74007")) {
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
            String subject = "From Test: The device ngrainger@gpwood.ie memory erased.";
            if(gsender->Subject(subject)->Send("woodco.data@gmail.com", "The device ngrainger@gpwood.ie memory erased")) {
              Serial.println("E-mail message 'The device ngrainger@gpwood.ie memory erased' sended.");
            }
         }
  wifiManager.resetSettings(); //  erase all the stored information if D5 on
  Serial.print("digitalRead(EraseMemorypin)");    
  Serial.println(!digitalRead(EraseMemorypin));
  resetFunc();
  Serial.println("Reset not happend");
  delay(1000);
  }
  // Web - server for diagnostic
  //ArduinoOTA.handle();
  //server.handleClient();
  
}
