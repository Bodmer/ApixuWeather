// The structures below are the repository for the data values extracted from the
// JSON message. The structures are popolated with the extracted data by the "value()"
// member function in the main ApixuWeather.cpp file.

// The content is zero or "" when first created.

/***************************************************************************************
** Description:   Structure for current weather
***************************************************************************************/
typedef struct APW_current {
  String   name;
  String   region;
  String   country;
  uint32_t localtime_epoch = 0;

  uint32_t last_updated_epoch = 0;
  String   text;
  uint8_t  code = 0;
  float    temp = 0;
  float    wind = 0;
  uint16_t wind_degree = 0;
  float    pressure = 0;
  float    precip = 0;
  uint8_t  humidity = 0;
  uint8_t  cloud = 0;

} APW_current;

/***************************************************************************************
** Description:   Structure for daily weather
***************************************************************************************/
typedef struct APW_forecast {

  uint32_t date_epoch[MAX_DAYS] = { 0 };
  float    maxtemp[MAX_DAYS] = { 0 };
  float    mintemp[MAX_DAYS] = { 0 };
  float    totalprecip[MAX_DAYS] = { 0 };
  String   text[MAX_DAYS];
  uint8_t  code[MAX_DAYS] = { 0 };
  String   sunrise[MAX_DAYS];
  String   sunset[MAX_DAYS];
  String   moonrise[MAX_DAYS];
  String   moonset[MAX_DAYS];

} APW_forecast;
