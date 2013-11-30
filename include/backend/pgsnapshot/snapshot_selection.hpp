#ifndef PGSNAPSHOT_SELECTION_HPP
#define PGSNAPSHOT_SELECTION_HPP

#include "data_selection.hpp"
#include "density_estimator.hpp"
#include <pqxx/pqxx>
#include <boost/program_options.hpp>

/**
 * a selection which operates against a writeable (i.e: non read-only
 * slave) pgsnapshot PostgreSQL database, imported with osmosis
 */
class snapshot_selection
	: public data_selection {
public:
	 snapshot_selection(pqxx::connection &conn, density_estimator &estimator);
	 ~snapshot_selection();

	 void write_nodes(output_formatter &formatter);
	 void write_ways(output_formatter &formatter);
	 void write_relations(output_formatter &formatter);

	 visibility_t check_node_visibility(osm_id_t id);
	 visibility_t check_way_visibility(osm_id_t id);
	 visibility_t check_relation_visibility(osm_id_t id);
   void check_bbox_data_size(const bbox &bounds);


	 int select_nodes(const std::list<osm_id_t> &);
	 int select_ways(const std::list<osm_id_t> &);
	 int select_relations(const std::list<osm_id_t> &);
	 int select_nodes_from_bbox(const bbox &bounds);
	 void select_nodes_from_relations();
	 void select_ways_from_nodes();
	 void select_ways_from_relations();
	 void select_relations_from_ways();
	 void select_nodes_from_way_nodes();
	 void select_relations_from_nodes();
	 void select_relations_from_way_nodes();
	 void select_relations_from_relations();
  void select_relations_members_of_relations();

   /**
    * abstracts the creation of transactions for the writeable
    * data selection.
    */
   class factory
      : public data_selection::factory {
   public:
      factory(const boost::program_options::variables_map &);
      virtual ~factory();
      virtual boost::shared_ptr<data_selection> make_selection();

   private:
      pqxx::connection m_connection;
      density_estimator m_density_estimator;
   };

private:

   // the transaction in which the selection takes place. although 
   // this *is* read-only, it may create temporary tables.
   pqxx::work w;

   // the estimator that tells us how many nodes to expect in an area
   density_estimator& de;
};

#endif /* PGSNAPSHOT_SELECTION_HPP */
