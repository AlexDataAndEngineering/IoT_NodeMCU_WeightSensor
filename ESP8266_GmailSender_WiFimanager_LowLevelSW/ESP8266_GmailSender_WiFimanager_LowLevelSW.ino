#define USING_AXTLS
#include <ESP8266WiFi.h>
#include "Gsender.h"
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager


#pragma region Globals

uint8_t connection_state = 0;                    // Connected to WIFI or not
uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
#pragma endregion Globals

// ---- SW ----------------------------------------------------------------------------------------------

int SWpin =   D4;         //SW
int SWValue = 0;

// ---- FlagMsg -----------------------------------------------------------------------------------------

bool FlagMsgSended = false;         //In futer version will be store in memory
long MinMsgSenderPeriod = 3600;       // The minimum time period between message [sec].
long CounterMsgSenderPeriod = MinMsgSenderPeriod - 5;       // The minimum time period between message [sec].

// ---- WiFiConnect -------------------------------------------------------------------------------------

uint8_t WiFiConnect()
{
    WiFiManager wifiManager;
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
    pinMode(SWpin, INPUT_PULLUP);
    
    
    

}

void loop(){
  
  SWValue = digitalRead(SWpin);  
  Serial.print("SW = ");
  Serial.println(SWValue);

  if ((SWValue == 1) & (CounterMsgSenderPeriod >=MinMsgSenderPeriod ))
    FlagMsgSended = false;
    
  
  // Low level message sending
  
  if ((SWValue == 0) & !FlagMsgSended & (CounterMsgSenderPeriod >=MinMsgSenderPeriod )) {  
    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "Low Level warning message.";
    if(gsender->Subject(subject)->Send("ngrainger@gpwood.ie", "Bulk storage low-level warning.")) {
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
  if (CounterMsgSenderPeriod < MinMsgSenderPeriod) {
    CounterMsgSenderPeriod++;    
  }
    
  delay(1000);
}
