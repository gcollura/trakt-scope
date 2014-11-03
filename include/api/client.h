#ifndef API_CLIENT_H_
#define API_CLIENT_H_

#include <api/config.h>

#include <atomic>
#include <deque>
#include <map>
#include <string>
#include <core/net/http/request.h>
#include <core/net/uri.h>

namespace Json {
class Value;
}

namespace api {

/**
 * Provide a nice way to access the HTTP API.
 *
 * We don't want our scope's code to be mixed together with HTTP and JSON handling.
 */
class Client {
public:

    struct Show {
        std::string title;
        std::string image;
    };

    struct Episode {
        std::string title;
        std::string overview;
        std::string image;
    };

    typedef std::pair<Show, Episode> ShowEpisode;
    typedef std::deque<ShowEpisode> EpisodeList;

    struct Day {
        std::string date;
        EpisodeList episodes;
    };
    /**
     * A list of weather information
     */
    typedef std::deque<Day> Shows;


    Client(Config::Ptr config);

    virtual ~Client() = default;

    /**
     * Get the weather forecast for the specified location and duration
     */
    virtual Shows upcoming_episodes(const std::string &date, unsigned int days = 7);
    virtual std::deque<Show> trending_shows();

    /**
     * Cancel any pending queries (this method can be called from a different thread)
     */
    virtual void cancel();

    virtual Config::Ptr config();

protected:
    void get(const core::net::Uri::Path &path,
             const core::net::Uri::QueryParameters &parameters,
             Json::Value &root);

    /**
     * Progress callback that allows the query to cancel pending HTTP requests.
     */
    core::net::http::Request::Progress::Next progress_report(
            const core::net::http::Request::Progress& progress);

    /**
     * Hang onto the configuration information
     */
    Config::Ptr config_;

    /**
     * Thread-safe cancelled flag
     */
    std::atomic<bool> cancelled_;
};

}

#endif // API_CLIENT_H_

