#include <api/client.h>

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <json/json.h>

namespace http = core::net::http;
namespace json = Json;
namespace net = core::net;

using namespace api;
using namespace std;

Client::Client(Config::Ptr config) :
    config_(config), cancelled_(false) {
}

void Client::get(const net::Uri::Path &path,
                 const net::Uri::QueryParameters &parameters, json::Value &root) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    net::Uri uri = net::make_uri(config_->apiroot, path, parameters);
    configuration.uri = client->uri_to_string(uri);

    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Client::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(root["error"].asString());
        }

        // Parse the JSON from the response
        json::Reader reader;
        reader.parse(response.body, root);

        // Open weather map API error code can either be a string or int
        // json::Value cod = root["cod"];
        // if ((cod.isString() && cod.asString() != "200")
        //         || (cod.isUInt() && cod.asUInt() != 200)) {
        //     throw domain_error(root["message"].asString());
        // }
    } catch (net::Error &) {
    }
}

Client::Shows Client::upcoming_episodes(const string& date, unsigned int cnt) {
    json::Value root;

    // Build a URI and get the contents
    // The fist parameter forms the path part of the URI.
    // The second parameter forms the CGI parameters.
    get( { "calendar", "shows.json", config_->apikey }, { }, root);
    // e.g. http://api.openweathermap.org/data/2.5/forecast/daily/?q=QUERY&units=metric&cnt=7

    Shows result;

    // Iterate through the weather data
    // json::Value list = root["list"];
    // cerr << root.size() << endl;
    for (json::ArrayIndex index = 0; index < root.size(); ++index) {
        json::Value item = root.get(index, json::Value());

        // Extract the first weather item
        json::Value date = item["date"];
        json::Value episodes = item["episodes"];
        // cerr << "Episodes " << episodes.size() << endl;

        EpisodeList episode_list;

        for (json::ArrayIndex i = 0; i < episodes.size(); ++i) {
            json::Value show_episode = episodes.get(i, json::Value());

            if (show_episode["show"]["title"].asString().length() < 1) {
                continue;
            }

            Show show { show_episode["show"]["title"].asString(), 
                show_episode["show"]["images"]["poster"].asString() };
            Episode episode { show_episode["episode"]["title"].asString(), 
                show_episode["episode"]["overview"].asString(),
                show_episode["episode"]["images"]["screen"].asString() };

            episode_list.push_back(std::make_pair(show, episode));
        }

        result.push_back(Day { date.asString(), episode_list });
    }
    // cerr << "Result size: " << result.size() << endl;
    return result;
}

std::deque<Client::Show> Client::trending_shows() {
    json::Value root;

    get({ "shows", "trending.json", config_->apikey }, { }, root);

    std::deque<Show> result;

    for (json::ArrayIndex index = 0; index < root.size(); ++index) {
        json::Value show = root.get(index, json::Value());

        result.push_back(Show { show["title"].asString(), show["images"]["poster"].asString() });
    }

    return result;
}

http::Request::Progress::Next Client::progress_report(
        const http::Request::Progress&) {

    return cancelled_ ?
                http::Request::Progress::Next::abort_operation :
                http::Request::Progress::Next::continue_operation;
}

void Client::cancel() {
    cancelled_ = true;
}

Config::Ptr Client::config() {
    return config_;
}

