//
// Created by juan-diego on 3/29/24.
//

#ifndef HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H
#define HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H


#include <queue>

#include "window_manager.h"
#include "graph.h"
#include <unordered_map>
#include <set>
#include <cmath>
#include <queue>
#include <climits>

#include <unordered_set>

using namespace std;

// Este enum sirve para identificar el algoritmo que el usuario desea simular
enum Algorithm {
    Dijkstra,
    AStar,
    BestFirst
};


//* --- PathFindingManager ---
//
// Esta clase sirve para realizar las simulaciones de nuestro grafo.
//
// Variables miembro
//     - path           : Contiene el camino resultante del algoritmo que se desea simular
//     - visited_edges  : Contiene todas las aristas que se visitaron en el algoritmo, notar que 'path'
//                        es un subconjunto de 'visited_edges'.
//     - window_manager : Instancia del manejador de ventana, es utilizado para dibujar cada paso del algoritmo
//     - src            : Nodo incial del que se parte en el algoritmo seleccionado
//     - dest           : Nodo al que se quiere llegar desde 'src'
//*
class PathFindingManager {
private:
    WindowManager *window_manager;
    std::vector<sfLine> path;
    std::vector<sfLine> visited_edges;
    int render_counter = 0;
    const int RENDER_FREQUENCY = 50;
    Graph* current_graph = nullptr;

    struct Entry {
        Node* node;
        double dist;
        double priority; // Para A* y Best First

        bool operator < (const Entry& other) const {
            return priority > other.priority;  // Min-heap
        }
    };

    // Complejidad O((V+E)logV) por uso de MinHeap
    void dijkstra(Graph &graph) {
        std::unordered_map<Node *, Node *> parent;
        if (src == NULL || dest == NULL) {
            set_final_path(parent);
            return;
        }
        std::unordered_map<Node*, double> dist;
        std::unordered_map<Node*, bool> visitado;
        double INF = 1000000000.0;
        for (auto it = graph.nodes.begin(); it != graph.nodes.end(); ++it) {
            Node* nodo = it->second;
            dist[nodo] = INF;
            visitado[nodo] = false;
            parent[nodo] = NULL;
        }
        dist[src] = 0.0;
        parent[src] = NULL;
        std::priority_queue<std::pair<double, Node*>,std::vector<std::pair<double, Node*> >,std::greater<std::pair<double, Node*>>> pq;
        pq.push(std::make_pair(0.0, src));
        while (!pq.empty()) {
            std::pair<double, Node*> top = pq.top();
            pq.pop();
            double dist_u = top.first;
            Node* u = top.second;
            if (visitado[u]) continue;
            visitado[u] = true;
            if (u == dest) break;
            for (std::size_t i = 0; i < u->edges.size(); ++i) {
                Edge* edge = u->edges[i];
                Node* v = NULL;
                if (edge->src == u) {
                    v = edge->dest;
                } else if (edge->dest == u) {
                    v = edge->src;
                } else {
                    continue;
                }
                if (visitado[v]) continue;
                double peso = edge->length;
                double nueva_dist = dist_u + peso;
                if (nueva_dist < dist[v]) {
                    dist[v] = nueva_dist;
                    parent[v] = u;
                    pq.push(std::make_pair(nueva_dist, v));
                    sfLine linea_explorada(u->coord,v->coord,sf::Color(100, 100, 255),1.0f);
                    visited_edges.push_back(linea_explorada);
                }
            }
        }
        set_final_path(parent);
    }

    // Complejidad O((V+E)logV) por uso de MinHeap con heurística
    void a_star(Graph &graph) {
        std::unordered_map<Node *, Node *> parent;
        // TODO: Add your code here
        if (src == NULL || dest == NULL) {
            set_final_path(parent);
            return;
        }

        // g[n] = costo real desde el origen hasta n
        std::unordered_map<Node*, double> g;
        // f[n] = g[n] + h[n]
        std::unordered_map<Node*, double> f;
        std::unordered_map<Node*, bool> visitado;

        double INF = 1000000000.0;

        // Inicializar todos los nodos
        for (auto it = graph.nodes.begin(); it != graph.nodes.end(); ++it) {
            Node* nodo = it->second;
            g[nodo] = INF;
            f[nodo] = INF;
            visitado[nodo] = false;
            parent[nodo] = NULL;
        }

        // Configurar nodo inicial
        g[src] = 0.0;
        f[src] = heuristic(src, dest);
        parent[src] = NULL;

        // Cola de prioridad ordenada por f (g + heurística)
        std::priority_queue<std::pair<double, Node*>,
                           std::vector<std::pair<double, Node*>>,
                           std::greater<std::pair<double, Node*>>> pq;

        pq.push(std::make_pair(f[src], src));

        while (!pq.empty()) {
            std::pair<double, Node*> top = pq.top();
            pq.pop();
            Node* u = top.second;

            if (visitado[u]) continue;
            visitado[u] = true;

            if (u == dest)
                break;

            for (std::size_t i = 0; i < u->edges.size(); ++i) {
                Edge* edge = u->edges[i];
                Node* v = NULL;

                if (edge->src == u) {
                    v = edge->dest;
                } else if (edge->dest == u) {
                    v = edge->src;
                } else {
                    continue;
                }

                if (visitado[v]) continue;

                double peso = edge->length;
                double tentative_g = g[u] + peso;

                if (tentative_g < g[v]) {
                    g[v] = tentative_g;
                    parent[v] = u;
                    f[v] = g[v] + heuristic(v, dest);
                    pq.push(std::make_pair(f[v], v));

                    // Dibujar arista explorada
                    sfLine linea_explorada(u->coord, v->coord, sf::Color(100, 100, 255), 1.0f);
                    visited_edges.push_back(linea_explorada);

                    // Visualizar el progreso del algoritmo
                    render(graph);
                }
            }
        }

        set_final_path(parent);
    }

