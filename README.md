# IoT_NodeMCU_WeightSensor

ESP8266_GmailSender_WiFiManager_LowLevelSW
installed on CrossBerry

device: NodeMCU v.3 
 *   Low-Level Sensor
 *   GND - D5 - Normal open contact (: Tank full = contact open; Tank empty = contact close)
 *   GND - D1 - Erase the memory settings
 *   
 *   - WiFiManager allows you to connect your ESP8266 to different Access Points (AP) without having to hard-code and upload new code to your board 
 *   - Sened E-mail from woodco.data@gmail.com when Level is Low
 *   - Erase memory if connect circuit GND and D5

The Weight sensor (HX711) send data throw Wi-Fi (NodeMCU -  ESP8266) to the client e-mail and administrative e-mail to woodco.data
