//
// Created by juan-diego on 3/29/24.
//

#ifndef HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H
#define HOMEWORK_GRAPH_PATH_FINDING_MANAGER_H


#include "window_manager.h"
#include "graph.h"
#include <unordered_map>
#include <set>
#include <queue>


// Este enum sirve para identificar el algoritmo que el usuario desea simular
enum Algorithm {
    None,
    Dijkstra,
    AStar
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
    WindowManager *window_manager;
    std::vector<sfLine> path;
    std::vector<sfLine> visited_edges;

    struct Entry {
        Node* node;
        double dist;

        bool operator < (const Entry& other) const {
            return dist < other.dist;
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

    void a_star(Graph &graph) {
        std::unordered_map<Node *, Node *> parent;
        // TODO: Add your code here

        set_final_path(parent);
    }

    //* --- render ---
    // En cada iteración de los algoritmos esta función es llamada para dibujar los cambios en el 'window_manager'
    void render() {
        sf::sleep(sf::milliseconds(10));
        // TODO: Add your code here
    }

    //* --- set_final_path ---
    // Esta función se usa para asignarle un valor a 'this->path' al final de la simulación del algoritmo.
    // 'parent' es un std::unordered_map que recibe un puntero a un vértice y devuelve el vértice anterior a el,
    // formando así el 'path'.
    //
    // ej.
    //     parent(a): b
    //     parent(b): c
    //     parent(c): d
    //     parent(d): NULL
    //
    // Luego, this->path = [Line(a.coord, b.coord), Line(b.coord, c.coord), Line(c.coord, d.coord)]
    //
    // Este path será utilizado para hacer el 'draw()' del 'path' entre 'src' y 'dest'.
    //*
    void set_final_path(std::unordered_map<Node *, Node *> &parent) {
        path.clear();
        if (dest == NULL) return;
        if (parent.find(dest) == parent.end()) return;
        Node* current = dest;
        std::vector<sfLine> temporal;
        while (current != NULL && current != src) {
            Node* p = parent[current];
            if (p == NULL) {
                break; // no hay más padres, se corta el camino
            }
            // Línea desde el padre hasta el hijo (p -> current)
            sfLine linea_camino(
                p->coord,
                current->coord,
                sf::Color::Red,  // color del camino final
                2.0f             // grosor del camino
            );
            temporal.push_back(linea_camino);
            current = p; // seguimos retrocediendo hacia el origen
        }
        for (int i = (int)temporal.size() - 1; i >= 0; --i) {
            path.push_back(temporal[i]);
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
        // Ejecutar el algoritmo seleccionado
        if (algorithm == Dijkstra) {
            dijkstra(graph);
        } else if (algorithm == AStar) {
            a_star(graph);
        }
    }

    void reset() {
        path.clear();
        visited_edges.clear();

        if (src) {
            src->reset();
            src = nullptr;
            // ^^^ Pierde la referencia luego de restaurarlo a sus valores por defecto
        }
        if (dest) {
            dest->reset();
            dest = nullptr;
            // ^^^ Pierde la referencia luego de restaurarlo a sus valores por defecto
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
