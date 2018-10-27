/*
 * -----------------------------
 * Developer: Yurii Alieksieiev
 * Sity: Lodz, Poland
 *    13.10.2018
 * -----------------------------
 * 
 * CODES_OF_ERROR
 * 0 - not error, all right
 * 1 - error of parsing JSON file 
 * 2 - error of type pin from JSON
 * 
 */

#include <ArduinoJson.h>
#include <Ethernet.h>
#include <dht11.h>

#define LIGTH_SENSOR A0
#define DHT_SENSOR 2

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "192.168.0.106";//My local server
dht11 DHT;

const String KEY = "3we42fi27rh";

IPAddress ip(192, 168, 0, 178);
EthernetClient client;

boolean flagForReadingJson = false;

const byte PINS_OUTPUT[] = 
        {3,4,5,6,7,8,9,10,
        11,12,13,22,23,24,25,
        26,27,28,29,30,31,32,
        33,34,35,36,37,38,39,
        40,41,42,43,44,45,46,
        47,48,49,50,51,52,53};

void setup() 
{
  for(int i = 0; i < sizeof(PINS_OUTPUT);i++)
    pinMode(PINS_OUTPUT[i],OUTPUT);

    Serial.begin(9600);
    if(Ethernet.begin(mac) == 0)
    {
      Serial.println("[!] Error of configurate Ethernat by DHCP!");
      Ethernet.begin(mac,ip);
    }
    delay(1000);
    Serial.println("Start:");

}

void loop() 
{
  String text = getJsonFromServer();
  //Serial.println(text);
  FunctionOfAction(text); 
  delay(100);
}

String getJsonFromServer()
{
  String jsonInformation;
  
  Serial.println("[*] Connecting....");
  if(client.connect(server,8080))
  {
    Serial.println("[*] Connected!");

    client.print("GET /actions?");
    client.print(getParamsForServer());
    client.println(" HTTP/1.1");
    client.println("Host: 192.168.0.106");
    client.println("Connection: close");
    client.println();
  }else
    Serial.println("[!] Error of conection!");
    
  while(client.available())
  {
    char c = client.read();
    if(c == '[')                  // I add it for reading only JSON
      flagForReadingJson = true;
      
    if(flagForReadingJson)
      jsonInformation.concat(c);
  }
  flagForReadingJson = false;

  if (!client.connected())
      client.stop();
      
    return jsonInformation;
}

String getParamsForServer()
{
  String params;
  params.concat("key=");
  params.concat(KEY);
  params.concat("&temperature=");
  params.concat(getTemperature());
  params.concat("&humidity=");
  params.concat(getHumidity());
  params.concat("&light=");
  params.concat(analogRead(LIGTH_SENSOR));
  
  return params;
}

byte FunctionOfAction(String jsonText)
{
  int sizeArray = jsonText.length() + 1;

  if(sizeArray < 5)
    return 4;
    
  Serial.println(sizeArray);
 
  char actions[sizeArray];
  jsonText.toCharArray(actions,sizeArray);
  
  StaticJsonDocument<3000> doc;
  
  DeserializationError error = deserializeJson(doc, actions);
  
  if (error) {
    String errorText = F("deserializeJson() failed: ");
    errorText.concat("#");
    errorText.concat(error.c_str());
    Serial.println(errorText);
    return 1;
  }
  
  JsonArray roots = doc.as<JsonArray>();  
  
  for(int i = 0; i < roots.size(); i++)
  {
    JsonObject root = roots[i];
    String TYPE_PIN = root["T"];
    
    if(TYPE_PIN.equals("D"))
      FunctionActionDIGITAL(root);
    else if(TYPE_PIN.equals("A"))
      FunctionActionPWM(root);
    else
      return 2;
      
  }
  return 0;
}
void FunctionActionDIGITAL(JsonObject root)
{
 String pinString = root["P"];
 int pin = pinString.toInt();
 
 String pin_state = root["S"];

 if(pin_state.equals("H"))
     digitalWrite(pin,HIGH);
 else if(pin_state.equals("L"))
     digitalWrite(pin,LOW);             
}

void FunctionActionPWM(JsonObject root)
{
 String pinString = root["P"];
 int pin = pinString.toInt();
 
 String valueString = root["V"];
 byte value = valueString.toInt();
 
 analogWrite(pin,value);
}
int getTemperature()
{
  int chk = DHT.read(DHT_SENSOR);
  return DHT.temperature;
}
int getHumidity()
{
  int chk = DHT.read(DHT_SENSOR);
  return DHT.humidity;
}

