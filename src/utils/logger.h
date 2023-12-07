
#ifndef MINIMACORE_LOGGER_H
#define MINIMACORE_LOGGER_H

#include <iostream>
#include <ctime>
#include <vector>


namespace minimacore {

using std::vector;

class logger {

public:
  template<typename T>
  logger& operator<<(T message)
  {
    for (auto& stream : _streams) {
      if (stream) *stream << message;
    }
    return *this;
  }
  
  void add_stream(std::ostream& stream)
  {
    _streams.push_back(&stream);
  }
  
  explicit logger() = default;

  static inline std::string uts_timestamp() {
    std::time_t time = std::time({});
    std::string timeString{"yyyy-mm-ddThh:mm:ssZ"};
    std::strftime(timeString.data(), std::size("yyyy-mm-ddThh:mm:ssZ"), "%FT%TZ", std::gmtime(&time));
    return timeString;
  }

  static inline std::string wrapped_uts_timestamp() {
    std::time_t time = std::time({});
    std::string timeString{"yyyy-mm-ddThh:mm:ssZ"};
    std::strftime(timeString.data(), std::size("yyyy-mm-ddThh:mm:ssZ"), "%FT%TZ", std::gmtime(&time));
    return "[" + timeString + "] ";
  }
  
private:
  
  vector<std::ostream*> _streams;
};

} // minimacore

#endif //MINIMACORE_LOGGER_H
