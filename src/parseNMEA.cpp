#include "earth.h"
#include "parseNMEA.h"

#include <sstream>

namespace NMEA
{

  bool isWellFormedSentence(std::string)
  {
    // Stub definition, needs implementing#
    // Jamie Will Do this
    return false;
  }

  bool hasValidChecksum(std::string)
  {
    // Stub definition, needs implementing
    // Jamie Will Do this
    return false;
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
