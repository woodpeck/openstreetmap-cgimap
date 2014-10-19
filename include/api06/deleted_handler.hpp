#ifndef API06_DELETED_HANDLER_HPP
#define API06_DELETED_HANDLER_HPP

#include "bbox.hpp"
#include "output_formatter.hpp"
#include "handler.hpp"
#include "osm_responder.hpp"
#include <fcgiapp.h>
#include <string>

namespace api06 {

class deleted_responder
  : public osm_responder {
public:
	 deleted_responder(mime::type, bbox, data_selection &);
  ~deleted_responder();
};

class deleted_handler 
  : public handler {
public:
  deleted_handler(FCGX_Request &request);
  ~deleted_handler();
  std::string log_name() const;
  responder_ptr_t responder(data_selection &x) const;

private:
  bbox bounds;

  static bbox validate_request(FCGX_Request &request);
};

} // namespace api06

#endif /* API06_DELETED_HANDLER_HPP */
