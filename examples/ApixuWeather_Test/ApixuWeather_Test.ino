// Sketch for ESP32 or ESP8266 to fetch the Weather Forecast from Apixu
// an example from the library here:
// https://github.com/Bodmer/ApixuWeather

// Sign up for a key and read API configuration info here:
// http://api.apixu.com

// Choose the WiFi library to load depending on the selected processor
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else // ESP32
  #include <WiFi.h>
#endif

#include <JSON_Decoder.h> // Load library from: https://github.com/Bodmer/JSON_Decoder

#include <ApixuWeather.h> // Load library from: https://github.com/Bodmer/ApixuWeather

#include <Time.h> // Standard Arduino Time library: https://github.com/PaulStoffregen/Time

// =====================================================
// ========= User configured stuff starts here =========
// Further configuration settings can be found in the
// ApixuWeather library "User_Setup.h" file

#define TIME_OFFSET 0UL * 3600UL // UTC + 0 hour

// Change to suit your WiFi router
#define SSID "Your_SSID"
#define SSID_PASSWORD "Your_password"

// Apixu API Details, replace x's with your API key
String api_key = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; // Obtain this from your Apixu account

// Set location to nearest city, zip code, post code, latitude & longitude etc
String loc =  "New%20York"; // No spaces allowed, use %20 for spaces
/* The location string could be following:

      Latitude and Longitude (Decimal degree) e.g: "48.8567,2.3508"
      city name e.g.: "Paris" or "New%20York"
      US zip e.g.: "10001"
      UK postcode e.g: "SW1"
      Canada postal code e.g: "G2J"
      metar:<metar code> e.g: "metar:EGLL"
      iata:<3 digit airport code> e.g: "iata:DXB"
      auto:ip IP lookup e.g: "auto:ip"
      IP address (IPv4 and IPv6 supported) e.g: "100.0.0.1"
*/
// =========  User configured stuff ends here  =========
// =====================================================

Apixu_Weather apw; // Weather forecast library instance

/***************************************************************************************
**                          setup
***************************************************************************************/
void setup() { 
  Serial.begin(250000); // Fast to stop it holding up the stream

  Serial.printf("Connecting to %s\n", SSID);

  WiFi.begin(SSID, SSID_PASSWORD);
   
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected\n");
}

/***************************************************************************************
**                          loop
***************************************************************************************/
void loop() {

  apw.setMetric(true); // mm rain, deg.C, millibars (hPa) pressure, kph
  printCurrentWeather();

  delay(1000);

  apw.setMetric(false); // inches rain, deg.F, inches pressure, mph
  printCurrentWeather();

  delay(5 * 60 * 1000UL); // Every 5 minutes

}

/***************************************************************************************
**                          Send weather info to serial port
***************************************************************************************/
void printCurrentWeather()
{
  // Create the structures that hold the retrieved weather
  APW_current  *current  = new APW_current;
  APW_forecast *forecast = new APW_forecast;

  time_t time;

  Serial.print("\nRequesting weather information from Apixu.net... ");

  apw.getForecast(current, forecast, api_key, loc);

  Serial.println("Weather from Apixu\n");

  // We can use the timezone to set the offset eventually...
  // Serial.print("Timezone            : "); Serial.println(current->timezone);
  
  Serial.println("############### Current weather ###############\n");

  Serial.print("name               : "); Serial.println(current->name);
  Serial.print("region             : "); Serial.println(current->region);
  Serial.print("rcountry           : "); Serial.println(current->country);
  Serial.print("localtime_epoch    : "); Serial.print(strTime(current->localtime_epoch));
  Serial.print("last_updated_epoch : "); Serial.print(strTime(current->last_updated_epoch));
  Serial.print("text               : "); Serial.println(current->text);
  Serial.print("icon code          : "); Serial.println(apw.iconName(current->code));
  Serial.print("precip             : "); Serial.println(current->precip);
  Serial.print("temp               : "); Serial.println(current->temp);
  Serial.print("humidity           : "); Serial.println(current->humidity);
  Serial.print("pressure           : "); Serial.println(current->pressure);
  Serial.print("wind speed         : "); Serial.println(current->wind);
  Serial.print("wind dirn          : "); Serial.println(current->wind_degree);
  Serial.print("cloud              : "); Serial.println(current->cloud);

  Serial.println();

  Serial.println("############## Forecast weather ###############\n");

  // MAX_DAYS is defined in User_Setup.h within the ApixuWeather library
  for (int i = 0; i < MAX_DAYS; i++)
  {
    Serial.print("\nForecast summary   "); Serial.print(i); Serial.println();
    Serial.print("date_epoch    : "); Serial.print(strTime(forecast->date_epoch[i]));
    Serial.print("maxtemp       : "); Serial.println(forecast->maxtemp[i]);
    Serial.print("mintemp       : "); Serial.println(forecast->mintemp[i]);
    Serial.print("totalprecip   : "); Serial.println(forecast->totalprecip[i]);
    Serial.print("text          : "); Serial.println(forecast->text[i]);
    Serial.print("code          : "); Serial.println(apw.iconName(forecast->code[i]));
    Serial.print("sunrise       : "); Serial.println(forecast->sunrise[i]);
    Serial.print("sunset        : "); Serial.println(forecast->sunset[i]);

    Serial.println();
  }

  // Delete to free up space and prevent fragmentation as strings change in length
  delete current;
  delete forecast;
}

/***************************************************************************************
**                          Convert unix time to a time string
***************************************************************************************/
String strTime(time_t unixTime)
{
  unixTime += TIME_OFFSET;
  return ctime(&unixTime);
}

/***************************************************************************************
**                          Get the icon file name from the index number
***************************************************************************************/
const char* getMeteoconIcon(uint8_t index)
{
  if (index > MAX_ICON_INDEX) index = 0;
  return apw.iconName(index);
}
