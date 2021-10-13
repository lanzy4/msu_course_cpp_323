#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

using VertexId = int;
using EdgeId = int;

class Vertex {
 public:
  explicit Vertex(const VertexId& init_id = 0, const int& init_depth = 0)
      : id(init_id), depth(init_depth) {}
  int check_edge_presence(const EdgeId& edge_id) {
    for (const auto& id : connected_edges_) {
      if (edge_id == id) {
        return 1;
      }
    }
    return 0;
  }
  void add_edge(const EdgeId& edge_id) {
    assert(!check_edge_presence(edge_id) &&
           "Attemptig to add added edge to vertex: Error.");
    connected_edges_.push_back(edge_id);
  }
  operator std::string() const {
    std::string result = "";
    result += "    {\n      \"id\": " + std::to_string(id);
    result += ",\n      \"edge_ids\": [";

    for (int i = 0; i < connected_edges_.size(); i++) {
      result += std::to_string(connected_edges_[i]);
      if (i == connected_edges_.size() - 1) {
        result += "],\n";
      } else {
        result += ", ";
      }
    }
    result += "      \"depth\": " + std::to_string(depth);
    result += "\n    }";
    return result;
  }

  const VertexId id;
  const int depth;

 private:
  std::vector<EdgeId> connected_edges_;
};

class Edge {
 public:
  Edge(const EdgeId& init_id,
       const VertexId& v1_init,
       const VertexId& v2_init,
       std::string color_init)
      : id(init_id),
        vertex1_id(v1_init),
        vertex2_id(v2_init),
        color(color_init){};
  operator std::string() const {
    std::string result = "";
    result += "    {\n      \"id\": " + std::to_string(id);
    result += ",\n      \"vertex_ids\": ";

    result += "[" + std::to_string(vertex1_id);
    result += ", " + std::to_string(vertex2_id);
    result += "],\n      \"color\": \"";
    result += color;
    result += "\"\n    }";

    return result;
  }

  const EdgeId id;
  const VertexId vertex1_id;
  const VertexId vertex2_id;
  const std::string color;
};

class Graph {
 public:
  Graph(int depth_init = 0) : depth(depth_init) {
    colors_counter["grey"] = 0;
    colors_counter["green"] = 0;
    colors_counter["blue"] = 0;
    colors_counter["yellow"] = 0;
    colors_counter["red"] = 0;
  }
  int get_vertices_amount() const { return vertices_.size(); }
  int get_edges_amount() const { return edges_.size(); }

  void add_new_vertex(int depth = 0) {
    vertices_.push_back(Vertex(vertices_.size(), depth));
  }

  int are_vertices_connected(const VertexId& id1, const VertexId& id2) {
    for (const auto& connection : connections_map_[id1]) {
      if (connection.second == id2) {
        return 1;
      }
    }
    return 0;
  }

  void bind_vertices(const VertexId& id1,
                     const VertexId& id2,
                     std::string edge_color = "grey") {
    assert(!are_vertices_connected(id1, id2) &&
           "Attemptig to connect connected vertices: Error.");
    assert(id1 < vertices_.size() && id2 < vertices_.size() &&
           "Attemptig to connect nonexistent vertex: Error.");
    const auto& edge = edges_.emplace_back(edges_.size(), id1, id2, edge_color);
    connections_map_[id1][edge.id] = id2;
    connections_map_[id2][edge.id] = id1;
    vertices_[id1].add_edge(edge.id);
    if (id1 != id2)
      vertices_[id2].add_edge(edge.id);
    colors_counter[edge_color]++;
  }

  void set_depths_map(std::vector<std::pair<int, int>>& d_m) {
    depths_map = d_m;
  }

  operator std::string() const {
    std::string result = "{\n  \"vertices\": [\n";

    for (int i = 0; i < vertices_.size(); i++) {
      std::string vertex_string = std::string(vertices_[i]);

      result += vertex_string;
      if (i != vertices_.size() - 1)
        result += ",\n";
    }

    result += "\n  ],\n  \"edges\": [\n";

    for (int i = 0; i < edges_.size(); i++) {
      std::string edge_string = std::string(edges_[i]);

      result += edge_string;
      if (i != edges_.size() - 1)
        result += ",\n";
    }

    result += "\n  ]\n}";
    return result;
  }

