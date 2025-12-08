//
// Created by bobenade on 08/12/2025.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <queue>

// potrebujem si pamatat stav celeho stroja pre kazdu moznost v rade
struct MachineState {
    int current_state;
    int head;
    std::vector<char> tape;
};

int main() {
    int num_conditions, num_tape_symbols, length_input;
    std::cin >> num_conditions >> num_tape_symbols >> length_input;

    std::vector<char> input_types(num_tape_symbols);
    for (int i = 0; i < num_tape_symbols; i++) {
        std::cin >> input_types[i];
        //std::cout << input_types[i] << " ";
    }

    bool blank = false;
    char blank_symbol; // povodne row_3
    std::cin >> blank_symbol;

    if (blank_symbol == 'B') {
        blank = true;
        //std::cout << "blank \n";
    }

    std::vector<int> turing_code;

    std::string raw_turing_code;
    std::cin >> raw_turing_code;  //nacitam cely dlllhy retazec znakov

    int count = 0;

    //zacinam od indexu 3 aby som preskocila uvodne "111"
    for (int i = 0; i < raw_turing_code.size(); i++) {
        if (raw_turing_code[i] == '0') {
            count++; //pocitam nuly
        } else {
            //ak som narazila na 1
            if (count > 0) {
                turing_code.push_back(count);
                count = 0;  //vynulujem counter pre dalsie cislo
            }
            //ak nasleduje dalsia 1 -> count je 0, taakze sa nic neulozi a len sa pocuniem dalej
        }
    }

    //kazda patica cisel predstavuje jednu hranu

    //vypis pre kontrolu:
    // for (int i = 0; i < turing_code.size(); i+=5) {
    //     std::cout << "Hrana z q "<<turing_code[i] << " citam " <<turing_code[i+1]<<" ->q "<<turing_code[i+2]<<" prepisem "<<turing_code[i+3]<<" smer "<<turing_code[i+4]<<"\n";
    // }

    std::vector<char> starting_condition(length_input);
    for (int i = 0; i < length_input; i++) {
        std::cin >> starting_condition[i];
    }
    //zisim ci je to D (deterministicky) alebo N (nedeterministicky) ---
    bool is_deterministic = true;
    // prechadzam vsetky pravidla a pozeram ci nie su dve rovnake pre ten isty stav a znak
    for (int i = 0; i < turing_code.size(); i += 5) {
        for (int j = i + 5; j < turing_code.size(); j += 5) {
            if (turing_code[i] == turing_code[j] && turing_code[i+1] == turing_code[j+1]) {
                is_deterministic = false; // nasla som dve rovnake startovacie podmienky
                break;
            }
        }
        if (!is_deterministic) break;
    }

    if (is_deterministic) std::cout << "D\n";
    else std::cout << "N\n";

    //simulacia
    std::queue<MachineState> bfs_queue; // povodne q
    // vlozim pociatocny stav
    bfs_queue.push({1, 0, starting_condition});

    std::vector<char> final_tape;
    bool found_end = false; // povodne nasla_som_koniec

    while (!bfs_queue.empty()) {
        // vyberiem aktualny stav z radu
        MachineState current_config = bfs_queue.front(); // povodne aktualny
        bfs_queue.pop();

        // ak som v koncovom stave 2, koncim
        if (current_config.current_state == 2) {
            final_tape = current_config.tape;
            found_end = true;
            break;
        }

        // najprv zistim aky znak je pod hlavickou
        char read_char;
        if (current_config.head >= 0 && current_config.head < current_config.tape.size()) {
            read_char = current_config.tape[current_config.head];
        } else {
            read_char = blank_symbol;  //ak som mimo je to BLANK
        }

        // dalej si prevediem znak na cislo
        int read_id = num_tape_symbols + 1; //defaultne to je blank
        for (int i = 0; i < num_tape_symbols; i++) {
            if (input_types[i] == read_char) {
                read_id = i + 1;
                break;
            }
        }

        // prejdem vsetky pravidla v turing_code a pridam do radu vsetky moznosti (pre NTM ich moze byt viac)
        for (int i = 0; i < turing_code.size(); i += 5) {
            if (turing_code[i] == current_config.current_state && turing_code[i+1] == read_id) {

                // vytvorim si kopiu stavu pre tuto moznost
                MachineState next_config = current_config; // povodne novy

                int new_state = turing_code[i+2];
                int write_id = turing_code[i+3];
                int move_dir = turing_code[i+4];

                // zapisem novy znak na pasku
                char write_char;
                if (write_id == num_tape_symbols + 1) write_char = blank_symbol;
                else write_char = input_types[write_id - 1];

                // osetrenie ak pisem mimo velkost pasky
                if (next_config.head < 0) {
                    next_config.tape.insert(next_config.tape.begin(), write_char);
                    next_config.head = 0; //vlozila som na zaciatok
                } else if (next_config.head >= next_config.tape.size()) {
                    next_config.tape.push_back(write_char);
                } else {
                    next_config.tape[next_config.head] = write_char;
                }

                // nastavim novy stav
                next_config.current_state = new_state;

                // posuniem hlavisku
                if (move_dir == 1) {
                    next_config.head++; // DOPRAVA
                } else {
                    next_config.head--; // DOLAVA
                }

                // ak som isla dolava pod nulu musim pridat dalsie miesto na zaciatok
                if (next_config.head < 0) {
                    next_config.tape.insert(next_config.tape.begin(), blank_symbol);
                    next_config.head = 0;
                }

                // pridam novu moznost do radu
                bfs_queue.push(next_config);
            }
        }
    }

    // vypis vysledku
    // vypisujem iba od prveho neblankoveho po posledny neblankovy
    if (found_end) {
        int start = -1, end = -1;

        // najdem zaciatok a koniec realnych dat
        for(int i=0; i<final_tape.size(); i++) {
            if(final_tape[i] != blank_symbol) {
                if(start == -1) start = i;
                end = i;
            }
        }

        if(start != -1) {
            for (int i = start; i <= end; i++) {
                std::cout << final_tape[i] << (i == end ? "" : " ");
            }
        }
        std::cout << "\n";
    }
}