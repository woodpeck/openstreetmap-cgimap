#ifndef DENSITY_ESTIMATOR_HPP
#define DENSITY_ESTIMATOR_HPP

#include <boost/program_options.hpp>
#include "bbox.hpp"
#include <limits>

/** 
 * estimates number of nodes based on an mmapped index file created with 
 * the osmium-based utility program in the "node-density-index" subdirectory.
 */
class density_estimator {

public:
  density_estimator(const boost::program_options::variables_map &);
  virtual ~density_estimator();

  // compute how many nodes there are likely to be
  int estimate_nodes_in_bbox(const bbox &bounds);

  // tell boost program options about our stuff
  static void add_program_options(boost::program_options::options_description & opt);

  // return configured max bbox area - used only if node index unset
  double get_max_bbox_area();

  // return configured max nodes
  int get_max_nodes();

  // return configured max estimated nodes - requests estimated to have
  // more than this will be rejected outright.
  int get_max_estimated_nodes();

private:
  uint8_t *density_index;
  int m_xsize;
  int m_ysize;

  double m_max_bbox_area;
  int m_max_nodes;
  int m_max_estimated_nodes; 

};

#endif /* DENSITY_ESTIMATOR_HPP */