  const int depth;

 private:
  std::vector<Edge> edges_;
  std::vector<Vertex> vertices_;

  // connections_map: vertex1 -> edge -> vertex2
  std::map<VertexId, std::map<EdgeId, VertexId>> connections_map_;
  std::vector<std::pair<int, int>> depths_map;
  std::map<std::string, int> colors_counter;
};

Graph generateRandomGraph(int depth, int new_vertices_num) {
  Graph graph = Graph(depth);

  srand(std::time(NULL));

  const float probability_decreasement = 1.0 / depth;
  float new_vertex_prob = 1.0;
  graph.add_new_vertex(0);
  // depths_map: depth -> (first_vertex_id; last_vertex_id)
  std::vector<std::pair<int, int>> depths_map;
  depths_map.push_back({0, 0});
  int vertices_amount = 1;

  for (int i = 0; i < depth; i++) {
    for (int j = depths_map[i].first; j <= depths_map[i].second; j++) {
      for (int k = 0; k < new_vertices_num; k++) {
        if ((double)std::rand() / RAND_MAX <= new_vertex_prob) {
          graph.add_new_vertex(i + 1);
          vertices_amount++;
          graph.bind_vertices(j, vertices_amount - 1);
          if (depths_map.size() <= i + 1) {
            depths_map.push_back({vertices_amount - 1, vertices_amount - 1});
          } else {
            depths_map[i + 1].second++;
          }
        }
      }
    }

    new_vertex_prob -= probability_decreasement;
  }

  enum { GREEN_PROB = 10, BLUE_PROB = 25, RED_PROB = 33, PROB_COEF = 100 };
  int cur_depth = 0;
  float yellow_probability = 0;
  const float probability_increasement = probability_decreasement;

  for (int cur_id = 0; cur_id < vertices_amount; cur_id++) {
    if ((double)std::rand() / RAND_MAX <= GREEN_PROB / (float)PROB_COEF) {
      graph.bind_vertices(cur_id, cur_id, "green");
    }

    if (cur_id != depths_map[cur_depth].second) {
      if ((double)std::rand() / RAND_MAX <= BLUE_PROB / (float)PROB_COEF) {
        graph.bind_vertices(cur_id, cur_id + 1, "blue");
      }
    }

    if (cur_depth != depth && (double)std::rand() / RAND_MAX <=
                                  yellow_probability / (float)PROB_COEF) {
      int next_depth_start = depths_map[cur_depth + 1].first;
      int next_depth_finish = depths_map[cur_depth + 1].second;

      int binded_id =
          next_depth_start +
          (std::rand() % (next_depth_finish - next_depth_start + 1));

      graph.bind_vertices(cur_id, binded_id, "yellow");
    }

    if (cur_depth < (depth - 1) &&
        (double)std::rand() / RAND_MAX <= RED_PROB / (float)PROB_COEF) {
      int next_next_depth_start = depths_map[cur_depth + 2].first;
      int next_next_depth_finish = depths_map[cur_depth + 2].second;

      int binded_id =
          next_next_depth_start +
          (std::rand() % (next_next_depth_finish - next_next_depth_start + 1));

      graph.bind_vertices(cur_id, binded_id, "red");
    }

    if (cur_id == depths_map[cur_depth].second) {
      cur_depth++;
    }
    yellow_probability += probability_increasement;
  }

  graph.set_depths_map(depths_map);
  return graph;
}

Graph generateCustomGraph(
    int vert_number,
    const std::vector<std::pair<VertexId, VertexId>>& connections) {
  Graph graph = Graph();
  for (int i = 0; i < vert_number; i++) {
    graph.add_new_vertex();
  }
  for (const auto& connection : connections) {
    graph.bind_vertices(connection.first, connection.second);
  }
  return graph;
}

int main() {
  int depth, new_vertices_num;

  std::cout << "Enter the depth:" << std::endl;
  std::cin >> depth;

  std::cout << "Enter max amount of vertices genereted from one vertex:"
            << std::endl;
  std::cin >> new_vertices_num;

  Graph graph = generateRandomGraph(depth, new_vertices_num);

  std::string filename = "graph.json";
  std::ofstream file;
  file.open(filename);
  if (file.is_open()) {
    file << (std::string)graph << std::endl;
  }
  file.close();

  return 0;
}
