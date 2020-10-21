#include <vector>
#include <iostream>
#include <string>
#include <map>

struct Active {
    int node, edge, len;
};

class SuffixTree {
    std::string input_text;

    int root, remainder, cur_pos;
    int node_need_suf_link;
    //для вывода
    int cur_number;
    //размеры строк
    int size1, size2;
    Active active{0, 0, 0};

    struct Node {
        //ребро(2 позиции в тексте)
        int start, end;
        int suf_link;
        //для вывода
        int parent, number;
        std::map<char, int> children;

        Node(int st = 0, int en = 0, int sl = 0) :
                start(st),
                end(en),
                suf_link(sl),
                parent(-1),
                number(-1) {}

        int len(int pos) {
            return end - start;
        }
    };

    std::vector<Node> tree;

public:
    SuffixTree(std::string &text, int size_1, int size_2) {
        size1 = size_1, size2 = size_2;

        input_text = text;
        remainder = 0, cur_pos = 0, node_need_suf_link = 0;
        cur_number = 0;
        root = active.node = insert_node(-1, -1);
    }

    //добавление суф ссылки
    void add_suf_link(int node);

    //вставка узла
    int insert_node(int start, int end);

    //построение дерева
    void build();

    //прохождение вниз
    bool go_down(int cur_node);

    //добавление символа
    void add_symb(char c);

    //вывод в задаче
    void numerate();

    //обход в глубину
    void dfs(int cur_node, int cur_from);
};


int SuffixTree::insert_node(int start, int end) {
    Node new_node(start, end);
    tree.push_back(new_node);
    return tree.size() - 1;
}

void SuffixTree::build() {
    for (auto c : input_text) {
        add_symb(c);
    }
}

bool SuffixTree::go_down(int cur_node) {
    if (active.len >= tree[cur_node].len(cur_pos)) {
        //переносим точку в тек позицию
        active.node = cur_node;
        int cur_len = tree[cur_node].len(cur_pos);
        //фиксим активные величины
        active.len -= cur_len;
        active.edge += cur_len;
        return true;
    }
    return false;
}

void SuffixTree::add_suf_link(int node) {
    //если это не корень
    if (node_need_suf_link != 0)
        tree[node_need_suf_link].suf_link = node;
    node_need_suf_link = node;
}

void SuffixTree::add_symb(char c) {
    node_need_suf_link = 0;
    ++remainder;

    while (remainder > 0) {
        if (active.len == 0) {
            active.edge = cur_pos;
        }

        //если переход из активного узла по символу есть
        bool exist = true;
        try {
            tree[active.node].children.at(input_text[active.edge]);
        }
        catch (const std::out_of_range &e) {
            exist = false;
        }
        if (exist) {
            int next_node = tree[active.node].children[input_text[active.edge]];

            //спускаемся
            if (go_down(next_node)) {
                //идем на следуюший шаг
                continue;
            }

            if (input_text[tree[next_node].start + active.len] == c) {
                ++active.len;
                add_suf_link(active.node);
                break;
            }
            //если мы дошли до сюда, значит не нашли такого символа
            //ну что ж, вставляем
            int inserted = insert_node(tree[next_node].start, tree[next_node].start + active.len);
            //переход, которого мы не нашли
            tree[active.node].children[input_text[active.edge]] = inserted;
            //лист с конечной границей на бесконечности
            int leaf_node = insert_node(cur_pos, input_text.length());
            tree[inserted].children.insert({c, leaf_node});

            //теперь он начинается на ребро позже
            tree[next_node].start += active.len;
            //перенос старого куска
            tree[inserted].children[input_text[tree[next_node].start]] = next_node;

            //добавляем суф ссылку
            add_suf_link(inserted);
        } else {
            //если перехода из активного узла по символу нет
            //добавление листа
            int leaf = insert_node(cur_pos, input_text.length());
            tree[active.node].children[input_text[active.edge]] = leaf;
            //добавление суф ссылки
            add_suf_link(active.node);
        }
        --remainder;

        //одно из правил, если активным узлом остался корень
        if (active.node == root && active.len > 0) {
            --active.len;
            active.edge = cur_pos - remainder + 1;
        } else {
            //переносим по ссылке активный узел
            active.node = tree[active.node].suf_link;
        }
    }
    ++cur_pos;
}

void SuffixTree::numerate() {
    std::cout << tree.size() << '\n';
    dfs(0, -1);
}

void SuffixTree::dfs(int cur_node, int cur_from) {
    tree[cur_node].parent = cur_from;
    tree[cur_node].number = cur_number;
    ++cur_number;
    cur_from = tree[cur_node].number;

    /////вывод
    if (cur_node != 0) {
        //родитель
        std::cout << tree[cur_node].parent << " ";
        int delta = 0, str = 0;
        if (tree[cur_node].start >= size1)
            delta = size1, str = 1;
        //номер строки
        std::cout << str << " ";
        //начало ребра
        std::cout << tree[cur_node].start - delta << " ";
        //конец
        if (tree[cur_node].end == input_text.length()) {
            if (str)
                std::cout << size2 << " ";
            else
                std::cout << size1 << " ";
        } else
            std::cout << tree[cur_node].end - delta << " ";

        std::cout << '\n';
    }
    /////
    //перебираем детей в лескикограф порядке
    for (auto next : tree[cur_node].children) {
        int next_node = next.second;
        if (tree[next_node].number == -1)
            dfs(next.second, cur_from);
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::string text1, text2, text;
    std::cin >> text1 >> text2;
    int size1 = text1.size(), size2 = text2.size();
    text = text1 + text2;

    SuffixTree t(text, size1, size2);
    t.build();
    t.numerate();
}


/*
 abcadbc$
*/

/*
 abcad$
 */

/*
 ab$
 ac#
 */

/*
 abc$
 adbc#
 */
/*
 aba$
baab#
 */

