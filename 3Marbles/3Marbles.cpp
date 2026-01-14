//
// Created by bobenade on 03/11/2025.
//

/*
  *vnutorne uzly = biele (0) , cervene (1)
   listy = modre (2)

Musi:
    ZACINAT aj KONCIT modrou (2)
    medzi zacinajucim a konciacim je iba biela (0) / cervena(1)
    NIKDY nemas tri rovnake farby za sebou (napr. 000 / 111 / 222 (aj ked dvojky su len na koncoch)

Hladam:
    Najdlhsiu cestu v binarnom strome co to splna
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>

// Struktura pre uzol stromu - reprezentuje jeden "bod" v grafe
struct Node {
    int colour;  // farba uzla: 0=biela, 1=cervena, 2=modra
    int left;    // index laveho potomka v poli 'tree' (-1 ak neexistuje)
    int right;   // index praveho potomka v poli 'tree' (-1 ak neexistuje)
};

// Informacie o ceste - tuto strukturu vraciame z kazdeho uzla smerom hore (Bottom-up)
struct PathInfo {
    int length;        // aktualna dlzka cesty v pocte uzlov
    int last_colour;   // farba posledneho pridaneho uzla (kvoli kontrole 3 za sebou)
    int streak;        // kolko uzlov tejto farby je momentalne za sebou na konci cesty
};

int num_marbles;
std::vector<Node> tree;
int best = 0;  // Globalna premenna pre ulozenie doteraz najdlhsej najdenej cesty

// DFS prechadzam strom a vraciam dve mozne cesty pre kazdy uzol
// (potrebujem dve verzie kvoli roznemu spojeniu ciest)
// PRINCIP: V binarnom strome moze cesta prechadzat cez uzol smerom hore (do rodica)
// alebo sa v uzle "zlomit" a spojit lavy a pravy podstrom.
std::vector<PathInfo> DFS(int node_idx) {
    // Bazicky stav rekurzie: Ak neexistuje uzol, vratim prazdne cesty (dlzka 0)
    if (node_idx == -1) return {{0, -1, 0}, {0, -1, 0}};

    int colour = tree[node_idx].colour;

    // Modry uzol (2) - podla pravidiel moze byt len na zaciatku alebo konci cesty.
    // Kedze modra nesmie byt v strede, cesta tu vzdy bud zacina, alebo konci.
    if (colour == 2) {
        best = std::max(best, 1); // Cesta dlzky 1 (len tento uzol) je tiez platna
        PathInfo p = {1, 2, 1};
        return {p, p};  // Vrati cestu zacinajucu modrou (dlzka 1, farba 2, streak 1)
    }

    // Rekurzivne spracujem lavy a pravy podstrom - ziskam najlepsie cesty zdola
    std::vector<PathInfo> left = DFS(tree[node_idx].left);
    std::vector<PathInfo> right = DFS(tree[node_idx].right);

    // Vyberiem najlepsie platne cesty z oboch podstromov, ktore mozu pokracovat do aktualneho uzla
    std::vector<PathInfo> possible_paths = {{0, -1, 0}, {0, -1, 0}};

    // Vyberiem dlhsiu platnu cestu z laveho podstromu
    // Kontrolujem ci uz nemam 2 rovnake farby za sebou (potom by tretia bola neplatna)
    if (left[0].length >= left[1].length) {
        // Kontrola: Ak by pridanie aktualneho uzla vytvorilo streak 3, musim skusit alternativnu cestu
        if (!(left[0].last_colour == colour && left[0].streak >= 2)) {
            possible_paths[0] = left[0];
        } else {
            possible_paths[0] = left[1];  // prva cesta nevyhovuje pravidlu "max 2 rovnake", beriem druhu
        }
    } else {
        if (!(left[1].last_colour == colour && left[1].streak >= 2)) {
            possible_paths[0] = left[1];
        } else {
            possible_paths[0] = left[0];
        }
    }

    // To iste pre pravy podstrom - snazime sa najst najdlhsiu cestu, ktora moze legalne pokracovat nahor
    if (right[0].length >= right[1].length) {
        if (!(right[0].last_colour == colour && right[0].streak >= 2)) {
            possible_paths[1] = right[0];
        } else {
            possible_paths[1] = right[1];
        }
    } else {
        if (!(right[1].last_colour == colour && right[1].streak >= 2)) {
            possible_paths[1] = right[1];
        } else {
            possible_paths[1] = right[0];
        }
    }

    PathInfo left_path = possible_paths[0];
    PathInfo right_path = possible_paths[1];

    // LOGIKA "ZALOMENIA" CESTY: Pokusim sa spojit cesty z laveho a praveho podstromu cez aktualny uzol.
    // Toto vytvori cestu v tvare "obrateneho V", ktora uz nemoze ist vyssie k rodicovi.
    if (left_path.length > 0 && right_path.length > 0) {
        bool can_connect = false;

        // Kontrolujem ci mozem spojit cesty bez vytvorenia troch rovnakych za sebou (napr. ...11 + [1] + 1...)
        if ((right_path.last_colour != colour) && (left_path.last_colour != colour)) {
            // Obe cesty maju inu farbu ako aktualny uzol - spojenie je vzdy bezpecne
            can_connect = true;
        } else if (left_path.last_colour == colour && left_path.streak < 2 && right_path.last_colour != colour) {
            // Lava cesta ma na konci rovnaku farbu (ale len 1x), prava je ina - mozem spojit
            can_connect = true;
        } else if (right_path.last_colour == colour && right_path.streak < 2 && left_path.last_colour != colour) {
            // Prava cesta ma na konci rovnaku farbu (ale len 1x), lava je ina - mozem spojit
            can_connect = true;
        }
        // Poznamka: Ak by mali obe strany na konci 'colour' so streakom 1, vznikol by streak 3 (1+1+1), co je zakazane.

        if (can_connect) {
            // Spocitam celkovu dlzku spojenej cesty (+1 za aktualny uzol) a aktualizujem globalne maximum
            int together = 1 + left_path.length + right_path.length;
            best = std::max(best, together);
        }
    }

    // PRIPRAVA CESTY PRE RODICA: Pridam aktualny uzol k cestam z podstromov a updatnem ich vlastnosti
    if (possible_paths[0].length > 0) {
        if (possible_paths[0].last_colour == colour && possible_paths[0].streak < 2) {
            // Mam rovnaku farbu ako koniec cesty podo mnou, ale este len 1x (streak bude 2)
            possible_paths[0].streak++;
            possible_paths[0].length++;
        } else if (possible_paths[0].last_colour != colour) {
            // Pridavam inu farbu, resetujem streak na 1
            possible_paths[0].last_colour = colour;
            possible_paths[0].streak = 1;
            possible_paths[0].length++;
        } else {
            // Uz tam boli 2 rovnake za sebou, pridanie tretej by porusilo pravidla - cesta zanika
            possible_paths[0] = {0, -1, 0};
        }
    }

    // To iste pre druhu moznu cestu (aby sme mali vzdy dve alternativy pre rodica)
    if (possible_paths[1].length > 0) {
        if (possible_paths[1].last_colour == colour && possible_paths[1].streak < 2) {
            possible_paths[1].streak++;
            possible_paths[1].length++;
        } else if (possible_paths[1].last_colour != colour) {
            possible_paths[1].last_colour = colour;
            possible_paths[1].streak = 1;
            possible_paths[1].length++;
        } else {
            possible_paths[1] = {0, -1, 0};
        }
    }

    // Vratim dve upravene cesty, ktore reprezentuju najdlhsie legalne vetvy iduce z tohto uzla dole
    return possible_paths;
}

// Funkcia na vybudovanie struktury stromu zo vstupu
void build_tree() {
    tree.resize(num_marbles);

    // Nacitam farby vsetkych uzlov (v poradi ako su v strome)
    for (int i = 0; i < num_marbles; i++) {
        std::cin >> tree[i].colour;
        tree[i].left = tree[i].right = -1;
    }

    // Postavim binarny strom pomocou BFS - uzly sa pridavaju postupne po urovniach (Level-order)
    std::queue<int> q;
    q.push(0);  // zacinam od korena (index 0)
    int next = 1;  // index dalsieho volneho uzla z pola 'tree', ktory bude dietatom

    while (!q.empty() && next < num_marbles) {
        int curr = q.front(); // zoberiem prvy uzol v poradi, ktory potrebuje deti
        q.pop();

        // Modry uzol (2) je podla zadania list, takze mu nepripajame ziadne deti
        if (tree[curr].colour == 2) continue;

        // Pridam laveho potomka, ak este mame k dispozicii nepriradene uzly
        if (next < num_marbles) {
            tree[curr].left = next++;
            q.push(tree[curr].left);
        }
        // Pridam praveho potomka
        if (next < num_marbles) {
            tree[curr].right = next++;
            q.push(tree[curr].right);
        }
    }
}

int main() {
    // Zakladny I/O setup
    if (!(std::cin >> num_marbles)) return 0;

    build_tree();  // Krok 1: Transformacia vstupu na pamatovu strukturu stromu

    DFS(0);  // Krok 2: Prehladavanie do hlbky a dynamicke vypocitavanie najdlhsej cesty

    // Vypis vysledku
    std::cout << best;
    return 0;
}