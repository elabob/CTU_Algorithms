//
// Created by bobenade on 10/10/2025.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <climits>

// Struktura pre Adjacency List (Zoznam susedov) pomocou spojoveho zoznamu
struct AdjNode {
    int vertex;
    AdjNode* next;
    AdjNode(int v) : vertex(v), next(nullptr) {}
};

// Globalne premenne pre ukladanie doteraz najlepsieho skore a poctu vrcholov
int best_score = INT_MIN / 2; // Inicializacia na velmi male cislo
int num_vertices;

// Predpocitam si zisky pre kazdy vrchol a typ agenta (zatial co v main neinicializovane, pripravene na rozsirenie)
int precomputed_gain[31][3]; // [vertex][agent_type: 0=none, 1=T1, 2=T2]

// Staticke polia pre rychly pristup k susedom (optimalizacia kvoli rychlosti rekuzie)
int neighbor_count[31];       // kolko susedov ma vrchol i
int neighbors[31][31];        // zoznam indexov susedov vrchola i

// Hlavna rekurzivna funkcia Branch and Bound
// t1_remaining/t2_remaining: kolko agentov daneho typu este musim umiestnit
// current_network_score: aktualny sucet bodov v rozostavanej sieti
void branch_and_bound(int current_index, int t1_remaining, int t2_remaining, int* agent_placement, int current_network_score,
                const int* vertex_order, const int* vertex_degree, int* best_possible_add) {

    // BAZICKY STAV: Ak sme presli vsetky vrcholy (list stromu prehladavania)
    if (current_index == num_vertices) {
        // Podmienka: Musime minut vsetkych agentov, aby bol vysledok validny
        if (t1_remaining == 0 && t2_remaining == 0 && current_network_score > best_score) {
            best_score = current_network_score;
        }
        return;
    }

    // OREZANIE 1: Ak je zostavajucich vrcholov menej ako agentov, ktorych treba umiestnit, tato vetva je nevalidna
    int vertices_left = num_vertices - current_index;
    if (t1_remaining + t2_remaining > vertices_left) return;

    // OREZANIE 2: Vypocet optimistickeho horneho odhadu (Upper Bound)
    // Berieme aktualne skore a pripocitame k nemu teoreticke maximum bodov,
    // ktore by mohli priniest zostavajuci agenti na zostavajucich vrcholoch.
    int optimistic_upper_bound = current_network_score;
    for (int i = current_index; i < num_vertices && (t1_remaining > 0 || t2_remaining > 0); i++) {
        optimistic_upper_bound += best_possible_add[i];
        if (i - current_index + 1 >= t1_remaining + t2_remaining) break;
    }

    // Ak ani v absolutne idealnom pripade neprekoname aktualne best_score, vetvu zahodime (pruning)
    if (optimistic_upper_bound <= best_score) return;

    // Ziskame realny index vrcholu podla vopred urceneho poradia (vertex_order)
    int vertex = vertex_order[current_index];
    int neighbor_cnt = neighbor_count[vertex];

    // --- MOZNOST A: Na vrchol umiestnime agenta typu T1 ---
    if (t1_remaining > 0) {
        agent_placement[vertex] = 1;
        int score_delta = 0;

        // Bodovanie T1: Body ziskava len za susedov, ktori su tiez T1 (+2 body za hranu)
        // Hrany so susedmi, ktori su T2 alebo prazdni, su za 0 bodov.
        for (int i = 0; i < neighbor_cnt; i++) {
            int nv_type = agent_placement[neighbors[vertex][i]];
            if (nv_type == 1) score_delta += 2;
        }

        branch_and_bound(current_index + 1, t1_remaining - 1, t2_remaining, agent_placement, current_network_score + score_delta,
                         vertex_order, vertex_degree, best_possible_add);

        // BACKTRACKING: Po navrate z rekurzie musime vrchol "vycistit" pre dalsie moznosti
        agent_placement[vertex] = 0;
    }

    // --- MOZNOST B: Na vrchol umiestnime agenta typu T2 ---
    if (t2_remaining > 0) {
        agent_placement[vertex] = 2;
        int score_delta = 0;

        // Bodovanie T2: Komplikovanejsie pravidla interakcie
        // T2 + (prazdny) = +1 bod
        // T2 + T1       = +1 bod
        // T2 + T2       = -1 bod (vzajomna rusivost)
        for (int i = 0; i < neighbor_cnt; i++) {
            int nv_type = agent_placement[neighbors[vertex][i]];
            if (nv_type == 0) score_delta += 1;
            else if (nv_type == 1) score_delta += 1;
            else if (nv_type == 2) score_delta -= 1;
        }

        branch_and_bound(current_index + 1, t1_remaining, t2_remaining - 1, agent_placement, current_network_score + score_delta,
                         vertex_order, vertex_degree, best_possible_add);

        agent_placement[vertex] = 0; // Backtracking
    }

    // --- MOZNOST C: Vrchol zostane prazdny (vobec tam nedat ziadneho agenta) ---
    branch_and_bound(current_index + 1, t1_remaining, t2_remaining, agent_placement, current_network_score, vertex_order,
        vertex_degree, best_possible_add);
}

