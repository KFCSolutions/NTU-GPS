#include "earth.h"
#include "parseNMEA.h"
#include <ctype.h>
#include <locale> 
#include <sstream>
#include <iostream>
#include <algorithm>
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

  GPS::Position positionFromSentenceData(SentenceData)
  {
    // Stub definition, needs implementing
    return GPS::Earth::NorthPole;
  }

  Route routeFromLog(std::istream &)
  {
    // Stub definition, needs implementing
    return {};
  }

}
