#ifndef VCU_FIRMWARE_2024_GPS_H
#define VCU_FIRMWARE_2024_GPS_H

#include <cstdint>
#include <string>
#include "usart.h"
#include "adafruit_defines.h"

#define MAXLINELENGTH 120 ///< how long are max NMEA lines to parse?
#define NMEA_MAX_SENTENCE_ID                                                   \
  20 ///< maximum length of a sentence ID name, including terminating 0
#define NMEA_MAX_SOURCE_ID                                                     \
  3 ///< maximum length of a source ID name, including terminating 0

/// type for resulting code from running check()
typedef enum {
    NMEA_BAD = 0, ///< passed none of the checks
    NMEA_HAS_DOLLAR =
    1, ///< has a dollar sign or exclamation mark in the first position
    NMEA_HAS_CHECKSUM = 2,   ///< has a valid checksum at the end
    NMEA_HAS_NAME = 4,       ///< there is a token after the $ followed by a comma
    NMEA_HAS_SOURCE = 10,    ///< has a recognized source ID
    NMEA_HAS_SENTENCE = 20,  ///< has a recognized sentence ID
    NMEA_HAS_SENTENCE_P = 40 ///< has a recognized parseable sentence ID
} nmea_check_t;

typedef enum {
    UNKNOWN = 0,
    PROBLEM = 1,
    INTERNAL = 2,
    EXTERNAL = 3
} AntennaStatus;

class Adafruit_GPS {
private:
    UART_HandleTypeDef& uart_handler;

    bool paused;

    bool received;

    const char* sources[7] = {"II", "WI", "GP", "PG",
                              "GN", "P",  "ZZZ"}; ///< valid source ids
    const char *sentences_parsed[6] = {"GGA", "GLL", "GSA", "RMC",
                                       "TOP", "ZZZ"}; ///< parseable sentence ids
    const char *sentences_known[4] = {"DBT", "HDM", "HDT",
                                      "ZZZ"}; ///< known, but not parseable

    bool check(char* nmea);
    const char* tokenOnList(char *token, const char **list);
    bool onList(char *nmea, const char **list);
    bool parseCoord(char *pStart, float *angleDegrees,
                    float *angle, int32_t *angle_fixed,
                    char *dir);
    char* parseStr(char *buff, char *p, int n);
    bool parseTime(char *p);
    bool parseFix(char *p);
    bool parseAntenna(char *p);
    bool isEmpty(char *pStart);
    uint8_t parseHex(char c);
    void newDataValue(nmea_index_t tag, float v);

public:

    uint8_t hour, minute, seconds, year, month, day;
    float latitude, longitude, geoidheight, altitude;
    float HDOP, VDOP, PDOP;
    float speed, angle;
    uint8_t satellites;
    char latitude_dir, longitude_dir;
    uint8_t fixquality, fixquality_3d;
    AntennaStatus antenna;

    float latitudeDegrees;
    float longitudeDegrees;

    int32_t latitude_fixed;
    int32_t longitude_fixed;

    bool has_fix;
    bool is_ready;
    uint16_t milliseconds;

    char last_line[128]{};

    std::string firmware_version;

    int thisCheck = 0; //<the results of the check on the current sentence
    char thisSource[NMEA_MAX_SOURCE_ID] = {0};
    char thisSentence[NMEA_MAX_SENTENCE_ID] = {0};
    char lastSource[NMEA_MAX_SOURCE_ID] = {0};
    char lastSentence[NMEA_MAX_SENTENCE_ID] = {0};



    Adafruit_GPS(UART_HandleTypeDef &hlpuart);
    virtual ~Adafruit_GPS() = default;
    int send_command(const char *cmd);
    bool newNMEAreceived() const;
    bool pause(bool p);
    char* lastNMEA();
    bool parse(char* nmea);
    char read_command();
};

typedef struct GpsData {
  float latitude;
  float longitude;
  float speed;
  float heading;
  uint64_t timeMillis;
} GpsData;

void gps_init();

void gps_periodic(GpsData* gpsData);

#endif //VCU_FIRMWARE_2024_GPS_H
