#include <string>
#include <iostream>
#include <vector>

class Zeta{
private:
    int l, r;
    std::vector<size_t> zeta;
public:
    void zeta_pattern(const std::string& pattern);
    void KMP(const std::string& text, const std::string& pattern);
};

void Zeta::zeta_pattern(const std::string &pattern) {
    //0-ой элемент не будем подсчитывать
    //r = max (l + z[l])
    //[l;r] - оптимальный z - блок
    l = 0, r = 0;
    int p_size = pattern.size(), i = 0;
    //посчитаем значения только для pattern
    zeta.resize(p_size, 0);

    while(i < p_size){
        //чтобы не вылезти за границы
        if (i <= r)
            zeta[i] = zeta[i - l] <= r - i? zeta[i - l] : r - i;
        //жадно набираем, пока либо не встретим несовпадающий символ
        // либо не наткнемся на конец строки
        while(pattern[i + zeta[i]] == pattern[zeta[i]] && i + zeta[i] < p_size)
            ++zeta[i];
        //обновляем r и l
        if(i + zeta[i] > r) {
            l = i;
            r = i + zeta[i];
        }
        ++i;
    }
}

void Zeta::KMP(const std::string& text, const std::string& pattern){
    zeta_pattern(pattern);

    int t_size = text.size(), p_size = pattern.size();
    //теперь считаем на тексте
    int i = p_size;
    while(i - p_size < t_size){
        int cur_zeta = 0;
        //чтобы не вылезти за границы
        if (i <= r) {
            int index = i - l < p_size ? i - l : i - l - 1;

            if(zeta[index] <= r - i)
                cur_zeta = zeta[index];
            else
                cur_zeta = r - i;
        }
        //жадно набираем, пока либо не встретим несовпадающий символ
        // либо не наткнемся на конец строки
        while(cur_zeta < p_size && i - p_size + cur_zeta < t_size && text[i - p_size + cur_zeta] == pattern[cur_zeta])
            ++cur_zeta;

        //обновляем r и l
        if(i + cur_zeta > r) {
            l = i;
            r = i + cur_zeta;
        }

        if(cur_zeta == p_size)
            std::cout << i - p_size << " ";

        ++i;
    }
}


int main() {
    std::string pattern, text;
    std::cin >> pattern >> text;
    Zeta z;
    z.KMP(text, pattern);

    return 0;
}
