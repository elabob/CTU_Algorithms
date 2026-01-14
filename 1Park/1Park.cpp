//
// Created by bobenade on 24/09/2025.
//

#include <iostream>
#include <vector>

// Funkcia na kontrolu, ci je hodnota x v uzavretom intervale [low, high]
bool in_range(long long x, long long low, long long high){
    return low <= x && x <= high;
}

// Specificka kontrola rozsahu pre skaly (pravdepodobne kvoli podmienke L < K/2)
bool in_range_rocks (long long x, long long low, long long high){
    return low <= 2*x && 2*x < high;
}

int main() {
    // Premenne pre rozmery a podmienky zadania
    int main_area, park_area, distance, rocks;
    /*N = main_area, (velkost celej mapy)
     *K = park_area, (velkost vyrezu - parku)
     *L = distance, (vzdialenost od kraja parku pre stredovu zonu)
     *S = rocks (minimalny pocet skal v stredovej zone) */
    std::cin >> main_area >> park_area >> distance >> rocks;

    // Validacia vstupov podla zadanych rozsahov
    if (in_range(main_area, 1, 3000) &&
        in_range(park_area, 1, main_area) &&
        in_range_rocks(distance, 0, park_area) &&
        in_range(rocks, 1, 15000)) {

        // Hlavna matica mapy (2D vektor)
        std::vector<std::vector<int>> matrix_main;

        // Nacitanie hlavnej matice zo vstupu (plnenie riadok po riadku)
        for (int i = 0; i < main_area; i++) {
            std::vector<int> row;
            for (int j = 0; j < main_area; j++){
                int number;
                std::cin >> number;
                row.push_back(number);
            }
            matrix_main.push_back(row);
        }

        // Vytvorenie prefixovych matic s velkostou (N+1)x(N+1) inicializovanych nulami
        // prefix_forest[i][j] = pocet stromov od (0,0) po (i-1,j-1)
        // prefix_rocks[i][j] = pocet skal od (0,0) po (i-1,j-1)
        std::vector<std::vector<int>> prefix_forest(main_area + 1, std::vector<int>(main_area + 1, 0));
        std::vector<std::vector<int>> prefix_rocks(main_area + 1, std::vector<int>(main_area + 1, 0));

        // Naplnenie prefixovych matic pomocou dynamickeho programovania
        for (int row_index=1; row_index <= main_area; row_index++) {
            for (int col_index=1; col_index <= main_area; col_index++) {

                // Kontrola, ci je dana bunka strom (hodnota 1 v matici)
                bool is_forest = (matrix_main[row_index-1][col_index-1] == 1);

                // Vzorec pre 2D prefixny sucet (forest)
                prefix_forest[row_index][col_index] = is_forest
                    +prefix_forest[row_index-1][col_index] // sucet nad
                    +prefix_forest[row_index][col_index-1] // sucet vlavo
                    -prefix_forest[row_index-1][col_index-1]; // odpocitanie prieniku

                // Kontrola, ci je dana bunka skala (hodnota 2 v matici)
                bool is_rocks = (matrix_main[row_index-1][col_index-1] == 2);

                // Vzorec pre 2D prefixny sucet (rocks)
                prefix_rocks[row_index][col_index] = is_rocks
                +prefix_rocks[row_index-1][col_index] // sucet nad
                +prefix_rocks[row_index][col_index-1] // sucet vlavo
                -prefix_rocks[row_index-1][col_index-1]; // odpocitanie prieniku
            }
        }

        int max_forest = 0; // Premenna pre ukladanie doteraz najlepsiieho vysledku

        // Prechadzanie vsetkych moznych pozicii parku velkosti KxK
        for (int start_row = 0; start_row <= main_area - park_area; start_row++) {
            for (int start_col = 0; start_col <= main_area - park_area; start_col++) {

                // Urcenie suradnic parku v hlavnej matici
                int top_left_row = start_row;
                int top_left_col = start_col;
                int bottom_right_row = start_row + park_area;
                int bottom_right_col = start_col + park_area;

                // Vypocet stromov v celom parku pomocou Inclusion-Exclusion (O(1))
                int forest_in_park = prefix_forest[bottom_right_row][bottom_right_col]
                                    -prefix_forest[top_left_row][bottom_right_col]
                                    -prefix_forest[bottom_right_row][top_left_col]
                                    +prefix_forest[top_left_row][top_left_col];

                // Vypocet skal v strede (vnutorny stvorec orezany o 'distance' z kazdej strany)
                int rocks_in_center = prefix_rocks[bottom_right_row-distance][bottom_right_col-distance]
                                    -prefix_rocks[top_left_row+distance][bottom_right_col-distance]
                                    -prefix_rocks[bottom_right_row-distance][top_left_col+distance]
                                    +prefix_rocks[top_left_row+distance][top_left_col+distance];


                // Aktualizacia maxima, ak je v strede dostatok skal
                if (rocks_in_center >= rocks) {
                    max_forest = std::max(max_forest, forest_in_park);
                }

                // Ponechane zakomentovane povodne casti:
                // std::cout << "Counter_rocks:" << counter_rocks << " ";
                // std::cout << "Counter_forest:" << counter_forest << "\n";
            }
    }

        // Vypis finalneho vysledku
        std::cout << max_forest << "\n";
    } else {
        // Chybove hlasenie pri nevalidnom vstupe
        std::cerr << "Wrong input";
    }
}