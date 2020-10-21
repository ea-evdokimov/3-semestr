#include <vector>
#include <iostream>
#include <string>
#include <map>

struct SuffixArray {
    std::string input_text;
    std::vector<int> suf_array;
    std::vector<size_t> suf_lcp;
    //конец первой строки
    int end_first = -1;

    //непосредственно построение суф массива
    void build(std::string &text);

    //нулевой шаг - сортировка подсчетом первых символов
    size_t zero_step(std::map<int, std::vector<int>> &indexes, std::vector<int> &pos, std::vector<int> &classes);

    //наибольший общий префикс
    std::vector<size_t> LCP();

    //нахождение к общей подстроки
    std::string k_substring(size_t k);

    //из разных ли строк данный префикс суффикса
    bool from_dif_strings(int j);
};

size_t
SuffixArray::zero_step(std::map<int, std::vector<int>> &indexes, std::vector<int> &poses, std::vector<int> &classes) {
    size_t dif_classes = 0, pos_i = 0;

    for (auto pair: indexes) {
        //pair.first -- символ
        for (auto ind : pair.second) {
            poses[pos_i] = ind;
            ++pos_i;
            classes[ind] = dif_classes;
        }
        //заполнили один класс
        ++dif_classes;
    }

    return dif_classes;
}

void SuffixArray::build(std::string &text) {
    //количество различных классов на данной итерации
    size_t dif_classes = 0;
    input_text = text;
    size_t n = text.size();
    std::vector<int> classes(text.size());
    std::vector<int> poses(text.size());
    //соответсвие символа и позициям, на которых этот символ встречается
    std::map<int, std::vector<int>> indexes;
    for (int i = 0; i < n; ++i) {
        indexes[text[i]].push_back(i);
    }
    //0 - этап, сортируем по первым 2^0=1 символов
    dif_classes = zero_step(indexes, poses, classes);

    //вспомогательные векторы для вторых частей
    std::vector<int> poses_second(n), classes_second(n), count(n);

    for (size_t k = 0; (1 << k) < n; ++k) {
        //если все суф в разных классах
        if (dif_classes == n)
            break;

        std::fill(count.begin(), count.end(), 0);
        //poses_second - сортировка по вторым частям
        for (size_t i = 0; i < n; i++)
            poses_second[i] = (poses[i] + n - (1 << k)) % n;

        //сортировка
        for (size_t i = 0; i < n; i++) {
            ++count[classes[poses_second[i]]];
        }

        for (size_t i = 1; i < dif_classes; ++i)
            count[i] += count[i - 1];

        for (int i = n - 1; i >= 0; --i) {
            --count[classes[poses_second[i]]];
            int cnt = count[classes[poses_second[i]]];
            poses[cnt] = poses_second[i];
        }

        //имеем новые позиции, проставим классы
        classes_second[poses[0]] = 0;
        dif_classes = 1;

        for (size_t i = 1; i < n; ++i) {
            int right_2 = (poses[i] + (1 << k)) % n, right_1 = (poses[i - 1] + (1 << k)) % n;
            int left_1 = poses[i - 1], left_2 = poses[i];
            //за O(1) понимаем, в разных классах суффиксы лежат или в одном
            if (classes[left_1] != classes[left_2] || classes[right_1] != classes[right_2])
                ++dif_classes;

            classes_second[poses[i]] = dif_classes - 1;
        }

        classes.swap(classes_second);
    }

    //теперь в векторе внутри класса хранится наш суф массив
    suf_array = std::move(poses);
}

//алгоритм Касаи
std::vector<size_t> SuffixArray::LCP() {
    size_t n = input_text.size(), cur = 0;
    std::vector<size_t> lcp(n - 1);
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

    suf_lcp.swap(lcp);
    return lcp;
}

bool SuffixArray::from_dif_strings(int j) {
    //первый раз посчитаем, где стоит разделитель
    if (end_first == -1) {
        size_t n = input_text.size();
        for (size_t i = 0; i < n; ++i)
            if (input_text[i] == '_')
                end_first = i;
    }

    //нам подходит, если в одном знак '_' встречается, а в другом - нет
    return (suf_array[j] > end_first) & (suf_array[j + 1] < end_first) ||
           (suf_array[j] < end_first) & (suf_array[j + 1] > end_first);
}

std::string SuffixArray::k_substring(size_t k) {
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

    return res;
}

int main() {
    size_t k = 0;
    std::string text1, text2;
    std::cin >> text1 >> text2;
    std::cin >> k;

    std::string text = text1 + '_' + text2 + '^';
    SuffixArray s;
    s.build(text);
    s.LCP();

    std::string res = s.k_substring(k);

    if (res.length() == 0)
        std::cout << "-1" << " " << std::endl;
    else
        std::cout << res << " " << std::endl;
}
/*
abab
cab
1
abab
cab
2
abab
cab
3
abab
cab
4
aaa
abaa
1
aaa
abaa
2
aaa
abaa
3
abcd
abcd
5
ddbbc
aabbc
4
*/
