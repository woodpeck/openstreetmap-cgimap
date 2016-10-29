#include <iostream>
#include <stdexcept>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

#include <sys/time.h>
#include <stdio.h>

#include "cgimap/config.hpp"
#include "cgimap/time.hpp"
#include "cgimap/oauth.hpp"
#include "cgimap/rate_limiter.hpp"
#include "cgimap/routes.hpp"
#include "cgimap/process_request.hpp"

#include "test_formatter.hpp"
#include "test_database.hpp"
#include "test_request.hpp"

namespace {

template <typename T>
void assert_equal(const T& a, const T&b, const std::string &message) {
  if (a != b) {
    throw std::runtime_error(
      (boost::format(
        "Expecting %1% to be equal, but %2% != %3%")
       % message % a % b).str());
  }
}

void test_historic_elements(test_database &tdb) {
  tdb.run_sql(
    "INSERT INTO users (id, email, pass_crypt, creation_time, display_name, data_public) "
    "VALUES "
    "  (1, 'user_1@example.com', '', '2013-11-14T02:10:00Z', 'user_1', true); "

    "INSERT INTO changesets (id, user_id, created_at, closed_at) "
    "VALUES "
    "  (2, 1, '2013-11-14T02:10:00Z', '2013-11-14T03:10:00Z'); "

    "INSERT INTO current_nodes (id, latitude, longitude, changeset_id, "
    "                           visible, \"timestamp\", tile, version) "
    "VALUES "
    "  (3, 0, 0, 2, false, '2015-03-02T18:27:00Z', 3221225472, 2); "

    "INSERT INTO nodes (node_id, latitude, longitude, changeset_id, visible, "
    "                   \"timestamp\", tile, version, redaction_id) "
    "VALUES "
    "  (3, 0, 0, 2, true,  '2015-03-02T18:27:00Z', 3221225472, 1, NULL), "
    "  (3, 0, 0, 2, false, '2015-03-02T18:27:00Z', 3221225472, 2, NULL); "
    "");
  boost::shared_ptr<data_selection> sel = tdb.get_data_selection();

  assert_equal<bool>(
    sel->supports_historical_versions(), true,
    "data selection supports historical versions");

  std::vector<osm_edition_t> editions;
  editions.push_back(std::make_pair(osm_nwr_id_t(3), osm_version_t(1)));
  editions.push_back(std::make_pair(osm_nwr_id_t(3), osm_version_t(2)));

  assert_equal<int>(
    sel->select_historical_nodes(editions), 2,
    "number of nodes selected");

  test_formatter f;
  sel->write_nodes(f);
  assert_equal<size_t>(f.m_nodes.size(), 2, "number of nodes written");

  assert_equal<test_formatter::node_t>(
    test_formatter::node_t(
      element_info(3, 1, 2, "2015-03-02T18:27:00Z", 1, std::string("user_1"), true),
      0.0, 0.0,
      tags_t()
      ),
    f.m_nodes[0], "first node written");
  assert_equal<test_formatter::node_t>(
    test_formatter::node_t(
      element_info(3, 2, 2, "2015-03-02T18:27:00Z", 1, std::string("user_1"), false),
      0.0, 0.0,
      tags_t()
      ),
    f.m_nodes[1], "second node written");
}

void test_historic_dup(test_database &tdb) {
  tdb.run_sql(
    "INSERT INTO users (id, email, pass_crypt, creation_time, display_name, data_public) "
    "VALUES "
    "  (1, 'user_1@example.com', '', '2013-11-14T02:10:00Z', 'user_1', true); "

    "INSERT INTO changesets (id, user_id, created_at, closed_at) "
    "VALUES "
    "  (2, 1, '2013-11-14T02:10:00Z', '2013-11-14T03:10:00Z'); "

    "INSERT INTO current_nodes (id, latitude, longitude, changeset_id, "
    "                           visible, \"timestamp\", tile, version) "
    "VALUES "
    "  (3, 0, 0, 2, false, '2015-03-02T18:27:00Z', 3221225472, 2); "

    "INSERT INTO nodes (node_id, latitude, longitude, changeset_id, visible, "
    "                   \"timestamp\", tile, version, redaction_id) "
    "VALUES "
    "  (3, 0, 0, 2, true,  '2015-03-02T18:27:00Z', 3221225472, 1, NULL), "
    "  (3, 0, 0, 2, false, '2015-03-02T18:27:00Z', 3221225472, 2, NULL); "
    "");
  boost::shared_ptr<data_selection> sel = tdb.get_data_selection();

  assert_equal<bool>(
    sel->supports_historical_versions(), true,
    "data selection supports historical versions");

  std::vector<osm_edition_t> editions;
  editions.push_back(std::make_pair(osm_nwr_id_t(3), osm_version_t(2)));
  assert_equal<int>(
    sel->select_historical_nodes(editions), 1,
    "number of historic nodes selected");

  std::vector<osm_nwr_id_t> ids;
  ids.push_back(3);
  assert_equal<int>(
    sel->select_nodes(ids), 1,
    "number of current nodes selected");

  test_formatter f;
  sel->write_nodes(f);
  assert_equal<size_t>(f.m_nodes.size(), 1, "number of nodes written");

  assert_equal<test_formatter::node_t>(
    test_formatter::node_t(
      element_info(3, 2, 2, "2015-03-02T18:27:00Z", 1, std::string("user_1"), false),
      0.0, 0.0,
      tags_t()
      ),
    f.m_nodes[0], "node written");
}

void test_historic_dup_way(test_database &tdb) {
  tdb.run_sql(
    "INSERT INTO users (id, email, pass_crypt, creation_time, display_name, data_public) "
    "VALUES "
    "  (1, 'user_1@example.com', '', '2013-11-14T02:10:00Z', 'user_1', true); "

    "INSERT INTO changesets (id, user_id, created_at, closed_at) "
    "VALUES "
    "  (3, 1, '2013-11-14T02:10:00Z', '2013-11-14T03:10:00Z'); "

    "INSERT INTO current_nodes (id, latitude, longitude, changeset_id, "
    "                           visible, \"timestamp\", tile, version) "
    "VALUES "
    "  (3, 0, 0, 3, false, '2015-03-02T18:27:00Z', 3221225472, 2); "

    "INSERT INTO current_ways (id, changeset_id, \"timestamp\", visible, version) "
    "VALUES "
    "  (1, 3, '2016-09-06T19:55:00Z', true, 2); "

    "INSERT INTO current_way_nodes (way_id, node_id, sequence_id) "
    "VALUES "
    "  (1, 3, 1); "

    "INSERT INTO ways (way_id, changeset_id, \"timestamp\", visible, "
    "                  version, redaction_id) "
    "VALUES "
    "  (1, 3, '2016-09-06T19:55:00Z', true, 2, NULL), "
    "  (1, 3, '2016-09-06T19:54:00Z', true, 1, NULL); "

    "INSERT INTO way_nodes (way_id, version, node_id, sequence_id) "
    "VALUES "
    "  (1, 2, 3, 1), "
    "  (1, 1, 3, 1), "
    "  (1, 1, 2, 2); "
    "");
  boost::shared_ptr<data_selection> sel = tdb.get_data_selection();

  assert_equal<bool>(
    sel->supports_historical_versions(), true,
    "data selection supports historical versions");

  std::vector<osm_edition_t> editions;
  editions.push_back(std::make_pair(osm_nwr_id_t(1), osm_version_t(2)));
  assert_equal<int>(
    sel->select_historical_ways(editions), 1,
    "number of historic ways selected");

  std::vector<osm_nwr_id_t> ids;
  ids.push_back(1);
  assert_equal<int>(
    sel->select_ways(ids), 1,
    "number of current ways selected");

  test_formatter f;
  sel->write_ways(f);
  assert_equal<size_t>(f.m_ways.size(), 1, "number of ways written");

  nodes_t way1_nds;
  way1_nds.push_back(3);

  assert_equal<test_formatter::way_t>(
    test_formatter::way_t(
      element_info(1, 2, 3, "2016-09-06T19:55:00Z", 1, std::string("user_1"), true),
      way1_nds,
      tags_t()
      ),
    f.m_ways[0], "way written");
}

void test_historic_dup_relation(test_database &tdb) {
  tdb.run_sql(
    "INSERT INTO users (id, email, pass_crypt, creation_time, display_name, data_public) "
    "VALUES "
    "  (1, 'user_1@example.com', '', '2013-11-14T02:10:00Z', 'user_1', true); "

    "INSERT INTO changesets (id, user_id, created_at, closed_at) "
    "VALUES "
    "  (3, 1, '2013-11-14T02:10:00Z', '2013-11-14T03:10:00Z'); "

    "INSERT INTO current_nodes (id, latitude, longitude, changeset_id, "
    "                           visible, \"timestamp\", tile, version) "
    "VALUES "
    "  (3, 0, 0, 3, false, '2015-03-02T18:27:00Z', 3221225472, 2); "

    "INSERT INTO current_relations (id, changeset_id, \"timestamp\", "
    "                               visible, version) "
    "VALUES "
    "  (1, 3, '2016-09-19T18:49:00Z', true, 2); "

    "INSERT INTO current_relation_members ("
        "relation_id, member_type, member_id, member_role, sequence_id) "
    "VALUES "
    "  (1, 'Node', 3, 'foo', 1); "

    "INSERT INTO relations (relation_id, changeset_id, \"timestamp\", "
    "                       visible, version, redaction_id) "
    "VALUES "
    "  (1, 3, '2016-09-19T18:49:00Z', true, 2, NULL), "
    "  (1, 3, '2016-09-19T18:48:00Z', true, 1, NULL); "

    "INSERT INTO relation_members (relation_id, member_type, member_id, "
    "                              member_role, sequence_id, version) "
    "VALUES "
    "  (1, 'Node', 3, 'foo', 1, 2), "
    "  (1, 'Node', 3, 'bar', 1, 1); "
    "");
  boost::shared_ptr<data_selection> sel = tdb.get_data_selection();

  assert_equal<bool>(
    sel->supports_historical_versions(), true,
    "data selection supports historical versions");

  std::vector<osm_edition_t> editions;
  editions.push_back(std::make_pair(osm_nwr_id_t(1), osm_version_t(2)));
  assert_equal<int>(
    sel->select_historical_relations(editions), 1,
    "number of historic relations selected");

  std::vector<osm_nwr_id_t> ids;
  ids.push_back(1);
  assert_equal<int>(
    sel->select_relations(ids), 1,
    "number of current relations selected");

  test_formatter f;
  sel->write_relations(f);
  assert_equal<size_t>(f.m_relations.size(), 1, "number of relations written");

  members_t relation1_members;
  relation1_members.push_back(member_info(element_type_node, 3, "foo"));

  assert_equal<test_formatter::relation_t>(
    test_formatter::relation_t(
      element_info(1, 2, 3, "2016-09-19T18:49:00Z", 1, std::string("user_1"), true),
      relation1_members,
      tags_t()
      ),
    f.m_relations[0], "relation written");
}

} // anonymous namespace

int main(int, char **) {
  try {
    test_database tdb;
    tdb.setup();

    tdb.run(boost::function<void(test_database&)>(
        &test_historic_elements));

    tdb.run(boost::function<void(test_database&)>(
        &test_historic_dup));

    tdb.run(boost::function<void(test_database&)>(
        &test_historic_dup_way));

    tdb.run(boost::function<void(test_database&)>(
        &test_historic_dup_relation));

  } catch (const test_database::setup_error &e) {
    std::cout << "Unable to set up test database: " << e.what() << std::endl;
    return 77;

  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
    return 1;

  } catch (...) {
    std::cout << "Unknown exception type." << std::endl;
    return 99;
  }

  return 0;
}