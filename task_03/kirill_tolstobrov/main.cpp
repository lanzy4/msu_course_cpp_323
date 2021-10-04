#include <iostream>
#include <map>
#include <cstdlib>
#include <ctime>
#include <iterator>
#include <vector>
#include <fstream>
#include <utility>


class Vertex {
    int id;
    int depth;
public:
    Vertex(int init_id, int init_depth): id(init_id), depth(init_depth) {}
    int get_id() const { return id; }
    int get_depth() const { return depth; }
};


class Edge{
    int id;
    int vertex1_id;
    int vertex2_id;
    std::string color;
public:
    Edge(int init_id, int v1_init, int v2_init, std::string color_init): id(init_id),
        vertex1_id(v1_init), vertex2_id(v2_init), color(color_init) {};
    int get_id() const { return id; }
    int get_v1_id() const { return vertex1_id; }
    int get_v2_id() const { return vertex2_id; }
    std::string get_color() const { return color; }
};


class Graph{
    std::vector<Edge> edges;
    std::vector<Vertex> vertices;

    //connections_map: vertex1 -> edge -> vertex2
    std::map<int, std::map<int, int>> connections_map;

    int vertices_amount;
    int edges_amount;
    int depth;

public:
    Graph(int depth_init = 0): vertices_amount(0), edges_amount(0), depth(depth_init) {}

    int get_vertices_amount() const { return vertices_amount; }
    int get_edges_amount() const { return edges_amount; }
    void set_depth(int new_depth) { depth = new_depth; }

    void add_new_vertex(int depth = 0){
        vertices.push_back(Vertex(vertices_amount, depth));
        vertices_amount++;
    }

    void bind_vertices(int id1, int id2, std::string edge_color = "grey") {
        edges.push_back(Edge(edges_amount, id1, id2, edge_color));
        edges_amount++;
        connections_map[id1][edges_amount - 1] = id2;
        connections_map[id2][edges_amount - 1] = id1;
    }

    operator std::string() {
        std::string result = "{\n  \"depth\": " + std::to_string(depth) + ",";
        result += "\n  \"vertices\": [\n";

        for(int i = 0; i < vertices_amount; i++) {

            std::string vertex_string = "";
            vertex_string += "    {\n      \"id\": " + std::to_string(i);
            vertex_string += ",\n      \"edge_ids\": ";

            auto &cur_vertex = connections_map[i];
            for (auto it = cur_vertex.begin(); it != cur_vertex.end(); it++) {
                if(it == cur_vertex.begin()) {
                    vertex_string += "[";
                } else {
                    vertex_string += ", ";
                }
                vertex_string += std::to_string(it->first);
            }
            vertex_string += "],\n";
            vertex_string += "      \"depth\": " + std::to_string(vertices[i].get_depth());
            vertex_string += "\n    }";

            result += vertex_string;
            if(i != vertices_amount - 1)
                result += ",\n";
        }

        result += "\n  ],\n  \"edges\": [\n";

        for(int i = 0; i < edges_amount; i++) {

            std::string edge_string = "";
            edge_string += "    {\n      \"id\": " + std::to_string(i);
            edge_string += ",\n      \"vertex_ids\": ";

            edge_string += "[" + std::to_string(edges[i].get_v1_id());
            edge_string += ", " + std::to_string(edges[i].get_v2_id());
            edge_string += "],\n";

            edge_string += "      \"color\": \"" + edges[i].get_color() + "\"";
            edge_string += "\n    }";

            result += edge_string;
            if(i != edges_amount - 1)
                result += ",\n";
        }

        result += "\n  ]\n}";
        return result;
    }

};

void generateRandomGraph(Graph &g, int depth, int new_vertices_num) {

    g.set_depth(depth);

    srand(std::time(NULL));

    const float probability_decreasement = 1.0 / depth;
    float new_vertex_prob = 1.0;
    g.add_new_vertex(0);
    //depths_map: depth -> (first_vertex_id; last_vertex_id)
    std::vector<std::pair<int, int>> depths_map;
    depths_map.push_back({0, 0});
    int vertices_amount = 1;

    for(int i = 0; i < depth; i++) {
        for(int j = depths_map[i].first; j <= depths_map[i].second; j++) {
            for(int k = 0; k < new_vertices_num; k++) {
                if ( (float)std::rand()/RAND_MAX <= new_vertex_prob) {
                    g.add_new_vertex(i + 1);
                    vertices_amount++;
                    g.bind_vertices(j, vertices_amount - 1);
                    if(depths_map.size() <= i + 1) {
                        depths_map.push_back({vertices_amount - 1, vertices_amount - 1});
                    } else {
                        depths_map[i+1].second++;
                    }
                }
            }
        }

        new_vertex_prob -= probability_decreasement;
    }


    enum {
        GREEN_PROB = 10,
        BLUE_PROB = 25,
        RED_PROB = 33,
        PROB_COEF = 100
    };
    int cur_depth = 0;
    float yellow_probability = 0;
    const float probability_increasement = probability_decreasement;

    for (int cur_id = 0; cur_id < vertices_amount; cur_id++) {

        if ( (float)std::rand()/RAND_MAX <= GREEN_PROB/(float)PROB_COEF ) {
            g.bind_vertices(cur_id, cur_id, "green");
        }

        if (cur_id != depths_map[cur_depth].second) {
            if ( (float)std::rand()/RAND_MAX <= BLUE_PROB/(float)PROB_COEF ) {
                g.bind_vertices(cur_id, cur_id + 1, "blue");
            }
        }

        if ( cur_depth != depth && (float)std::rand()/RAND_MAX <= yellow_probability/(float)PROB_COEF) {
            int next_depth_start = depths_map[cur_depth + 1].first;
            int next_depth_finish = depths_map[cur_depth + 1].second;

            int binded_id = next_depth_start + (std::rand() % (next_depth_finish - next_depth_start + 1));

            g.bind_vertices(cur_id, binded_id, "yellow");
        }

        if ( cur_depth < (depth - 1) && (float)std::rand()/RAND_MAX <= RED_PROB/(float)PROB_COEF) {
            int next_next_depth_start = depths_map[cur_depth + 2].first;
            int next_next_depth_finish = depths_map[cur_depth + 2].second;

            int binded_id = next_next_depth_start + (std::rand() % (next_next_depth_finish - next_next_depth_start + 1));

            g.bind_vertices(cur_id, binded_id, "red");
        }

        if (cur_id == depths_map[cur_depth].second) {
            cur_depth++;
        }
        yellow_probability += probability_increasement;
    }
        
}

void generateCustomGraph(Graph &g, int vert_number, std::vector<std::pair<int, int>> connections){

    for(int i = 0; i < vert_number; i++) {
        g.add_new_vertex();
    }
    for(int i = 0; i < connections.size(); i++) {
        g.bind_vertices(connections[i].first, connections[i].second);
    }
}


int main() {

    const int graphs_amount = 10;
    Graph graphs[10];

    int depth, new_vertices_num;

    std::cout << "Enter the depth:" << std::endl;
    std::cin >> depth;

    std::cout << "Enter max amount of vertices genereted from one vertex:" << std::endl;
    std::cin >> new_vertices_num;

    for(int i = 0; i < graphs_amount; i++) {
        generateRandomGraph(graphs[i], depth, new_vertices_num);
    }

    for(int i = 0; i < graphs_amount; i++) {
        std::string filename = "graph" + std::to_string(i + 1) + ".json";
        std::ofstream file;
        file.open(filename);

        if(file.is_open()) {
            file << (std::string)graphs[i] << std::endl;
        }
    }

    return 0;
}