int main() {
    // Optimalizacia standardneho vstupu a vystupu pre C++
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int num_edges, num_t1_agents, num_t2_agents;
    std::cin >> num_vertices >> num_edges >> num_t1_agents >> num_t2_agents;

    // Validacia vstupnych obmedzeni podla zadania (Max 30 vrcholov)
    if (num_vertices > 30 || num_edges > 180 || num_t1_agents + num_t2_agents > num_vertices) {
        std::cout << "Wrong input\n";
        return 0;
    }

    // Inicializacia adjacency listu pomocou vectoru pointrov (na uvolnenie pamate na konci)
    std::vector<AdjNode*> graph(num_vertices, nullptr);
    memset(neighbor_count, 0, sizeof(neighbor_count));

    // Nacitavanie hran a budovanie reprezentacie grafu
    for (int i = 0; i < num_edges; i++) {
        int u, v;
        std::cin >> u >> v;
        u--; v--; // Konverzia z 1-indexovania na 0-indexovanie

        // Pridanie do dynamickeho adjacency listu (neorientovany graf)
        auto* edge_a = new AdjNode(v);
        edge_a->next = graph[u];
        graph[u] = edge_a;

        auto* edge_b = new AdjNode(u);
        edge_b->next = graph[v];
        graph[v] = edge_b;

        // Pridanie do statickeho pola susedov pre extremne rychly pristup v rekurzii
        neighbors[u][neighbor_count[u]++] = v;
        neighbors[v][neighbor_count[v]++] = u;
    }

    // Heuristika: Zoradenie vrcholov podla ich stupna (Degree)
    // Branch and Bound funguje lepsie, ak zacneme vrcholmi s najvyssim poctom hran
    std::vector<std::pair<int,int>> degree_vertex_pairs;
    int* vertex_degree = new int[num_vertices];

    for (int i = 0; i < num_vertices; i++) {
        vertex_degree[i] = neighbor_count[i];
        degree_vertex_pairs.push_back({neighbor_count[i], i});
    }

    // Zoradenie zostupne (od najvyssieho stupna po najmensi)
    std::sort(degree_vertex_pairs.begin(), degree_vertex_pairs.end(),std::greater<std::pair<int,int>>());

    int* vertex_order = new int[num_vertices];
    int* best_possible_add = new int[num_vertices];

    // Priprava struktur pre rekurziu
    for (int i = 0; i < num_vertices; i++) {
        vertex_order[i] = degree_vertex_pairs[i].second; // Indexy vrcholov v poradi spracovania
        // Teoreticky maximalny mozny pridavok z jedneho vrchola (pre optimisticky odhad)
        // Vychadza z T1 pravidla: max 2 body na jednu hranu
        best_possible_add[i] = vertex_degree[vertex_order[i]] * 2;
    }

    // Pole pre sledovanie umiestnenia agentov v ramci rekurzivneho prehladavania
    int* agent_placement = new int[num_vertices];
    memset(agent_placement, 0, num_vertices * sizeof(int));

    // Spustenie hlavneho algoritmu od indexu 0
    branch_and_bound(0, num_t1_agents, num_t2_agents, agent_placement, 0, vertex_order,
        vertex_degree, best_possible_add);

    // Vypis vysledneho maximalneho skore
    std::cout << best_score << "\n";

    // --- MANUALNE UVOLNENIE PAMATE (Clean-up) ---
    // Aby sme sa vyhli memory leakom, kedze pouzivame 'new'
    for (int i = 0; i < num_vertices; i++) {
        AdjNode* current = graph[i];
        while (current) {
            AdjNode* temp = current;
            current = current->next;
            delete temp;
        }
    }
    delete[] vertex_degree;
    delete[] vertex_order;
    delete[] agent_placement;
    delete[] best_possible_add;

    return 0;
}