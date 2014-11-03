#include <boost/algorithm/string/trim.hpp>

#include <scope/localization.h>
#include <scope/query.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>

#include <iostream>
#include <iomanip>
#include <sstream>

namespace sc = unity::scopes;
namespace alg = boost::algorithm;

using namespace std;
using namespace api;
using namespace scope;

const static string SHOW_TEMPLATE =
        R"(
{
    "schema-version": 1,
    "template": {
        "category-layout": "grid",
        "card-layout": "vertical",
        "card-size": "medium",
        "overlay": true
    },
    "components": {
        "title": "title",
        "art": {
            "field": "art"
        },
        "overlay-color": "overlay"
    }
}
        )";
/**
 * Define the layout for the forecast results
 *
 * The icon size is small, and ask for the card layout
 * itself to be horizontal. I.e. the text will be placed
 * next to the image.
 */
const static string EPISODE_TEMPLATE =
        R"(
{
    "schema-version": 1,
    "template": {
        "category-layout": "grid",
        "card-layout": "horizontal",
        "card-size": "small"
    },
    "components": {
        "title": "title",
        "art" : {
            "field": "art"
        },
        "subtitle": "subtitle"
    }
}
        )";


Query::Query(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             Config::Ptr config) :
    sc::SearchQueryBase(query, metadata), client_(config) {
}

void Query::cancelled() {
    client_.cancel();
}


void Query::run(sc::SearchReplyProxy const& reply) {
    try {
        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());

        // Trim the query string of whitespace
        string query_string = alg::trim_copy(query.query_string());

        auto shows_cat = reply->register_category("shows", _("Trending shows"),
                "", sc::CategoryRenderer(SHOW_TEMPLATE));

        std::deque<Client::Show> trending_shows;
        trending_shows = client_.trending_shows();

        for (const auto &show : trending_shows) {
            sc::CategorisedResult res(shows_cat);
            res.set_uri(show.title);
            res.set_title(show.title);
            res.set_art(show.image);

            res["overlay"] = "#88111111";

            if (!reply->push(res)) {
                return;
            }
        }

        // Client::Shows upcoming_episodes;
        // upcoming_episodes = client_.upcoming_episodes("");

        // // For each of the forecast days
        // for (const auto &day : upcoming_episodes) {

        //     // Register a category for the forecast
        //     auto cat = reply->register_category(day.date,
        //             day.date, "", sc::CategoryRenderer(EPISODE_TEMPLATE));

        //     for (const auto &episode : day.episodes) {
        //         // Create a result
        //         sc::CategorisedResult res(cat);
        //         auto show = episode.first;
        //         auto epis = episode.second;

        //         // We must have a URI
        //         res.set_uri(show.title + epis.title);

        //         // Build the description for the result
        //         res.set_title(show.title);

        //         // Set the rest of the attributes
        //         res.set_art(show.image);
        //         res["subtitle"] = epis.title;
        //         res["description"] = epis.overview;

        //         // Push the result
        //         if (!reply->push(res)) {
        //             // If we fail to push, it means the query has been cancelled.
        //             // So don't continue;
        //             cerr << "Failed to push" << endl;
        //             return;
        //         }
        //     }
        // }

    } catch (domain_error &e) {
        // Handle exceptions being thrown by the client API
        cerr << "Error " << e.what() << endl;
        reply->error(current_exception());
    }
}

