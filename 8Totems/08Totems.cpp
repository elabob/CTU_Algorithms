//
// Created by bobenade on 30/12/2025.
//

#include <iostream>
#include <vector>
#include <cmath> //pre abs()
#include <algorithm> //pre min()

int main() {
    // N = num_villages (počet dedín)
    // price_for_totem (odmena/zisk za postavenie totemu)
    // price_for_fighter (pokuta/náklad za rozdiel v počte bojovníkov)
    int num_villages, price_for_totem, price_for_fighter;
    if (!(std::cin >> num_villages >> price_for_totem >> price_for_fighter)) return 0;

    std::vector<int> fighter_for_village;
    for (int i = 0; i < num_villages; i++) {
        int num_fighter;
        std::cin >> num_fighter;
        fighter_for_village.push_back(num_fighter);
    }

    // --- PÔVODNÉ ZAKOMENTOVANÉ ÚVAHY ---
    // KONTROLNY vypis matice
    // for (int x : fighter_for_village) {
    //     std::cout << x << ' ';
    // }
    // std::cout<< "\n";

    // for (int x: fighter_for_village) {
    //     if (x == (x+1)) {
    //         std::cout<< x << x+1;
    //     }
    //     else {
    //         std::cout<<"Nope";
    //     }
    // }

    // int expenses = 0;
    // for (int i = 0; i < num_villages - 1; i++) {
    //     int diff = abs(fighter_for_village[i] - fighter_for_village[i + 1]);
    //     if (diff == 0) {
    //         std::cout << "Totem je zadarmo" << std::endl;
    //     } else {
    //         expenses += price_for_totem * diff;
    //         std::cout << "Expenses for " << diff << " totem is: " << price_for_totem * diff << std::endl;
    //     }
    // }

    // if (expenses != 0) {
    //     std::cout << "Total expenses: " << expenses << std::endl;
    // } else {
    //     std::cout << "No expenses incurred." << std::endl;
    // }

    // int max_profit = price_for_totem * (num_villages -1);
    // std::cout << "Max profit: " << max_profit;
    // ------------------------------------

    // PREDVÝPOČET PREFIXOVÝCH SÚČTOV
    // sum[i] = celkový počet bojovníkov v dedinách od indexu 0 po i-1
    // Umožňuje rýchlo zistiť sumu bojovníkov v akomkoľvek intervale dedín za O(1)
    std::vector<int> sum(num_villages + 1, 0);
    for (int i = 0; i < num_villages; i++) {
        sum[i + 1] = sum[i] + fighter_for_village[i];
    }

    // DP TABUĽKA 1: INTERVALOVÉ DP
    // dp[i][j] = maximálny zisk, ktorý dosiahnem, ak zlúčim VŠETKY dediny od i po j do jednej jedinej skupiny
    std::vector<std::vector<int>> dp(num_villages, std::vector<int>(num_villages, 0));

    // Prechádzame dĺžky intervalov (od dĺžky 2 až po celú mapu)
    for (int len = 2; len <= num_villages; len++) {
        // Prechádzame štartovacie body intervalu i
        for (int i = 0; i <= num_villages - len; i++) {
            int j = i + len - 1; // Koncový index intervalu

            // Inicializácia na veľmi malé číslo (hľadáme maximum, môže vzniknúť aj strata)
            dp[i][j] = -1000000000;

            // Skúšame všetky miesta 'k', kde sa tento interval mohol rozdeliť na dva pred tým, než sa spojil posledným totemom
            for (int k = i; k < j; k++) {
                // Počet bojovníkov v lavej kope (dediny i až k)
                int left_fighters = sum[k + 1] - sum[i];
                // Počet bojovníkov v pravej kope (dediny k+1 až j)
                int right_fighters = sum[j + 1] - sum[k + 1];

                // Náklad za postavenie posledného totemu medzi týmito dvoma kopami
                int cost = price_for_fighter * abs(left_fighters - right_fighters);

                // Zisk za tento konkrétny krok: Odmena - Náklad
                int profit = price_for_totem - cost;

                // Ak sú pod-intervaly väčšie než 1 dedina, musíme pripočítať zisky z ich predchádzajúcich zlúčení
                if (k > i) {
                    profit += dp[i][k];
                }
                if (k + 1 < j) {
                    profit += dp[k + 1][j];
                }

                // Hľadáme také rozdelenie k, ktoré maximalizuje zisk pre celý interval [i, j]
                dp[i][j] = std::max(dp[i][j], profit);
            }
        }
    }

    // DP TABUĽKA 2: GLOBÁLNE MAXIMUM
    // best[i][j] = maximálny možný zisk na úseku i..j, pričom NIE SME nútení zlúčiť úplne všetko do jednej kopy
    // (niektoré totemy môžeme vynechať, ak by nám priniesli stratu)
    std::vector<std::vector<int>> best(num_villages, std::vector<int>(num_villages, 0));

    for (int len = 1; len <= num_villages; len++) {
        for (int i = 0; i <= num_villages - len; i++) {
            int j = i + len - 1;

            if (i == j) {
                best[i][j] = 0; // Jedna dedina = nie je čo spájať
                continue;
            }

            // Možnosť 1: Skúsime vziať výsledok, kde sme museli zlúčiť všetko (z prvej DP tabuľky)
            best[i][j] = dp[i][j];

            // Možnosť 2: Skúsime nájsť bod rozdelenia 'k', kde sa oplatí interval rozdeliť a riešiť dve časti
            // nezávisle (bez toho, aby sme ich spájali finálnym totemom)
            for (int k = i; k < j; k++) {
                best[i][j] = std::max(best[i][j], best[i][k] + best[k + 1][j]);
            }

            // Poznámka: Ak je najlepší profit záporný, v tabuľke 'best' sa zachová 0 (vďaka inicializácii),
            // pretože nič nerobiť je lepšie ako byť v strate.
        }
    }

    // Výsledok je v pravom hornom rohu tabuľky: najlepší možný zisk pre celú škálu dedín
    std::cout << best[0][num_villages - 1] << std::endl;

    return 0;
}