//
// Created by bobenade on 15/01/2026.
//

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

/**
 * Úloha: Promo jazda v lyžiarskom areáli (ALG)
 * Riešenie: Dynamické programovanie na orientovanom acyklickom grafe (DAG).
 */

const long long INF = 1e18; // Dostatočne veľká hodnota pre inicializáciu

struct Hrana {
    int ciel;
    int kredity;
};

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int N, M;
    if (!(cin >> N >> M)) return 0;

    vector<vector<Hrana>> adj(N + 1);
    vector<vector<Hrana>> rev_adj(N + 1);
    vector<int> in_degree(N + 1, 0);
    vector<int> out_degree(N + 1, 0);

    for (int i = 0; i < M; ++i) {
        int u, v, c;
        cin >> u >> v >> c;
        adj[u].push_back({v, c});
        rev_adj[v].push_back({u, c});
        in_degree[v]++;
        out_degree[u]++;
    }

    // 1. Identifikácia východiskových staníc (zdrojov)
    vector<int> zdroje;
    for (int i = 1; i <= N; ++i) {
        if (in_degree[i] == 0) {
            zdroje.push_back(i);
        }
    }
    int L = zdroje.size();

    // 2. DP pre minimálne náklady príchodu VŠETKÝCH lyžiarov do stanice i
    // cost_to_reach[i] = suma min. nákladov z každého zdroja do i
    // Najprv spočítame min. cestu z každého zdroja do každého uzla
    // Alebo efektívnejšie: dist[uzol] je min. cena z akéhokoľvek zdroja
    // Keďže každý lyžiar musí prísť z "svojho" zdroja, využijeme topologické radenie.

    // Pre každý uzol 'u' potrebujeme vedieť:
    // min_cost[u] = minimálna suma kreditov potrebná, aby sa L lyžiarov
    // (každý zo svojho unikátneho štartu) stretlo v bode 'u'.

    // Keďže cesty sú nezávislé, min_sum[u] = suma_{s v zdroje} (min_vzdialenost(s, u))
    // To by bolo pomalé. Využijeme fakt, že graf je DAG a každý uzol je dosiahnuteľný
    // zo zdrojov.

    vector<long long> min_sum(N + 1, 0);
    vector<int> count_reached(N + 1, 0); // Počet zdrojov, ktoré vedia dosiahnuť uzol i

    // Pre každý zdroj spustíme jeden prechod (keďže L je malé, max 10)
    for (int start_node : zdroje) {
        vector<long long> dist(N + 1, INF);
        dist[start_node] = 0;

        // Topologické spracovanie pre konkrétny zdroj
        priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<pair<long long, int>>> pq;
        pq.push({0, start_node});

        while(!pq.empty()){
            long long d = pq.top().first;
            int u = pq.top().second;
            pq.pop();
            if(d > dist[u]) continue;
            for(auto& hrana : adj[u]){
                if(dist[u] + hrana.kredity < dist[hrana.ciel]){
                    dist[hrana.ciel] = dist[u] + hrana.kredity;
                    pq.push({dist[hrana.ciel], hrana.ciel});
                }
            }
        }

        for(int i = 1; i <= N; ++i) {
            if(dist[i] != INF) {
                min_sum[i] += dist[i];
                count_reached[i]++;
            }
        }
    }

    // 3. DP pre maximálny zisk zo stanice i do cieľa (spoločná jazda)
    // profit_to_end[i] = max suma kreditov z i do cieľa
    vector<long long> profit_to_end(N + 1, -1);

    // Nájdeme cieľovú stanicu (out_degree == 0)
    int ciel = -1;
    for(int i = 1; i <= N; ++i) if(out_degree[i] == 0) ciel = i;

    // Funkcia pre výpočet profitu (memoizácia)
    auto get_profit = [&](auto self, int u) -> long long {
        if (u == ciel) return 0;
        if (profit_to_end[u] != -1) return profit_to_end[u];

        long long max_p = -INF;
        for (auto& hrana : adj[u]) {
            long long res = self(self, hrana.ciel);
            if (res != -INF) {
                max_p = max(max_p, (long long)hrana.kredity + res);
            }
        }
        return profit_to_end[u] = max_p;
    };

    // 4. Výpočet celkového výsledku
    long long max_total_profit = -INF;
    for (int i = 1; i <= N; ++i) {
        // Musia sa tam vedieť stretnúť VŠETCI lyžiari
        if (count_reached[i] == L) {
            long long p = get_profit(get_profit, i);
            if (p != -INF) {
                // Zisk = (L * profit z i do cieľa) - (náklady na stretnutie v i)
                long long current_z = (L * p) - min_sum[i];
                if (current_z > max_total_profit) {
                    max_total_profit = current_z;
                }
            }
        }
    }

    cout << L << " " << max_total_profit << endl;

    return 0;
}