// Client library for the Apixu weather datapoint server
// https://api.apixu.com

// Created by Bodmer 10/11/2018

// See license.txt in root folder of library

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

#include <WiFiClient.h>

// The streaming parser to use is not the Arduino IDE library manager default,
// but this one which is slightly different and renamed to avoid conflicts:
// https://github.com/Bodmer/JSON_Decoder


#include <JSON_Listener.h>
#include <JSON_Decoder.h>

#include "ApixuWeather.h"


/***************************************************************************************
** Function name:           getForecast
** Description:             Setup the weather forecast request from api.apixu.com
***************************************************************************************/
// The structures etc are created by the sketch and passed to this function.
// Pass a nullptr for current, hourly or forecast pointers to exclude in response.
bool Apixu_Weather::getForecast(APW_current *current, APW_forecast *forecast,
                             String api_key, String city)
{
  data_set = "";
  forecast_index = 0;

  // Local copies of structure pointers, the structures are filled during parsing
  this->current  = current;
  this->forecast = forecast;

  // Fetch the current and the forecast
  String url = "http://api.apixu.com/v1/forecast.json?key=" + api_key + "&q=" + city + "&days=" + (String)MAX_DAYS;

  // Send GET request and feed the parser
  bool result = parseRequest(url);

  // Null out pointers to prevent crashes
  this->current  = nullptr;
  this->forecast = nullptr;

  return result;
}

#ifdef ESP32 // The ESP32 and ESP8266 have different and evolving client library
             // behaviours so there are two versions of parseRequest

/***************************************************************************************
** Function name:           parseRequest (for ESP32)
** Description:             Fetches the JSON message and feeds to the parser
***************************************************************************************/
bool Apixu_Weather::parseRequest(String url) {

  uint32_t dt = millis();

  WiFiClient client;

  JSON_Decoder parser;
  parser.setListener(this);

  const char*  host = "api.apixu.com";

  if (!client.connect(host, 80))
  {
    Serial.println("Connection failed.");
    return false;
  }

  uint32_t timeout = millis();
  char c = 0;
  int ccount = 0;
  uint32_t readCount = 0;
  parseOK = false;

  // Send GET request
  Serial.println("\nSending GET request to api.apixu.com...");
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  Serial.println("\nParsing JSON");

  // Wait for a response to arrive
  while ( client.available() == 0 )
  {
    if ((millis() - timeout) > 4000UL)
    {
      parser.reset();
      client.stop();
      return false;
    }
    yield();
  }

  // Parse the JSON data, the JSON_Decoder drops header characters until first '{'
  // Using client.connected() here can make the ESP32 WiFiClient library hang
  while(client.available() > 0)
  {
    c = client.read();
    parser.parse(c);
#ifdef SHOW_JSON
    if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
    Serial.print(c); if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
#endif

    if ((millis() - timeout) > 8000UL)
    {
      //Serial.println ("JSON parse loop timeout");
      parser.reset();
      client.stop();
      return false;
    }
    yield();
  }

  Serial.println("");
  Serial.print("Done in "); Serial.print(millis()-dt); Serial.println(" ms\n");

  parser.reset();

  client.stop();

  // A message has been parsed but the datapoint correctness is unknown
  return parseOK;
}

#else // ESP8266 version

/***************************************************************************************
** Function name:           parseRequest (for ESP8266)
** Description:             Fetches the JSON message and feeds to the parser
***************************************************************************************/
bool Apixu_Weather::parseRequest(String url) {

  uint32_t dt = millis();

  WiFiClient client;

  JSON_Decoder parser;
  parser.setListener(this);

  const char*  host = "api.apixu.com";

  if (!client.connect(host, 80))
  {
    Serial.println("Connection failed.");
    return false;
  }


  uint32_t timeout = millis();
  char c = 0;
  int ccount = 0;

  parseOK = false;

  // Send GET request
  Serial.println("Sending GET request to api.apixu.com...");
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  Serial.println("Parsing JSON");
  
  // Parse the JSON data, the JSON_Decoder drops header characters until first '{'
  // OR used because ESP8266 WiFiClientlibrary can disconnect when buffer is not empty
  while ( (client.available() > 0 ) || client.connected())
  {
    while ( client.available() )
    {
      c = client.read();
      parser.parse(c);
  #ifdef SHOW_JSON
      if (c == '{' || c == '[' || c == '}' || c == ']') Serial.println();
      Serial.print(c); if (ccount++ > 100 && c == ',') {ccount = 0; Serial.println();}
  #endif
      yield();
    }

    if ((millis() - timeout) > 8000UL)
    {
      Serial.println ("JSON client timeout");
      parser.reset();
      client.stop();
      return false;
    }
    yield();
  }

  Serial.println("");
  Serial.print("Done in "); Serial.print(millis()-dt); Serial.println(" ms\n");

  parser.reset();

  client.stop();
  
  // A message has been parsed without error but the datapoint correctness is unknown
  return parseOK;
}

