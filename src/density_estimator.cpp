#include "density_estimator.hpp"
#include "logger.hpp"
#include <sys/mman.h>
#include <limits>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using boost::format;

#define GRID_SQUARES_PER_DEGREE 50

#define DEFAULT_MAX_BBOX_AREA 0.25
#define DEFAULT_MAX_NODES 50000
#define DEFAULT_MAX_ESTIMATED_NODES 75000

density_estimator::~density_estimator() {
}

density_estimator::density_estimator(const boost::program_options::variables_map &config) {

  // read max-nodes config option
  if (config.count("max-nodes")) {
    m_max_nodes = config["max-nodes"].as<int>();
  } else {
    m_max_nodes = DEFAULT_MAX_NODES;
  }

  // read max-estimated-nodes config option
  if (config.count("max-estimated-nodes")) {
    m_max_estimated_nodes = config["max-estimated-nodes"].as<int>();
  } else {
    m_max_estimated_nodes = DEFAULT_MAX_ESTIMATED_NODES;
  }

  // read max-bbox-area config option
  if (config.count("max-bbox-area")) {
    m_max_bbox_area = config["max-bbox-area"].as<double>();
  } else {
    m_max_bbox_area = DEFAULT_MAX_BBOX_AREA;
  }

  density_index = NULL;
  m_xsize = 360 * GRID_SQUARES_PER_DEGREE;
  m_ysize = 180 * GRID_SQUARES_PER_DEGREE;

  // initialise density index from command line parameter
  if (config.count("density-index") == 0)
  {
    logger::message("Node density index disabled - config option density-index not given");
    return;
  }

  const char *filename = config["density-index"].as<const char *>();

  int fd = open(filename, O_RDONLY);
  if (fd < 0)
  {
    logger::message((format("Node density index disabled - cannot open file "
        "'%1%'") % filename).str());
    return;
  }

  void *rawdata = mmap(NULL, 8 + m_xsize* m_ysize, PROT_READ, MAP_SHARED, fd, 0);
  if (rawdata == MAP_FAILED)
  {
    logger::message((format("Node density index disabled - cannot mmap file "
        "'%1%'") % filename).str());
    close(fd);
    return;
  }

  if (memcmp(rawdata, "NDNS1000", 8))
  {
    logger::message((format("Node density index disabled - file '%1%' does "
        "not seem to be a density index file") % filename).str());
    munmap(rawdata, 8 + m_xsize* m_ysize);
    close(fd);
    return;
  }

  density_index = static_cast<uint8_t *>(rawdata);

}

int density_estimator::estimate_nodes_in_bbox(const bbox &bounds) {

  // if we don't have an index file available, return -1 
  if (!density_index) return -1;

  // else make guess
  // TODO!!
  return 0;
}

void density_estimator::add_program_options(boost::program_options::options_description& opt) {

  opt.add_options()
    ("max-nodes", boost::program_options::value<int>(), "maximum number of nodes to serve")
    ("max-estimated-nodes", boost::program_options::value<int>(), "reject request if node estimate is higher than this")
    ("max-bbox-area", boost::program_options::value<double>(), "reject request if bbox area is larger than this and node estimate not possible")
    ("density-index", boost::program_options::value<std::string>(), "use this file for node density index")
    ;
}

double density_estimator::get_max_bbox_area() { return m_max_bbox_area; }
int density_estimator::get_max_nodes() { return m_max_nodes; }
int density_estimator::get_max_estimated_nodes() { return m_max_estimated_nodes; }

