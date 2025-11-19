//
// Created by bobenade on 16/11/2025.
//

// BFS

#include <iostream>
#include <vector>
#include <algorithm>  // pre std::find
#include <queue>

bool in_range(long long x, long long low, long long high){
    return low <= x && x <= high;
}

bool in_range_noisy(long long x, long long low, long long high){
    return low <= x && x < high;
}

struct NodeInfo {
    int dist;       // vzdialenost od startu
    int noisy_cnt;  // pocet hlucnych uzlov
};

void bfs_cat(const std::vector<std::vector<int>>& graph, int max_jumps, const std::vector<int>& noisy_nodes, std::vector<bool>& safe) {
    int n = graph.size();

    // Pre kazdy hlucny uzol spravim BFS
    for (int noisy : noisy_nodes) {
        std::vector<int> dist(n, -1);  // vzdialenost od hlucneho uzla
        std::queue<int> q;             // fronta pre BFS

        dist[noisy] = 0;  // zacinam od hlucneho uzla
        q.push(noisy);

        while (!q.empty()) {
            int u = q.front(); q.pop();

            safe[u] = false; // tento uzol je v dosahu kocura = nebezpecny

            if (dist[u] == max_jumps) {
                continue; // uzol je na maxime dosahu kocura, dalej sa nedostanem
            }

            // prejdem vsetky susedne uzly
            for (int v : graph[u]) {
                if (dist[v] == -1) {        // uzol este nebol navstiveny
                    dist[v] = dist[u] + 1;  // nastavim vzdialenost od hlucneho uzla
                    q.push(v);              // pridam do fronty na dalsie spracovanie
                }
            }
        }
    }
}


void bfs_mouse(const std::vector<std::vector<int>>& graph, int start, int mouse_end_idx,
               const std::vector<bool>& is_noisy, std::vector<int>& parent, std::vector<bool>& safe, NodeInfo info[]) {
    int n = graph.size();
    for (int i = 0; i < n; i++) {
       info[i].dist = -1;      // -1 znamena, ze uzol este nebol navstiveny
       info[i].noisy_cnt = 100000; // nastavim velke cislo, aby som mohla porovnavat
       parent[i] = -1;
    }

    std::queue<int> q;

    info[start].dist = 0;       // vzdialenost od startu nastavim na 0
    info[start].noisy_cnt = 0;  // pocet hlucnych uzlov od startu je 0
    q.push(start);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v : graph[u]) {
            // prechadzam aj cez unsafe uzly, BFS si sama vybera minimalny pocet hlučných
            int new_noisy = info[u].noisy_cnt + (is_noisy[v] ? 1 : 0);
            int new_dist = info[u].dist + 1;

            if (info[v].dist == -1 || new_noisy < info[v].noisy_cnt ||
                (new_noisy == info[v].noisy_cnt && new_dist < info[v].dist)) { // uzol este nebol navstiveny alebo nasla som lepsiu cestu
                info[v].dist = new_dist;
                info[v].noisy_cnt = new_noisy;
                parent[v] = u;  // ulozim, z ktoreho uzla som prisla
                q.push(v);      // pridam uzol do fronty
                }
        }
    }
}


int main() {
     // N = num_nodes,
     // H = num_noisy_nodes,
     // E = num_edges

    int num_nodes, num_noisy_nodes, num_edges;
    std::cin >> num_nodes >> num_noisy_nodes >> num_edges;

    // S = mouse_start_idx,
    // C = mouse_end_idx,
    // K = cat_start_idx,
    // M = cat_max_jumps

    int mouse_start_idx, mouse_end_idx, cat_start_idx, cat_max_jumps;
    std::cin >> mouse_start_idx >> mouse_end_idx >> cat_start_idx >> cat_max_jumps;

    if (!(in_range(num_nodes,10, (2*100000)) &&
        in_range_noisy(num_noisy_nodes, 1, (num_nodes-3)) &&
        in_range(cat_max_jumps, 1,50) &&
        in_range((num_edges*cat_max_jumps),20, 10000000))) {

        std::cout << "Wrong input";
        return -1;
    }

    std::vector<int> noisy_nodes(num_noisy_nodes);
    for (int i=0; i<num_noisy_nodes; i++) {
        std::cin >> noisy_nodes[i];
        // std::cout << "Loaded noisy_node_" << (i+1) << " = " << noisy_nodes[i] << "\n";
    }

    std::vector<std::vector<int>> graph(num_nodes);
    for (int i = 0; i < num_edges; i++) {
        int u, v;
        std::cin >> u >> v;
        graph[u].push_back(v);
        graph[v].push_back(u);  // graf je neorientovany
    }

    std::vector<bool> safe(num_nodes, true); //vsetko je bezpecne na zaciatku

    bfs_cat(graph, cat_max_jumps, noisy_nodes, safe);

    //std::cout << "Nebezpecne uzly kvoli kocurovi: ";
    // for (int i = 0; i < num_nodes; i++) {
    //     if (!safe[i]) {
    //         std::cout << i << " ";
    //     }
    // }
    // std::cout << "\n";

    std::vector<int> dist(num_nodes, -1);       // vzdialenost od startu mysi
    std::vector<int> noisy_count(num_nodes, 0); // pocet hlučných uzlov po ceste
    std::vector<bool> is_noisy(num_nodes, false);
    for (int node : noisy_nodes) is_noisy[node] = true;


    std::vector<int> parent(num_nodes, -1); //rodic = uzol z ktoreho som prisla na ten aktualny; vektor o velkosti poctu uzlov a na zaciatku je kazdy rodic -1 (=nebol navstiveny)
    NodeInfo info[num_nodes];
    bfs_mouse(graph, mouse_start_idx, mouse_end_idx, is_noisy, parent, safe, info);

    std::vector<int> path;
    for (int v = mouse_end_idx; v != -1; v = parent[v]) {
        path.push_back(v);  //Idem z ciela do startu
    }
    std::reverse(path.begin(), path.end());  //otocim to -> idem od startu do ciela

    //std::cout << "Trasa mysi: ";
    int count_noisy = 0;
    for (int u : path) {
        //std::cout << u << " ";
        if (is_noisy[u]) {
            count_noisy++;
        }
    }

    std::cout << (path.size()-1) << " " << count_noisy<< "\n";


    // for (int neighbor_mouse : graph[mouse_pos]) {
    //     //std::cout << "Mys moze na uzol " << neighbor_mouse<< "\n";
    //
    //     if (std::find (nodes_mouse_knows.begin(), nodes_mouse_knows.end(), neighbor_mouse) == nodes_mouse_knows.end()) {
    //         //mys nepozna tento uzol ako jeden z nebezpecnych -->moze tam ist :)
    //         std::cout << "Mys moze bezpecne ist na uzol " << neighbor_mouse << "\n";
    //     }
    // }


}