#ifndef API_CONFIG_H_
#define API_CONFIG_H_

#include <memory>
#include <string>

namespace api {

struct Config {
    typedef std::shared_ptr<Config> Ptr;

    /*
     * The root of all API request URLs
     */
    std::string apiroot { "http://api.trakt.tv/" };
    std::string apikey { "6ac4c8cca22c71d6c538be7ebf99f498" };
    /*
     * The custom HTTP user agent string for this library
     */
    std::string user_agent { "unity-trakt-scope 0.1; (gcollura)" };
};

}

#endif /* API_CONFIG_H_ */

