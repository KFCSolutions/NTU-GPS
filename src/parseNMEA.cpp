#include "earth.h"
#include "parseNMEA.h"
#include <bits/c++config.h>
#include <ctype.h>
#include <regex>
#include <locale> 
#include <cmath>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "geometry.h"
#include "earth.h"

#include <stdexcept>
std::string supported_formats[] = {"GLL", "GGA", "RMC"};

namespace NMEA
{

  bool isWellFormedSentence(std::string gpsData)
  {
    const std::locale loc;

    // Checks to see if the first 3 characters of the GPS file contain $GP
    if (gpsData.substr(0,3) != "$GP")
      return false;

    // Loops through the 3 characters after $GPS and checks if they are English alphabet characters
    std::string indent = gpsData.substr(3,3);
    for (std::string::iterator it=indent.begin(); it!=indent.end(); ++it)
      if (!std::isalpha(*it,loc))
        return false;

    // Check if the third character from the end is an astrix
    if (gpsData[gpsData.length() - 3] != '*') 
      return false;

    // Check if the last two characters are valid hex values
    if ((!isxdigit(gpsData[gpsData.length() - 2])) || !isxdigit(gpsData[gpsData.length() - 1]))
      return false;

    // Sanitize the string and remove valid locations for * and $
    gpsData.erase(gpsData.end() - 3);
    gpsData.erase(0, 1);

    // Check to see if the new string contains * and $ and if it does return false
    if ((gpsData.find('*') != std::string::npos)  || (gpsData.find('$') != std::string::npos))
      return false;

    // All checks passed
    return true;
  }

  bool hasValidChecksum(std::string gpsData)
  {
    // Get the checksum value from the string
    std::string checkSum = gpsData.substr(gpsData.length() - 2,2);
    // Remove the checksum value from the string along with the $ from the start
    gpsData.erase(gpsData.length() - 3, 3);
    gpsData.erase(0, 1);

    // XOR the string to create the checksum
    int lastNum = 0;
    for( long unsigned int i = 0; i < gpsData.length(); i++) {
      lastNum ^= int(gpsData[i]);
    }

    // Convert generated checksum to hex
    char generatedCheckSum[20];
    sprintf(generatedCheckSum, "%X", lastNum);

    // Check checksum from string and make it upper case
    std::for_each(checkSum.begin(), checkSum.end(), [](char & c) {
        c = ::toupper(c);
        });

    // Cross check checksum from file and generated checksum
    if ((generatedCheckSum !=  checkSum)) 
      return false;

    return true;
  }

  SentenceData extractSentenceData(std::string sen)
  {
    // prefixLen = length of NMEA prefix ($GP)
    // suffixLen = length of checksum suffix (*FF)
    // formatLen = length of sentance format string (GSV)
    const int prefixLen = 3;
    const int suffixLen = 3;
    const int formatLen = 3;

    // variables used for operations 
    std::string formatRet, fieldsStr;
    formatRet = sen;

    // remove prefix from formatRet & assign it to fieldsStr
    formatRet.erase(0, prefixLen); 
    fieldsStr = formatRet;

    // remove everything that is not the format string from formatRet
    formatRet.erase(formatLen, std::string::npos); 
    std::size_t found = fieldsStr.find(',');
    if(found == std::string::npos){
      // empty field set
      SentenceData ret;
      ret.first = formatRet;
      ret.second = {};
      return ret;
    }

    //remove the format string and the suffix from fieldsStr
    fieldsStr.erase(0, formatLen + 1);
    fieldsStr.erase(fieldsStr.length() - suffixLen);

    std::vector<std::string> fieldsRet;

    // iterate fieldsStr to populate fieldsRet
    std::stringstream s_stream(fieldsStr);
    while(s_stream.good()) {
      std::string substr;
      getline(s_stream, substr, ',');
      fieldsRet.push_back(substr);
    }

    // use formatRet and fieldsRet to init the return value
    SentenceData ret;


    ret.first = formatRet;
    ret.second = fieldsRet;
    return ret;
  }

  float convert_NMEA(std::string strData) {
    int DD = std::stof(strData) / 100;
    float SS = std::stof(strData) - DD * 100;
    SS = DD + (SS / 60);
    return SS;
  }

  GPS::Position positionFromSentenceData(SentenceData senData)
  {
    float latitude = 0;
    float longitude = 0;

    if(senData.first == "GLL"){
          latitude = convert_NMEA(senData.second[0]);
          longitude = convert_NMEA(senData.second[2]);
          if (senData.second[3] == "W")
            longitude = -fabs(longitude);
          if (senData.second[1] == "S") 
            latitude = -fabs(latitude);
          return GPS::Position(latitude,longitude,0);  
    }else if(senData.first == "RMC"){
          latitude = convert_NMEA(senData.second[2]);
          longitude = convert_NMEA(senData.second[4]);
          if (senData.second[5] == "W")
            longitude = -fabs(longitude);
          if (senData.second[3] == "S") 
            latitude = -fabs(latitude);
          return GPS::Position(latitude,longitude,0);  
    }else if(senData.first == "GGA"){
          latitude = convert_NMEA(senData.second[1]);
          longitude = convert_NMEA(senData.second[3]); 
          if (senData.second[4] == "W")
            longitude = -fabs(longitude);
          if (senData.second[2] == "S") 
            latitude = -fabs(latitude);
          return GPS::Position(latitude,longitude,std::stof(senData.second[8]));  
    } else
      throw std::invalid_argument("Invalid syntax.");
  }

  Route routeFromLog(std::istream & fs)
  {
    Route ret;
    for(std::string line; getline(fs, line);){
      if(!isWellFormedSentence(line)){
        // ignore if not valid sentence
        continue;
      }
      if(!hasValidChecksum(line)){
        // ignore if checksum not valid
        continue;
      }
      SentenceData data = extractSentenceData(line);

      if(std::find(std::begin(supported_formats), std::end(supported_formats), data.first) == std::end(supported_formats)){
        // ignore if format not in supported formats
        continue;
      }
      if(data.second.empty()){
        // ignore if empty data
        continue;
      }
      ret.push_back(positionFromSentenceData(data));
    }
    return ret;
  }
}
