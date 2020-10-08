#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

struct Solution{
private:
    std::vector<int> zeta;

public:
    void zeta_function(const std::string& text, const std::string& pattern){
        //0-ой элемент не будем подсчитывать
        //r = max (l + z[l])
        //[l;r] - оптимальный z - блок
        int i = 1, l = 0, r = 0, t_size = text.size(), p_size = pattern.size();
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

        //теперь считаем на тексте
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
};

int main() {
    std::string pattern, text;
    std::cin >> pattern >> text;
    Solution s;
    s.zeta_function(text, pattern);

    return 0;
}