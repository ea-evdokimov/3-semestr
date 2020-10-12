#include <iostream>
#include <string>
#include <vector>

class Trie {
private:
    static const int ALPHABET_SIZE = 26;
    //узел бора
    struct TrieNode {
        //длина вектора - размер алфавита
        std::vector<int> children;

        //массив переходов
        std::vector<int> go_to;

        bool is_terminal;
        //массив номеров шаблонов, заканчивающихся тут
        std::vector<int> end_templ;

        //суф ссылка
        int suf_link;

        //сжатая суф ссылка
        int comp_suf_link;

        //указатель на родителя узла
        int parent;

        //переход по какому символу между родителем и ребенком
        char char_to_parent;

        TrieNode(bool flag, int sl, int csl, int p, char cp) :
                is_terminal(flag),
                suf_link(sl),
                comp_suf_link(csl),
                parent(p),
                char_to_parent(cp),
                children(ALPHABET_SIZE, -1),
                go_to(ALPHABET_SIZE, -1) {}
    };

    //массив нодов, 0 - корень
    std::vector<TrieNode> t;
    //количество шаблонов, которые начинауются в этом индексе в исходном тексте
    // (если оно совпадает с числом шаблонов, этот индекс попадает в ответ)
    std::vector<int> count_templ;
    //вектор конечных позиций шаблонов без масок
    std::vector<int> templ_ends;
    //количество первых знаков вопроса и последних
    int last_questions;

    int get_suf_link(int cur);

    int get_go(int cur, char c);

    int get_compr_suf_link(int cur);

public:
    Trie() {
        //корень
        t.push_back(TrieNode(false, 0, 0, 0, 0));
        t[0].parent = t[0].suf_link = t[0].comp_suf_link = 0;
    }

    void build(const std::string &pattern);

    void add_word(const std::string &pattern, int start, int finish);

    void find_template(const std::string &text);

    void get_res(std::vector<int> &res);
};

//построение бора по шаблону с масками
void Trie::build(const std::string &pattern) {
    //start, finish - шаблона без ?
    size_t start = 0, finish = 0;
    int fin_q = 0;

    //знаки вопроса конце
    int len = pattern.length();
    for (int i = len - 1; i >= 0; --i) {
        if (pattern[i] == '?')
            ++fin_q;
        else
            break;
    }
    last_questions = fin_q;


    for (int i = 0; i < len; ++i) {
        if (pattern[i] == '?') {
            if (finish - start > 0) {
                //добавляем непустой шаблон
                add_word(pattern, start, finish);
                templ_ends.push_back(finish - 1);
            }
            ++finish;
            start = finish;
        } else {
            ++finish;
        }
    }

    if (finish - start > 0) {
        //добавляем непустой шаблон
        add_word(pattern, start, finish);
        templ_ends.push_back(finish - 1);
    }
}

//добавление шаблона без масок
void Trie::add_word(const std::string &pattern, int start, int finish) {
    int cur = 0;

    for (int i = start; i < finish; ++i) {
        int index = pattern[i] - 'a';
        if (t[cur].children[index] == -1) {
            t.push_back(TrieNode(false, -1, -1, cur, index));

            t[cur].children[index] = t.size() - 1;
        }
        cur = t[cur].children[index];
    }
    //конец паттерна
    t[cur].is_terminal = true;
    //номер шаблона, заканчивающегося тут
    t[cur].end_templ.push_back(templ_ends.size());
}

//непосредственно поиск шаблонов
void Trie::find_template(const std::string &text) {
    count_templ.resize(text.size());
    int cur = 0;

    int size = text.size();
    for (int j = 0; j < size - last_questions; ++j) {
        cur = get_go(cur, text[j] - 'a');

        int cur_save = cur;
        while (cur != 0) {
            if (t[cur].is_terminal) {
                //бежим по всем шаблонам, которые заканчиваются здесь
                for (auto k : t[cur].end_templ) {
                    if (j >= templ_ends[k])
                        ++count_templ[j - templ_ends[k]];
                }
            }
            cur = get_compr_suf_link(cur);
        }
        cur = cur_save;
    }
}

//вычисление сжатой суф ссылки(чтобы не было перехода - перехода - ...)
int Trie::get_compr_suf_link(int cur) {
    //если еще не вычислена
    if (t[cur].comp_suf_link == -1) {
        if (t[get_suf_link(cur)].is_terminal) {
            //если суф ссылка это лист
            t[cur].comp_suf_link = get_suf_link(cur);
        } else {
            //если корень
            if (get_suf_link(cur) == 0)
                t[cur].comp_suf_link = 0;
            else
                t[cur].comp_suf_link = get_compr_suf_link(get_suf_link(cur));
        }
    }
    return t[cur].comp_suf_link;
}

//вычисление массива переходов
int Trie::get_go(int cur, char c) {
    //если еще не вычисляли
    if (t[cur].go_to[c] == -1) {
        //если есть переход по символу
        if (t[cur].children[c] != -1)
            t[cur].go_to[c] = t[cur].children[c];
        else {
            if (cur == 0)
                t[cur].go_to[c] = 0;
            else
                t[cur].go_to[c] = get_go(get_suf_link(cur), c);
        }
    }
    return t[cur].go_to[c];
}

//вычисление суф ссылки
int Trie::get_suf_link(int cur) {
    //если еще не вычисляли
    if (t[cur].suf_link == -1) {
        //если это корень или родиетль корень
        if (cur == 0 || t[cur].parent == 0) {
            t[cur].suf_link = 0;
        } else {
            t[cur].suf_link = get_go(get_suf_link(t[cur].parent), t[cur].char_to_parent);
        }
    }
    return t[cur].suf_link;
}

//вывод индексов, в которых число равно количеству подшаблонов
void Trie::get_res(std::vector<int> &res) {
    int num_of_templ = templ_ends.size(), len = count_templ.size();

    //отдельно обрабатываем случай, когда вся строка из ?
    if (num_of_templ == 0)
        --last_questions;

    for (int i = 0; i < len - last_questions; ++i) {
        if (count_templ[i] == num_of_templ) {
            res.push_back(i);
        }
    }
}

int main() {
    Trie bor;
    std::string pattern, text;
    std::cin >> pattern >> text;
    //массив для ответа
    std::vector<int> res;

    if (text.length() >= pattern.length()) {
        bor.build(pattern);
        bor.find_template(text);
        bor.get_res(res);
    }

    for (auto i : res) {
        std::cout << i << " ";
    }
}

