#include <vector>
#include <iostream>
#include <string>
#include <map>

struct SuffixArray {
    std::string input_text;
    std::vector<int> suf_array;
    std::vector<int> suf_lcp;

    //непосредственно построение суф массива
    void build(std::string &text);

    //нулевой шаг - сортировка подсчетом первых символов
    size_t zero_step(std::map<int, std::vector<int>> &indexes, std::vector<int> &pos, std::vector<int> &classes);

    //наибольший общий префикс
    std::vector<int> LCP();

    //нахождение количества различных подстрок
    size_t dif_substrings();
};

size_t SuffixArray::zero_step(std::map<int, std::vector<int>> &indexes, std::vector<int> &poses, std::vector<int> &classes) {
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
    text += '$';
    input_text = text;
    size_t n = text.size();

    std::vector<int> classes(n), poses(n);
    //соответсвие символа и позициям, на которых этот символ встречается
    std::map<int, std::vector<int>> indexes;
    for (size_t i = 0; i < n; ++i) {
        indexes[text[i]].push_back(i);
    }
    //0 - этап, сортируем по первым 2^0=1 символов
    dif_classes = zero_step(indexes, poses, classes);

    //вспомогательные векторы для вторых частей
    int count_size = n > 26 ? n : 26;
    std::vector<int> poses_second(n), classes_second(n), count(count_size);

    for (size_t k = 0; (1 << k) < text.size(); ++k) {
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
            int right_2 = (poses[i] + (1 << k)) % n, right_1 = (poses[i - 1] + (1 << k)) % n,
            left_1 = poses[i - 1], left_2 = poses[i];

            //за O(1) понимаем, в разных классах суффиксы лежат или в одном
            if (classes[left_1] != classes[left_2] || classes[right_1] != classes[right_2])
                ++dif_classes;

            classes_second[poses[i]] = dif_classes - 1;
        }

        classes.swap(classes_second);
    }

    //теперь в векторе внутри класса хранится наш суф массив
    suf_array =std::move(poses);
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
    //воспользуемсчя посчитанными lcp и suf_array
    size_t n = input_text.size();
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
    SuffixArray s;
    s.build(text);
    s.LCP();
    std::cout << s.dif_substrings();
}

