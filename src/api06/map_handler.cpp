#include "api06/map_handler.hpp"
#include "http.hpp"
#include "request_helpers.hpp"
#include "logger.hpp"
#include <boost/format.hpp>
#include <map>

using boost::format;
using std::string;
using std::auto_ptr;
using std::map;

#define MAX_AREA 0.25
#define MAX_NODES 50000

namespace api06 {

map_responder::map_responder(mime::type mt, bbox b, data_selection &x)
  : osm_responder(mt, x, boost::optional<bbox>(b)) {

  // pre-flight request with the node density index. This will throw an 
  // exception if there are too many (expected) nodes in the bounding box,
  // or in the absence of a working node density estimation, fall back on
  // checking the bbox size.
  sel.check_bbox_data_size(b);

  // create temporary tables of nodes, ways and relations which
  // are in or used by elements in the bbox
  // the maximum number of nodes will now be enforced in the data_selection
  // object and an exception thrown there.
  int num_nodes = sel.select_nodes_from_bbox(b);

  // Short-circuit empty areas
  if (num_nodes > 0) {
    sel.select_ways_from_nodes();
    sel.select_nodes_from_way_nodes();
    sel.select_relations_from_ways();
    sel.select_relations_from_nodes();
    sel.select_relations_from_relations();
  }

  // map calls typically have a Content-Disposition header saying that
  // what's coming back is an attachment.
  add_response_header("Content-Disposition: attachment; filename=\"map.osm\"");
}

map_responder::~map_responder() {
}

map_handler::map_handler(request &req) 
  : bounds(validate_request(req)) {
}

map_handler::~map_handler() {
}

string
map_handler::log_name() const {
  return (boost::format("map(%1%,%2%,%3%,%4%)") % bounds.minlon % bounds.minlat % bounds.maxlon % bounds.maxlat).str();
}

responder_ptr_t
map_handler::responder(data_selection &x) const {
  return responder_ptr_t(new map_responder(mime_type, bounds, x));
}

/**
 * Validates an FCGI request, returning the valid bounding box or 
 * throwing an error if there was no valid bounding box.
 */
bbox
map_handler::validate_request(request &req) {
  string decoded = http::urldecode(get_query_string(req));
  const map<string, string> params = http::parse_params(decoded);
  map<string, string>::const_iterator itr = params.find("bbox");

  bbox bounds;
  if ((itr == params.end()) ||
      !bounds.parse(itr->second)) {
    throw http::bad_request("The parameter bbox is required, and must be "
			    "of the form min_lon,min_lat,max_lon,max_lat.");
  }

  // check that the bounding box is within acceptable limits. these
  // limits taken straight from the ruby map implementation.
  if (!bounds.valid()) {
    throw http::bad_request("The latitudes must be between -90 and 90, "
			    "longitudes between -180 and 180 and the "
			    "minima must be less than the maxima.");
  }

  return bounds;
}

} // namespace api06