#endif // ESP32 or ESP8266 parseRequest

/***************************************************************************************
** Function name:           key etc
** Description:             These functions are called while parsing the JSON message
***************************************************************************************/
void Apixu_Weather::key(const char *key) {

  currentKey = key;

#ifdef SHOW_CALLBACK
  Serial.print("<<< Key <<<\n");
#endif
}

void Apixu_Weather::startDocument() {

  currentObject = currentKey = "";
  objectLevel = 0;
  arrayIndex = 0;
  parseOK = true;

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start document >>>");
#endif
}

void Apixu_Weather::endDocument() {

  currentObject = currentKey = "";
  objectLevel = 0;
  arrayIndex = 0;

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End document <<<");
#endif
}

void Apixu_Weather::startObject() {

  if (currentKey == "location") {
    data_set = "location";
  }

  if (currentKey == "current") {
    data_set = "current";
  }

  if (currentKey == "forecast") {
    data_set = "forecast";
  }

  objectLevel++;

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Start object level:" + (String) objectLevel + " index:" + (String) arrayIndex +" >>>");
#endif
}

void Apixu_Weather::endObject() {

  currentObject = "";
  objectLevel--;

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End object <<<");
#endif
}

void Apixu_Weather::startArray() {

  arrayIndex  = 0;

#ifdef SHOW_CALLBACK
  Serial.print("\n>>> Array index " + (String) arrayIndex +" >>>");
#endif
}

void Apixu_Weather::endArray() {

  arrayIndex  = 0;

#ifdef SHOW_CALLBACK
  Serial.print("\n<<< End array <<<");
#endif
}

void Apixu_Weather::whitespace(char c) {
}

void Apixu_Weather::error( const char *message ) {
  Serial.print("\nParse error message: ");
  Serial.print(message);
  parseOK = false;
}

/***************************************************************************************
** Function name:           iconIndex
** Description:             Convert the weather condition code to an icon array index
***************************************************************************************/
uint8_t Apixu_Weather::iconIndex(uint16_t code)
{
  // 48 weather condition codes are listed on Apixu website
  if ( code == 1063 || code == 1186 || code == 1189 || code == 1195 ) return 1;
  if ( code == 1240 || code == 1243 || code == 1246 || code == 1249 ) return 1;
  if ( code == 1192 ) return 1;

  if ( code == 1069 || code == 1072 || code == 1168 || code == 1171 ) return 2;
  if ( code == 1198 || code == 1201 || code == 1204 || code == 1207 ) return 2;
  if ( code == 1252 ) return 2;

  if ( code == 1066 || code == 1114 || code == 1117 || code == 1210 ) return 3;
  if ( code == 1213 || code == 1216 || code == 1219 || code == 1222 ) return 3;
  if ( code == 1225 || code == 1255 || code == 1258 || code == 1282 ) return 3;

  if ( code == 1000 ) return 4;

  if ( code == 1003 ) return 6;

  if ( code == 1006 || code == 1009 ) return 8;

  if ( code == 1030 || code == 1135 || code == 1147 ) return 9;

  if ( code == 1087 || code == 1273 || code == 1276 || code == 1279 ) return 11;

  if ( code == 1150 || code == 1153 ) return 12;

  if ( code == 1180 || code == 1183 ) return 13;

  if ( code == 1261 || code == 1264 ) return 14;

  if ( code == 1237 ) return 15;

  return NO_VALUE;
}

