#include <vector>
#include <iostream>
#include <string>
#include <map>

struct SuffixArray {
    size_t dif_classes, n;
    std::string input_text;
    std::vector<int> suf_array, suf_lcp;
    std::vector<int> classes;

    static char const separator = '$';
    static int const alphabet_size = 26;

    //непосредственно построение суф массива
    SuffixArray(const std::string &text);

    //нулевой шаг - сортировка подсчетом первых символов
    size_t zero_step(const std::map<int, std::vector<int>> &indexes);

    void cur_step(size_t k, std::vector<int> &poses_second, std::vector<int> &classes_second);
    //наибольший общий префикс
    std::vector<int> LCP();

    //нахождение количества различных подстрок
    size_t dif_substrings();
};

size_t SuffixArray::zero_step(const std::map<int, std::vector<int>> &indexes) {
    size_t dif_classes = 0, pos_i = 0;

    for (auto [sym, ind]: indexes) {
        //pair.first -- символ
        for (auto i : ind) {
            suf_array[pos_i] = i;
            ++pos_i;
            classes[i] = dif_classes;
        }
        //заполнили один класс
        ++dif_classes;
    }

    return dif_classes;
}

void SuffixArray::cur_step(size_t k, std::vector<int> &poses_second, std::vector<int> &classes_second) {
    //если все суф в разных классах
    if (dif_classes == n)
        return;

    int count_size = n > alphabet_size ? n : alphabet_size;
    std::vector<int> count(count_size);

    //poses_second - сортировка по вторым частям
    for (size_t i = 0; i < n; i++)
        poses_second[i] = (suf_array[i] + n - (1 << k)) % n;

    //сортировка
    for (size_t i = 0; i < n; i++) {
        ++count[classes[poses_second[i]]];
    }

    for (size_t i = 1; i < dif_classes; ++i)
        count[i] += count[i - 1];

    for (int i = n - 1; i >= 0; --i) {
        --count[classes[poses_second[i]]];
        int cnt = count[classes[poses_second[i]]];
        suf_array[cnt] = poses_second[i];
    }

    //имеем новые позиции, проставим классы
    classes_second[suf_array[0]] = 0;
    dif_classes = 1;

    for (size_t i = 1; i < n; ++i) {
        int right_2 = (suf_array[i] + (1 << k)) % n, right_1 = (suf_array[i - 1] + (1 << k)) % n,
                left_1 = suf_array[i - 1], left_2 = suf_array[i];

        //за O(1) понимаем, в разных классах суффиксы лежат или в одном
        if (classes[left_1] != classes[left_2] || classes[right_1] != classes[right_2])
            ++dif_classes;

        classes_second[suf_array[i]] = dif_classes - 1;
    }
    classes.swap(classes_second);
}

SuffixArray::SuffixArray(const std::string &text) {
    //количество различных классов на данной итерации
    dif_classes = 0;
    input_text = text;
    input_text += separator;
    n = input_text.size();

    suf_array.resize(n), classes.resize(n);
    std::vector<int> poses_second(n), classes_second(n);
    //соответсвие символа и позициям, на которых этот символ встречается
    std::map<int, std::vector<int>> indexes;
    for (size_t i = 0; i < n; ++i) {
        indexes[input_text[i]].push_back(i);
    }
    //0 - этап, сортируем по первым 2^0=1 символу
    dif_classes = zero_step(indexes);

    for (size_t k = 0; (1 << k) < n; ++k) {
        cur_step(k, poses_second, classes_second);
    }
}

//алгоритм Касаи
std::vector<int> SuffixArray::LCP() {
    size_t n = input_text.size(), cur = 0;
    std::vector<int> lcp(n - 1);
    std::vector<int> suf_pos(n);

    for (size_t i = 0; i < n; ++i) {
        suf_pos[suf_array[i]] = i;
    }

    for (size_t i = 0; i < n; ++i) {
        if (suf_pos[i] != n - 1) {
            //следующий суффикс
            int j = suf_array[suf_pos[i] + 1];
            //жадно добираем
            while (i + cur < n && j + cur < n && input_text[i + cur] == input_text[j + cur])
                ++cur;

            lcp[suf_pos[i]] = cur;
            if (cur != 0)
                --cur;
        } else
            cur = 0;
    }

    suf_lcp = std::move(lcp);
    return lcp;
}

size_t SuffixArray::dif_substrings(){
    // lcp и suf_array
    size_t res = 0;

    for(size_t i = 0; i < n; ++i){
        if(i != n - 1){
            res += (n - suf_array[i] - suf_lcp[i]);
        }
        else
            res += (n - suf_array[i]);
    }
    //так как последний символ не принадлежит строке
    res -= n;

    return res;
}

int main() {
    std::string text; std::cin >> text;
    SuffixArray s(text);
    s.LCP();
    std::cout << s.dif_substrings();
}
