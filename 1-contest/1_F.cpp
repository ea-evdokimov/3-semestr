#include <vector>
#include <iostream>
#include <string>
#include <map>

char const separator1 = '_';
char const separator2 = '^';

struct SuffixArray {
    size_t dif_classes, n;
    std::string input_text;
    std::vector<int> suf_array, suf_lcp;
    std::vector<int> classes;

    //конец первой строки
    int end_first = -1;

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

    //нахождение к общей подстроки
    std::optional<std::string> k_substring(size_t k);

    //из разных ли строк данный префикс суффикса
    bool from_dif_strings(int j);
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

    //вспомогательные векторы для вторых частей
    int count_size = n > alphabet_size ? n : alphabet_size;

    std::vector<int> count(count_size);
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

bool SuffixArray::from_dif_strings(int j) {
    //первый раз посчитаем, где стоит разделитель
    if (end_first == -1) {
        end_first = input_text.find(separator1);
    }

    //нам подходит, если в одном знак '_' встречается, а в другом - нет
    return (suf_array[j] > end_first) & (suf_array[j + 1] < end_first) ||
           (suf_array[j] < end_first) & (suf_array[j + 1] > end_first);
}

std::optional<std::string> SuffixArray::k_substring(size_t k) {
    size_t n = input_text.size();
    std::string res;
    //текущий минимальный lcp и текущий
    size_t cur_min_lcp = 0, cur_k = 0;

    //идем по суффиксному массиву(отсортирован в лексикографическом порядке)
    for (size_t j = 2; j < n - 1; ++j) {
        //если lcp >= 0 разных строк!
        cur_min_lcp = cur_min_lcp > suf_lcp[j] ? suf_lcp[j] : cur_min_lcp;
        if (suf_lcp[j] != 0 && from_dif_strings(j)) {
            cur_k += suf_lcp[j];
            //вычитаем минимальный lcp на трезке от последнего подошедшего до текщуего
            cur_k -= cur_min_lcp;
            //обновляем
            cur_min_lcp = suf_lcp[j];
        }
        //если перескочили
        if (cur_k >= k) {
            for (size_t i = 0; i < suf_lcp[j] - (cur_k - k); ++i) {
                res += input_text[suf_array[j] + i];
            }
            break;
        }
    }
    if(res.size() == 0)
        return std::nullopt;
    return res;
}

int main() {
    size_t k = 0;
    std::string text1, text2;
    std::cin >> text1 >> text2;
    std::cin >> k;

    std::string text = text1 + separator1 + text2 + separator2;
    SuffixArray s(text);
    s.LCP();

    std::cout << s.k_substring(k).value_or("-1") << std::endl;
    return 0;
}