/***************************************************************************************
** Function name:           iconFilename
** Description:             Convert the icon array index to an icon filename
***************************************************************************************/
const char* Apixu_Weather::iconName(uint8_t index)
{
/*
  {0 "unknown", 1 "rain", 2 "sleet", 3 "snow", 4 "clear-day",  5 "clear-night",
   6 "partly-cloudy-day", 7 "partly-cloudy-night", 8 "cloudy", 9 "fog",
  10 "wind", 11 "thunderstorm", 12 "drizzle", 13 "light rain", 14 "hail", 15 "none" };
*/

  return iconList[index];
}

/***************************************************************************************
** Function name:           metric
** Description:             Set the metric or imperial units
***************************************************************************************/
void Apixu_Weather::setMetric(bool m)
{
  metric = m;
}


/***************************************************************************************
** Function name:           value (full data set)
** Description:             Stores the parsed data in the structures for sketch access
***************************************************************************************/
 // Nested "if" with "return" reduces comparison count for each key
 
void Apixu_Weather::value(const char *val) {

  String value = val;

  if (data_set == "location") {
    // Using the APW_current struct rather than create one for location
    if (currentKey == "name") current->name = value;
    else
    if (currentKey == "region") current->region = value;
    else
    if (currentKey == "country") current->country = value;
    else
    if (currentKey == "localtime_epoch") current->localtime_epoch = (uint32_t)value.toInt();
    return;
  }

  if (data_set == "current") {
    if (currentKey == "last_updated_epoch") current->last_updated_epoch = (uint32_t)value.toInt();
    else
    if (currentKey == "temp_c"  && metric) current->temp = value.toFloat();
    else
    if (currentKey == "temp_f"  && !metric) current->temp = value.toFloat();
    else
    if (currentKey == "text") current->text = value;
    else
    if (currentKey == "code") current->code =  iconIndex( (uint16_t)value.toInt() );
    else
    if (currentKey == "wind_mph" && !metric) current->wind = value.toFloat();
    else
    if (currentKey == "wind_kph" && metric) current->wind = value.toFloat();
    else
    if (currentKey == "wind_degree") current->wind_degree = (uint16_t)value.toInt();
    else
    if (currentKey == "pressure_mb" && metric) current->pressure = value.toFloat();
    else
    if (currentKey == "pressure_in" && !metric) current->pressure = value.toFloat();
    else
    if (currentKey == "precip_mm" && metric) current->precip = value.toFloat();
    else
    if (currentKey == "precip_in" && !metric) current->precip = value.toFloat();
    else
    if (currentKey == "humidity") current->humidity = (uint8_t)value.toInt();
    else
    if (currentKey == "cloud") current->cloud = (uint8_t)value.toInt();

    return;
  }

  if (data_set == "forecast") {
    if (currentKey == "date_epoch") forecast->date_epoch[arrayIndex] = (uint32_t)value.toInt();
    else
    if (currentKey == "maxtemp_c"  && metric) forecast->maxtemp[arrayIndex] = value.toFloat();
    else
    if (currentKey == "maxtemp_f"  && !metric) forecast->maxtemp[arrayIndex] = value.toFloat();
    else
    if (currentKey == "mintemp_c"  && metric) forecast->mintemp[arrayIndex] = value.toFloat();
    else
    if (currentKey == "mintemp_f"  && !metric) forecast->mintemp[arrayIndex] = value.toFloat();
    else
    if (currentKey == "totalprecip_mm"  && metric) forecast->totalprecip[arrayIndex] = value.toFloat();
    else
    if (currentKey == "totalprecip_in"  && !metric) forecast->totalprecip[arrayIndex] = value.toFloat();
    else
    if (currentKey == "text") forecast->text[arrayIndex] = value;
    else
    if (currentKey == "code") forecast->code[arrayIndex] = iconIndex( (uint16_t)value.toInt() );
    else
    // These are in the forecast/astro object but have unique names, so no need to use the object name
    if (currentKey == "sunrise") forecast->sunrise[arrayIndex] = value;
    else
    if (currentKey == "sunset") forecast->sunset[arrayIndex] = value;
    else
    if (currentKey == "moonrise") forecast->moonrise[arrayIndex] = value;
    else
    if (currentKey == "moonset")
    {
      forecast->moonset[arrayIndex] = value;
      arrayIndex++; // Using last item in JSON list to increment array index
    }
    return;
  }
}
