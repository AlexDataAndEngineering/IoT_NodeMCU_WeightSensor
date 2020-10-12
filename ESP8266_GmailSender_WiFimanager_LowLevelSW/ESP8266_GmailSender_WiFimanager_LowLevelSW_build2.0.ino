
/*********************************************************************
 *   autor: Aleksandr Sidorov
 *   ver. 2.1. 27 - September - 2020 (test version)
 *   device: NodeMCU v.3 
 *   Low-Level Sensor
 *   GND - D3 - Test contact. Normal open contact (: Tank full = contact open; Tank empty = 
 *   GND - D2 - Normal open contact (: Tank full = contact open; Tank empty = 
 *   GND - D5 - Erase the memory settings
 *   
 *   WiFiNetwork: NG 2.4Ghz network hse
 *   Password: 3thegrove
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
int SWarray[]={1, 1, 1, 1, 1};
WiFiManager wifiManager;


// ---- SW ----------------------------------------------------------------------------------------------

int SWpin =   D2;         //SW to LowLevel sensor

int SWValue = 1;   // SW_Value = 0 if tunk full (contact close)
                   // SW_Value = 1 if tunk empty (contact open)

// ---- Reset Confi--------------------------------------------------------------------------------------

int EraseMemorypin =   D5;         // Reset config pin
int ResetValue = 0;

// ---- FlagMsg -----------------------------------------------------------------------------------------

bool FlagMsgSended = true;           //In futer version will be store in memory
bool FlagMsgTestSended = false;           //In futer version will be store in memory

long MinMsgSenderPeriod = 3600;       // The minimum time period between message [number of interagation intervals]. (production settings equvalent 30 hours) 
//long MinMsgSenderPeriod = 10;           // The minimum time period between message. (test settings)

long InteragationInterval = 30000;     // interragaition interval [msec]. (production settings) by default 30 sec.
//long InteragationInterval = 1000;        // interragaition interval [msec]. (test settings) 

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
 if(gsender->Subject(subject)->Send("woodco.data@gmail.com", "The device ngrainger@gpwood.ie (ver.3) connected.")) {
      Serial.println("The message about sucessful connection to the local network sended.");
 }
 
}

void loop(){
  unsigned long currentMillis = millis(); 
   
  if(currentMillis < previousMillis) {previousMillis = currentMillis;}  // Reset timer about ones per 50 days to avoid overflow
  
  if((currentMillis - previousMillis) > InteragationInterval) {         // The interagation timer (default value 30 sec.)
    previousMillis = currentMillis;    
    int SW_sum = 0;
    Serial.print("[");
    for (int j = 0; j < 4; j++) 
    {
      SWarray[j] = SWarray[j+1];
      //Serial.print(" j=");Serial.print(j, DEC); Serial.print(" SWarray[j]=");
      Serial.print(SWarray[j], DEC);
      Serial.print(","); 
      SW_sum = SW_sum + SWarray[j];
    }
    SWarray[4] = digitalRead(SWpin); // Read the current value of SW from SWpin
    SW_sum = SW_sum + SWarray[4];
    Serial.print(SWarray[4], DEC);   Serial.print("]  ");Serial.print("SW_sum =");   Serial.println(SW_sum, DEC);
    Serial.print ("      CounterMsgSenderPeriod =");Serial.println (CounterMsgSenderPeriod);
    
    if (SW_sum < 1) SWValue = 0;
    if (SW_sum > 4) SWValue = 1;  
    if ((!SWValue) & (CounterMsgSenderPeriod >=MinMsgSenderPeriod )){    // If tank Empty long time -> release FlagMsgSended if counter exeded (default value 30 hours) 
      FlagMsgSended = false;     
      FlagMsgTestSended = false;  
    } 
 // =============================================================================================
 // SEND MESSAGE "Low level"  if D4 ON
 // =============================================================================================
    
    if ((SWValue) & !FlagMsgSended & (CounterMsgSenderPeriod >=MinMsgSenderPeriod )) {   // If tank Full (SWValue 1) and Message was not sended 
      Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
      String subject = "From Test: Low Level warning message.";
      if(gsender->Subject(subject)->Send("woodco.data@gmail.com", "Bulk storage low-level ngrainger@gpwood.ie warning.")) {
          FlagMsgSended = true;
          CounterMsgSenderPeriod = 0;
          Serial.println("Message send.");
//          if (gsender->Subject(subject)->Send("woodco.data@gmail.com", "Bulk storage low-level warning (ver.3)to ngrainger@gpwood.ie was sended."))
//            Serial.println("Report to woodco.data@gmail.com was sended.");
      } else {
          Serial.print("Error sending message: ");
          Serial.println(gsender->getError());
          //FlagMsgSended = false;
      }   
    }  
    if (CounterMsgSenderPeriod < MinMsgSenderPeriod) CounterMsgSenderPeriod++;   
 
  }
 
 // =============================================================================================
 // RESET if D5 ON
 // =============================================================================================
       
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
 // =============================================================================================

  // Web - server for diagnostic
  //ArduinoOTA.handle();
  //server.handleClient();
  
}
