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

  float getDegreeConversion(std::string nmeaString) {
    int DD = std::stof(nmeaString) / 100;
    return DD + ((std::stof(nmeaString) - DD * 100) / 60);
  }

  GPS::Position convertPosition (SentenceData senData,long unsigned int size, int NS, int EW,float other = 0) {
    //Gets the degree conversion for the NMEA DATA 
    float latitude = getDegreeConversion(senData.second[NS - 1]);
    float longitude = getDegreeConversion(senData.second[EW - 1]);

    // Checks to see if the data has any invalid params
    if (senData.second.size() < size)
      throw std::invalid_argument("Missing Param");

    // Checks to see if the NMEA data is West or South and if it is invert it
    if (senData.second[EW] == "W")
      longitude = -fabs(longitude);
    if (senData.second[NS] == "S") 
      latitude = -fabs(latitude);

    // Return Pos
    return GPS::Position(latitude,longitude,other); 
  }

  GPS::Position positionFromSentenceData(SentenceData senData)
  {
    // Checks if second part of sendata is empty. Return if it is
    if (senData.second.empty())
      throw std::invalid_argument("Filed Empty");

    //Need to fix these magic numbers. Calls and retuns the correct position for all supported formats
    if(senData.first == "GLL")
      return convertPosition(senData, 5, 1, 3);
    else if(senData.first == "RMC")
      return convertPosition(senData, 11, 3, 5); 
    else if(senData.first == "GGA")
      return convertPosition(senData, 14, 2, 4, std::stof(senData.second[8]));
    else
      throw std::invalid_argument("Invalid syntax.");
  }

  Route routeFromLog(std::istream & fs)
  {
    Route ret;
    for(std::string line; getline(fs, line);){
      // ignore if not valid sentence
      if(!isWellFormedSentence(line))
        continue;
      // ignore if checksum not valid
      if(!hasValidChecksum(line))
        continue;

      SentenceData data = extractSentenceData(line);

      // Checks if GLL strings are valid with REGEX and if they are not skip it.
      if (data.first == "GLL") {
				std::regex re("\\$GPGLL,[0-9]*.[0-9]*,N,[0-9]*.[0-9]*,W,[0-9]*\\*[A-Za-z0-9]{2,}");
				if(!std::regex_match(line, re)) continue;
      }
      // Checks if RMC strings are valid with REGEX and if they are not skip it.
      else if (data.first == "RMC") {
				std::regex re("\\$GPRMC,[0-9]*.[0-9]*,[AV],[0-9]*.[0-9]*,N,[0-9]*.[0-9]*,[EW],[0-9]*.[0-9]*,[0-9]*.[0-9]*,[0-9]*,,[AW]\\*[A-Za-z0-9]{2,}");
				if(!std::regex_match(line, re)) continue;
      }
      // Checks if GGA strings are valid with REGEX and if they are not skip it.
      else if (data.first == "GGA") {
				std::regex re("\\$GPGGA,[0-9]*.[0-9]*,[0-9]*.[0-9]*,N,[0-9]*.[0-9]*,W,[0-9]*,[0-9]*,,-?[0-9]*.[0-9]*,M,,M,,\\*[A-Za-z0-9]{2,}");
				if(!std::regex_match(line, re)) continue;
      }

      // ignore if format not in supported formats
      if(std::find(std::begin(supported_formats), std::end(supported_formats), data.first) == std::end(supported_formats))
        continue;
      
      // ignore if empty data
      if(data.second.empty())
        continue;
      
      ret.push_back(positionFromSentenceData(data));
    }
    return ret;
  }
}
