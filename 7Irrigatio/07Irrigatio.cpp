//
// Created by bobenade on 20/12/2025.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>   // Knižnica na meranie reálneho času behu programu

// Štruktúra uchovávajúca stav v konkrétnom bode mriežky
struct State {
    int last_sum;      // Hodnota (súčet) posledného vytvoreného úseku potrubia
    int total_valves;  // Celkový počet nazbieraných ventilov na celej ceste
};

// Komparátor pre zoradenie stavov: primárne podľa celkového počtu ventilov (zostupne),
// sekundárne podľa veľkosti posledného úseku (zostupne)
bool compare(const State &a, const State &b) {
    if (a.total_valves != b.total_valves) {
        return a.total_valves > b.total_valves;
    }
    return a.last_sum > b.last_sum;
}

int main() {
    // Spustenie merania času (užitočné pri ladení efektivity algoritmu)
    auto start_time = std::chrono::high_resolution_clock::now();

    // Zrýchlenie štandardného I/O v C++
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    int num_row, num_column;
    if (!(std::cin >> num_row >> num_column)) return 0;

    // Načítanie vstupnej matice hodnôt (ventilv)
    std::vector<std::vector<int>> matrix(num_row);
    for (int i=0; i< num_row; i++) {
        for (int j=0; j<num_column; j++) {
            int number;
            std::cin>>number;
            matrix[i].push_back(number);
        }
    }

    // VÝPOČET PREFIXOVÝCH SÚČTOV (kľúčové pre rýchle sčítanie úsekov v O(1))

    // row_prefix[i][j] = súčet hodnôt v riadku i od indexu 0 po j
    std::vector<std::vector<int>>row_prefix(num_row);
    for (int i=0; i< num_row ; i++) {
        for (int j=0; j<num_column; j++) {
            if (j == 0) {
                row_prefix[i].push_back(matrix[i][j]);
            } else {
                row_prefix[i].push_back(matrix[i][j] + row_prefix[i][j-1]);
            }
        }
    }

    // column_prefix[i][j] = súčet hodnôt v stĺpci j od indexu 0 po i
    std::vector<std::vector<int>> column_prefix(num_row);
    for (int i=0; i< num_row ; i++) {
        for (int j=0; j<num_column; j++) {
            if (i == 0) {
                column_prefix[i].push_back(matrix[i][j]);
            } else {
                column_prefix[i].push_back(matrix[i][j] + column_prefix[i-1][j]);
            }
        }
    }

    // TABUĽKY PRE DYNAMICKÉ PROGRAMOVANIE
    // Pre každý bod [i][j] si pamätáme zoznam možných stavov (State),
    // rozdelené podľa toho, či sme do bodu prišli vertikálne alebo horizontálne.
    static std::vector<State> dynamic_vertical_path[205][205];
    static std::vector<State> dynamic_horizontal_path[205][205];

    // Inicializácia štartovacieho bodu [0][0]
    // last_sum je nastavený na veľké číslo, aby prvý úsek mohol byť akokoľvek veľký
    dynamic_vertical_path[0][0].push_back({100000000,0});
    dynamic_horizontal_path[0][0].push_back({100000000,0});

    // Hlavné cykly DP - prechádzame mriežku po bunkách
    for (int i=0; i < num_row; i++) {
        for (int j = 0; j < num_column; j++) {

            if (i == 0 && j == 0) continue;

            // 1. SKÚŠAME HORIZONTÁLNY POHYB (prichádzame z bodu [i][k] do [i][j])
            for (int k = 0; k < j; k++) {
                // Aby sme teraz išli vodorovne, museli sme v [i][k] skončiť zvislo
                if (dynamic_vertical_path[i][k].empty()) continue;

                // Výpočet súčtu ventilov na novom vodorovnom úseku pomocou prefixov
                int current_sum = row_prefix[i][j - 1] - row_prefix[i][k];

                for (auto &prev_state : dynamic_vertical_path[i][k]) {
                    // Podmienka klesajúcej postupnosti: nový úsek musí byť menší ako predošlý
                    if (prev_state.last_sum > current_sum) {
                        State new_state;
                        new_state.last_sum = current_sum;
                        new_state.total_valves = prev_state.total_valves + current_sum;
                        dynamic_horizontal_path[i][j].push_back(new_state);
                        // Vďaka zoradeniu stačí vziať prvý platný stav (najviac ventilov)
                        break;
                    }
                }
            }

            // Optimalizácia horizontálnych stavov v bunke (ponechanie len Pareto-optimálnych stavov)
            if (!dynamic_horizontal_path[i][j].empty()) {
                std::sort(dynamic_horizontal_path[i][j].begin(), dynamic_horizontal_path[i][j].end(), compare);
                int p = 0;
                int max_limit = -1;
                for (const auto& s : dynamic_horizontal_path[i][j]) {
                    // Ak má stav menej ventilov, musí mať aspoň väčší last_sum, aby bol perspektívny
                    if (s.last_sum > max_limit) {
                        dynamic_horizontal_path[i][j][p++] = s;
                        max_limit = s.last_sum;
                    }
                }
                dynamic_horizontal_path[i][j].resize(p);
            }

            // 2. SKÚŠAME VERTIKÁLNY POHYB (prichádzame z bodu [k][j] do [i][j])
            for (int k = 0; k < i; k++) {
                // Aby sme teraz išli zvislo, museli sme v [k][j] skončiť vodorovne
                if (dynamic_horizontal_path[k][j].empty()) continue;

                // Výpočet súčtu ventilov na novom zvislom úseku
                int current_sum = column_prefix[i - 1][j] - column_prefix[k][j];

                for (auto &prev_state : dynamic_horizontal_path[k][j]) {
                    if (prev_state.last_sum > current_sum) {
                        State new_state;
                        new_state.last_sum = current_sum;
                        new_state.total_valves = prev_state.total_valves + current_sum;
                        dynamic_vertical_path[i][j].push_back(new_state);
                        break;
                    }
                }
            }

            // Optimalizácia vertikálnych stavov v bunke
            if (!dynamic_vertical_path[i][j].empty()) {
                std::sort(dynamic_vertical_path[i][j].begin(), dynamic_vertical_path[i][j].end(), compare);
                int p = 0;
                int max_limit = -1;
                for (const auto& s : dynamic_vertical_path[i][j]) {
                    if (s.last_sum > max_limit) {
                        dynamic_vertical_path[i][j][p++] = s;
                        max_limit = s.last_sum;
                    }
                }
                dynamic_vertical_path[i][j].resize(p);
            }
        }
    }

    // FINÁLNY VÝSLEDOK: hľadáme maximum v poslednej bunke matice
    int final_result = 0;

    // Cesta mohla skončiť vodorovným príjazdom
    for (const auto& s : dynamic_horizontal_path[num_row-1][num_column-1]) {
        if (s.total_valves > final_result) final_result = s.total_valves;
    }
    // Alebo zvislým príjazdom
    for (const auto& s : dynamic_vertical_path[num_row-1][num_column-1]) {
        if (s.total_valves > final_result) final_result = s.total_valves;
    }

    std::cout << final_result;

    // Výpočet a voliteľný výpis času behu programu
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
}