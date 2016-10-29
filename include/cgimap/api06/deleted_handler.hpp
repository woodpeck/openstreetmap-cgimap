#ifndef API06_DELETED_HANDLER_HPP
#define API06_DELETED_HANDLER_HPP

#include "cgimap/bbox.hpp"
#include "cgimap/output_formatter.hpp"
#include "cgimap/handler.hpp"
#include "cgimap/osm_current_responder.hpp"
#include "cgimap/request.hpp"
#include <fcgiapp.h>
#include <string>

namespace api06 {

class deleted_responder : public osm_current_responder {
public:
	 deleted_responder(mime::type, bbox, factory_ptr &);
  ~deleted_responder();
};

class deleted_handler 
  : public handler {
public:
  deleted_handler(request &req);
  ~deleted_handler();
  std::string log_name() const;
  responder_ptr_t responder(factory_ptr &x) const;

private:
  bbox bounds;

  static bbox validate_request(request &req);
};

} // namespace api06

#endif /* API06_DELETED_HANDLER_HPP */