    void best_first_search(Graph &graph) {
        std::unordered_map<Node *, Node *> parent;
        std::unordered_set<Node *> visited;
        priority_queue<Entry> pq;
        double h_start = euclid(src, dest);
        pq.push({src, 0, h_start});
        while (!pq.empty()) {
            Entry current_entry = pq.top();
            pq.pop();
            Node* current = current_entry.node;
            if (visited.find(current) != visited.end()) {
                continue;
            }
            visited.insert(current);
            if (current == dest) {
                break;
            }
            for (Edge* edge : current->edges) {
                Node* vecino = nullptr;
                if (edge->src == current) {
                    vecino = edge->dest;
                } else if (!edge->one_way && edge->dest == current) {
                    vecino = edge->src;
                } else {
                    continue;
                }
                if (visited.find(vecino) != visited.end()) {
                    continue;
                }
                double heuristic = euclid(vecino, dest);
                parent[vecino] = current;
                pq.push({vecino, 0, heuristic});
                visited_edges.push_back(sfLine(current->coord, vecino->coord,
                                              sf::Color::Yellow, 1.5f));
            }
        }
        set_final_path(parent);
    }

    double euclid(Node* a, Node* b) {
        double dx = a->coord.x - b->coord.x;
        double dy = a->coord.y - b->coord.y;
        return sqrt(dx * dx + dy * dy);
    }

    void render() {
        if (current_graph == nullptr) {
            return;
        }
        window_manager->clear();
        for (const auto& [id, node] : current_graph->nodes) {
            node->draw(window_manager->get_window());
        }
        for (const sfLine& line : visited_edges) {
            line.draw(window_manager->get_window(), sf::RenderStates::Default);
        }
        if (src != nullptr) {
            src->draw(window_manager->get_window());
        }
        if (dest != nullptr) {
            dest->draw(window_manager->get_window());
        }
        window_manager->display();
        sf::sleep(sf::milliseconds(1));
    }

    void set_final_path(std::unordered_map<Node *, Node *> &parent) {
        path.clear();
        Node* current = dest;
        while (current != nullptr && parent.find(current) != parent.end()) {
            Node* prev = parent[current];
            if (prev != nullptr) {
                path.push_back(sfLine(prev->coord, current->coord,
                                     sf::Color::Red, 3.0f));
            }
            current = prev;
        }
    }

public:
    Node *src = nullptr;
    Node *dest = nullptr;

    explicit PathFindingManager(WindowManager *window_manager) : window_manager(window_manager) {}

    void exec(Graph &graph, Algorithm algorithm) {
        if (src == nullptr || dest == nullptr) {
            return;
        }
        path.clear();
        visited_edges.clear();
        render_counter = 0;
        current_graph = &graph;
        switch (algorithm) {
            case Dijkstra:
                dijkstra(graph);
                break;
            case AStar:
                a_star(graph);
                break;
            case BestFirst:
                best_first_search(graph);
                break;
            default:
                break;
        }
    }

    void reset() {
        path.clear();
        visited_edges.clear();
        render_counter = 0;
        if (src) {
            src->reset();
            src = nullptr;
        }
        if (dest) {
            dest->reset();
            dest = nullptr;
        }
    }

    void draw(bool draw_extra_lines) {
        // Dibujar todas las aristas visitadas
        if (draw_extra_lines) {
            for (sfLine &line: visited_edges) {
                line.draw(window_manager->get_window(), sf::RenderStates::Default);
            }
        }

        // Dibujar el camino resultante entre 'str' y 'dest'
        for (sfLine &line: path) {
            line.draw(window_manager->get_window(), sf::RenderStates::Default);
        }

        // Dibujar el nodo inicial
        if (src != nullptr) {
            src->draw(window_manager->get_window());
        }

        // Dibujar el nodo final
        if (dest != nullptr) {
            dest->draw(window_manager->get_window());
        }
    }
};


#endif //HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H
