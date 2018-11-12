// Client library for the Apixu weather datapoint server
// https://api.apixu.com

// The API server uses http

// Created by Bodmer 10/11/2018

// See license.txt in root folder of library
// iconList[] ndex default

#define MAX_ICON_INDEX 15 // Maximum for weather icon index

#define NO_VALUE 15       // for precipType default (none)

#ifndef ApixuWeather_h
#define ApixuWeather_h

#include "User_Setup.h"

#if (MAX_DAYS < 1)
  #undef MAXDAYS
  #define MAX_DAYS 1
#endif

#if (MAX_DAYS > 7)
  #undef MAXDAYS
  #define MAX_DAYS 7
#endif

#include "Data_Point_Set.h"


/***************************************************************************************
** Description:   JSON interface class
***************************************************************************************/
class Apixu_Weather: public JsonListener {

  public:
    // Sketch calls this forecast request, it returns true if no parse errors encountered
    bool getForecast(APW_current *current, APW_forecast  *forecast,
                     String api_key, String city);

    // Called by library (or user sketch), sends a GET request to a http url
    bool parseRequest(String url); // and parses response, returns true if no parse errors

    // Convert the icon index to an icon filename e.g. "partly-cloudy"
    const char* iconName(uint8_t index);

    // Set values to be metric (true) or imperial (false)
    void setMetric(bool true_or_false);

  private:

    // Streaming parser callback functions, allow tracking and decisions
    void startDocument(); // JSON document has started, typically starts once
                          // Initialises varaibles used, e.g. sets objectLayer = 0
                          // and arrayIndex =0
    void endDocument();   // JSON document has ended, typically ends once

    void startObject();   // Called every time an Object start is detected
                          // may be called multiple times as object layers entered
                          // Used to increment objectLayer
    void endObject();     // Called every time an object ends
                          // Used to decrement objectLayer and zero arrayIndex


    void startArray();    // An array of name:value pairs entered
    void endArray();      // Array member ended

    void key(const char *key);         // The current "object" or "name" for a name:value pair
    void value(const char *value);     // String value from name:value pair e.g. "1.23" or "rain"

    void whitespace(char c);           // Whitespace character in JSON - not used

    void error( const char *message ); // Error message is sent to serial port

    // Convert the weather condition number to an icon image index
    uint8_t iconIndex(uint16_t index); 

  private: // Variables used internal to library

    uint16_t forecast_index; // index into the APW_daily structure's data arrays

    // The value storage structures are created and deleted by the sketch and
    // a pointer passed via the library getForecast() call the value() function
    // is then used to populate the structs with values
    APW_current  *current;  // pointer provided by sketch to the APW_current struct
    APW_forecast *forecast; // pointer provided by sketch to the APW_daily struct


    bool     parseOK;       // true if the parse been completed
                            // (does not mean data values gathered are good!)

    bool     metric;        // Metric units if true

    String   currentObject; // Current object e.g. "daily"
    String   data_set;      // A copy of the last object name at the head of an array
                            // short equivalent to path.
    uint16_t objectLevel;   // Object level, increments for new object, decrements at end
    String   currentKey;    // Name key of the name:value pair e.g "temperature"
    uint16_t arrayIndex;    // Array index e.g. 5 for day 5 forecast

    // Lookup table to convert  an array index to a weather icon bmp filename e.g. rain.bmp

    const char* iconList[MAX_ICON_INDEX + 1] = {"unknown", "rain", "sleet", "snow", "clear-day",
    "clear-night", "partly-cloudy-day", "partly-cloudy-night", "cloudy", "fog",
    "wind", "thunderstorm", "drizzle", "lightRain", "hail", "none" };
    /*
    {0 "unknown", 1 "rain", 2 "sleet", 3 "snow", 4 "clear-day",
    5 "clear-night", 6 "partly-cloudy-day", 7 "partly-cloudy-night", 8 "cloudy", 9 "fog",
    10 "wind", 11 "thunderstorm", 12 "drizzle", 13 "light rain", 14 "hail", 15 "none" };
    */
};

/***************************************************************************************
***************************************************************************************/
#endif
