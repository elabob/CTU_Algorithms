//
// Created by bobenade on 16/11/2025.
//

//Moje teoreticke okienko k C++ :)
// v C++ set je zoradene poradie (ak dam do setu "7" "5" "1" vrati mi 1 5 7) a nedovoli mi robit duplitikaty - je to kolekcia unikatnych hodnot
        //vektor je nezoradene (ak dam do vektora "7" "5" "1" vrati mi 7 5 1)
        //existuje aj unordered set => ked ma nezaujima poradie ale chcem kolekciu unikatnych hodnot (bez duplikatov)

#include <algorithm>
#include <iostream>
#include <queue>
#include <vector>
#include <unordered_set>
#include <climits>

// Pomocna funkcia na kontrolu, ci je x v uzavretom intervale [min, max]
bool in_range(long long x, long long min, long long max) {
    return x >= min && x <= max;
}

// Pomocna funkcia na kontrolu, ci je x v polootvorenom intervale [min, max)
bool in_range_noisy(long long x, long long min, long long max) {
    return x >= min && x < max;
}

// Struktura na reprezentaciu stavu v prehladavani (Dijkstra/BFS)
struct State {
    int mouse_pos;      // Aktualny uzol, kde sa nachadza myska
    int cat_max_dist;   // Kolko "skokov" (dosah) uz kocur stihol ziskat hlukom
    int distance;       // Celkovy pocet krokov mysky od zaciatku
    int noisy_count;    // Celkovy pocet hlucnych uzlov, ktore myska doteraz navstivila

    // Operator porovnavania pre priority_queue (tzv. min-priority queue)
    // Prioritizujeme: 1. najmensiu vzdialenost, 2. najmensi pocet hlucnych uzlov
    bool operator>(const State &other) const {
        if (distance != other.distance) {
            return distance > other.distance;
        }
        return noisy_count > other.noisy_count;
    }
};

// Klasicky BFS algoritmus na vypocet najkratsej cesty (v pocte hran) z jedneho uzla do vsetkych ostatnych
std::vector<int> compute_distances(int start, const std::vector<std::vector<int>>& graph) {
    int n = graph.size();
    std::vector<int> dist(n, INT_MAX);
    std::queue<int> q;

    dist[start] = 0;
    q.push(start);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v : graph[u]) {
            if (dist[v] == INT_MAX) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }

    return dist;
}

