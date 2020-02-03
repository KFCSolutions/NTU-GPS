#include "earth.h"
#include "parseNMEA.h"

namespace NMEA
{

  bool isWellFormedSentence(std::string)
  {
      // Stub definition, needs implementing
      return false;
  }

  bool hasValidChecksum(std::string)
  {
      // Stub definition, needs implementing
      return false;
  }

  SentenceData extractSentenceData(std::string)
  {
      // Stub definition, needs implementing
      return {"",{}};
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
