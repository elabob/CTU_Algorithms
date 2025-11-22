//
// Created by bobenade on 22/11/2025.
//

#include <iostream>
#include <vector>
#include <algorithm>

bool in_range(int x, int min, int max) {
    return min <= x && x <= max;
}

struct Node {
    int key;
    Node *left;
    Node *right;
    bool deleted;   // true = uzol je oznaceny ako zmazany
    int height;     // vyska uzla v strome

    Node(int k): key(k), left(NULL), right(NULL), deleted(false), height(0) {}
};

class LazyBST {
private:
    Node *root;
    int compact_count;   //pocet volani Compact
    long long deleted_sum;    //sucet vysok zmazanych uzlov
    long long active_sum;     //sucet vysok aktivnych uzlov
    int tree_depth;     //aktualna hlbka stromu
    int deleted_count;      //pocet zmazanych uzlov
    int active_count;   //pocet aktivnych uzlov

    //aktualizujem vysku uzla podla jeho deti
    void update_height(Node* node) {
        if (node == NULL) return;
        int left_h = node->left ? node->left->height : -1;
        int right_h = node->right ? node->right->height : -1;
        node->height = 1 + std::max(left_h, right_h);
    }

    //pomocna funkcia pre insert - rekurzivne hladam miesta
    Node* insert_helper(Node* node, int key, bool& found_deleted, bool& added_new, int& new_node_dist, int dist) {
        if (node == NULL) {
            //vytvorim novy uzol
            added_new = true;
            new_node_dist = dist;  //ulozim si vzdialenost
            return new Node(key);
        }

        if (key == node->key) {
            //kluc uz existuje
            if (node->deleted) {
                node->deleted = false; //obnovim zmazany uzol
                found_deleted = true;
                //presuniem vysku z deleted do active
                int h = tree_depth - dist;
                deleted_sum -= h;
                active_sum += h;
                deleted_count--;
                active_count++;
            }
            return node;
        }

        if (key < node->key) {
            node->left = insert_helper(node->left, key, found_deleted, added_new, new_node_dist, dist + 1);
        } else {
            node->right = insert_helper(node->right, key, found_deleted, added_new, new_node_dist, dist + 1);
        }

        update_height(node);

        return node;
    }

    //pomocna funkcia pre delete
    Node* delete_helper(Node* node, int key, bool& found, int dist) {
        if (node == NULL) {
            return NULL;    //kluc neexistuje
        }

        if (key == node->key) {
            if (!node->deleted) {
                node->deleted = true;   //oznaceny ako zmazany
                found = true;
                //presuniem vysku z active do deleted
                int h = tree_depth - dist;
                active_sum -= h;
                deleted_sum += h;
                active_count--;
                deleted_count++;
            }
            return node;
        }

        if (key < node->key) {
            node->left = delete_helper(node->left, key, found, dist + 1);
        } else {
            node->right = delete_helper(node->right, key, found, dist + 1);
        }

        return node;
    }

public:
    LazyBST(): root(NULL), compact_count(0), deleted_sum(0), active_sum(0), tree_depth(-1), deleted_count(0), active_count(0) {}

    void insert(int key) {
        int old_depth = tree_depth;
        bool found_deleted = false;
        bool added_new = false;
        int new_node_dist = 0;

        root = insert_helper(root, key, found_deleted, added_new, new_node_dist, 0);

        //zistim novu hlbku
        int new_depth = root ? root->height : -1;

        if (added_new) {
            //pridala som novy uzol
            active_count++;

            if (new_depth != old_depth) {
                //hlbka sa zmenila
                //vsetky stare vysky sa zvysili o (new_depth - old_depth)
                int diff = new_depth - old_depth;
                active_sum += (active_count - 1) * diff;  // -1 lebo novy uzol este nemal vysku
                deleted_sum += deleted_count * diff;

                //pridam vysku noveho uzla s novou hlbkou
                tree_depth = new_depth;
                int h = tree_depth - new_node_dist;
                active_sum += h;
            } else {
                //hlbka sa nezmenila, len pridam vysku noveho uzla
                tree_depth = new_depth;
                int h = tree_depth - new_node_dist;
                active_sum += h;
            }
        } else {
            //nezmenil sa pocet uzlov, len obnovil som deleted
            tree_depth = new_depth;
        }

        check_and_compact();
    }

    void remove(int key) {
        bool found = false;
        root = delete_helper(root, key, found, 0);
        check_and_compact();
    }

    int get_depth() {
        return tree_depth;
    }

    int get_compact_count() {
        return compact_count;
    }

private:
    //pomocna funkcia
    void check_and_compact() {
        if (root == NULL) {
            return;
        }

        if (deleted_sum > active_sum) {
            compact();
        }
    }

    //zozbieram nezmazane kluce v pre-order poradi
    void collect_preorder(Node *node, std::vector<int>& keys) {
        if (node == NULL) {
            return;
        }

        if (!node->deleted) {
            keys.push_back(node->key);
        }

        collect_preorder(node->left, keys);
        collect_preorder(node->right, keys);
    }

    //postavim novy BST z klucov v pre-order poradi
    Node* build_tree_from_preorder(std::vector<int>& keys, int& index, int min_val, int max_val) {
        if (index >= (int)keys.size()) {
            return NULL;
        }

        int key = keys[index];

        if (key < min_val || key > max_val) {
            return NULL;
        }

        index++;
        Node* node = new Node(key);

        node->left = build_tree_from_preorder(keys, index, min_val, key - 1);
        node->right = build_tree_from_preorder(keys, index, key + 1, max_val);

        update_height(node);

        return node;
    }

    void compact() {
        //prebudujem strom
        std::vector<int> keys;
        collect_preorder(root, keys);

        //vymazem stary strom
        delete_tree(root);
        root = NULL;

        //postavim novy strom zo zozbieranych klucov (pre-order rebuild)
        if (!keys.empty()) {
            int index = 0;
            root = build_tree_from_preorder(keys, index, 1, 10000);
        }

        compact_count++;

        tree_depth = root ? root->height : -1;

        //prepocitam sucty po compacte
        deleted_sum = 0;
        active_sum = 0;
        deleted_count = 0;
        active_count = (int)keys.size();

        if (root != NULL) {
            sum_heights_after_compact(root, tree_depth, 0);
        }
    }

    void sum_heights_after_compact(Node *node, int depth, int distance) {
        if (node == NULL) {
            return;
        }

        active_sum += (depth - distance);

        sum_heights_after_compact(node->left, depth, distance + 1);
        sum_heights_after_compact(node->right, depth, distance + 1);
    }

    //uvolnim pamat
    void delete_tree(Node *node) {
        if (node == NULL) {
            return;
        }
        delete_tree(node->left);
        delete_tree(node->right);
        delete node;
    }
};

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    //N = pocet operacii
    int num_operations;
    std::cin >> num_operations;

    LazyBST tree;   //vytvorim strom

    //O = typ operacie
    //K = kluc
    for (int i = 0; i < num_operations; i++) {
        std::string operation;
        int key;

        std::cin >> operation >> key;

        if (!(in_range(num_operations, 1, 2.5 * 100000) && in_range(key, 1, 10000))) {
            std::cout << "wrong input\n";
            return 1;
        }

        if (operation == "ins") {
            tree.insert(key);
        } else if (operation == "del") {
            tree.remove(key);
        }
    }

    std::cout << tree.get_compact_count() << " " << tree.get_depth();
}