int main() {
    // N = num_nodes,
    // H = num_noisy_nodes,
    // E = num_edges

    int num_nodes, num_noisy_nodes, num_edges;
    if (!(std::cin >> num_nodes >> num_noisy_nodes >> num_edges)) return 0;

    // S = mouse_start_idx,
    // C = mouse_end_idx,
    // K = cat_start_idx,
    // M = cat_max_jumps

    int mouse_start_idx, mouse_end_idx, cat_start_idx, cat_max_jumps;
    std::cin >> mouse_start_idx >> mouse_end_idx >> cat_start_idx >> cat_max_jumps;

    // Validacia vstupov podla zadanych limitov
    if (!(in_range(num_nodes, 10, (2 * 100000)) &&
          in_range_noisy(num_noisy_nodes, 1, num_nodes - 3) &&
          in_range(cat_max_jumps, 1, 50) &&
          in_range(num_edges * cat_max_jumps, 20, 10000000))) {
        std::cout << "Wrong input!\n";
        return 1;
    }

    // Nacitanie zoznamu hlucnych uzlov
    std::vector<int> noisy_nodes(num_noisy_nodes);
    for (int i = 0; i < num_noisy_nodes; i++) {
        std::cin >> noisy_nodes[i];
    }

    // Presun hlucnych uzlov do unordered_setu pre rychlu kontrolu (O(1)) v buducnosti
    std::unordered_set<int> noisy_set(noisy_nodes.begin(), noisy_nodes.end());

    // Budovanie adjacency listu (reprezentacia grafu)
    std::vector<std::vector<int> > graph(num_nodes);
    for (int i = 0; i < num_edges; i++) {
        int edge_1, edge_2;
        std::cin >> edge_1 >> edge_2;
        graph[edge_1].push_back(edge_2);
        graph[edge_2].push_back(edge_1); // graf je neorientovany
    }

    // PREDVYPOCET: Zistime, ako daleko je kocur od kazdeho uzla na mape
    std::vector<int> cat_distances = compute_distances(cat_start_idx, graph);

    // Inicializacia pociatocneho stavu mysky
    State initial_state;
    initial_state.mouse_pos = mouse_start_idx;
    initial_state.cat_max_dist = 0; // kocur zatial nema ziadny bonusovy dosah z hluku
    initial_state.distance = 0;
    initial_state.noisy_count = 0;

    // Priority queue pre Dijkstrov algoritmus (vzdy spracujeme "najlacnejsi" mozny stav)
    std::priority_queue<State, std::vector<State>, std::greater<State> > pq;

    // DP pole/Visited tabulka: visited[poloha_mysky][dosah_kocura] -> {najkratsia_cesta, min_hluku}
    // Stav v tomto probleme nie je len poloha, ale kombinacia (kde je mys a aky dosah ma kocur)
    std::vector<std::vector<std::pair<int, int>>> visited(num_nodes,
        std::vector<std::pair<int, int>>(cat_max_jumps + 1, {INT_MAX, INT_MAX}));

    pq.push(initial_state);
    visited[initial_state.mouse_pos][initial_state.cat_max_dist] = {0, 0};

    // Hlavny cyklus prehladavania stavoveho priestoru
    while (!pq.empty()) {
        State current = pq.top();
        pq.pop();

        // CIEL: Ak sme dorazili do domceka, vypiseme vysledok a koncime (vdaka priority_queue je prvy najdeny najlepsi)
        if (current.mouse_pos == mouse_end_idx) {
            std::cout << current.distance << " " << current.noisy_count << "\n";
            return 0;
        }

        // OREZA: Ak sme v tomto konkretnom stave (pozicia + dosah kocura) uz boli s lepsim vysledkom, ignorujeme
        if (current.distance > visited[current.mouse_pos][current.cat_max_dist].first ||
            (current.distance == visited[current.mouse_pos][current.cat_max_dist].first &&
             current.noisy_count > visited[current.mouse_pos][current.cat_max_dist].second)) {
            continue;
        }

        // Skusime sa pohnut na vsetkych susedov aktualneho uzla
        for (int neighbor: graph[current.mouse_pos]) {

            // Zistime, ci uzol, do ktoreho ideme, sposobi hluk
            bool is_noisy = (noisy_set.count(neighbor) > 0);

            // Ak je uzol hlucny, kocurovi sa zvacsi jeho "akcny radius" (dosah) o 1
            int new_cat_max_dist = current.cat_max_dist;
            if (is_noisy && current.cat_max_dist < cat_max_jumps) {
                new_cat_max_dist = current.cat_max_dist + 1;
            }

            // PODMIENKA BEZPECNOSTI:
            // Myska moze ist na 'neighbor' len vtedy, ak sa tam kocur nevie dostat
            // v ramci svojho aktualneho dosahu (new_cat_max_dist).
            if (cat_distances[neighbor] <= new_cat_max_dist) {
                // Kocur by mysku chytil - tento pohyb je zakazany
                continue;
            }

            // Vypocitame metriky pre novy stav
            int new_distance = current.distance + 1;
            int new_noisy_count = current.noisy_count + (is_noisy ? 1 : 0);

            // Kontrola, ci sme nasli lepsiu cestu do (neighbor, new_cat_max_dist)
            auto& best = visited[neighbor][new_cat_max_dist];
            if (new_distance < best.first ||
                (new_distance == best.first && new_noisy_count < best.second)) {

                best = {new_distance, new_noisy_count};

                // Vytvorime a pridame novy stav do priority fronty
                State new_state;
                new_state.mouse_pos = neighbor;
                new_state.cat_max_dist = new_cat_max_dist;
                new_state.distance = new_distance;
                new_state.noisy_count = new_noisy_count;

                pq.push(new_state);
            }
        }
    }

    return 0;
}