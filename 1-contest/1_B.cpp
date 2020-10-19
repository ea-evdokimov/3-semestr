#include <iostream>
#include <string>
#include <vector>

typedef int32_t i32;

struct Solution{
public:
    void manaker(std::string &text){
        //предобработка (abc -> |a|b|c|)
        preproc(text);
        //храним и заполняем массивы с первого индекса
        //храним начало и конец найденного на текщий момент палиндрома с самой правой границей
        i32 left = 0, right = 0, size;
        uint64_t result = 0;
        size = text.size();

        //изначально все 1
        std::vector<i32> max_rad(size, 1);

        for(i32 i = 1; i < size; ++i){
            i32 cur_l, cur_r;

            if(i <= right) {
                //воспользуемся тем, что уже знаем
                i32 reversed = right + left - i;
                cur_l = i + 1 - max_rad[reversed], cur_r = i - 1 + max_rad[reversed];

                //если кусочек вылазит(нет информации)
                if(cur_r > right){
                    i32 cut = cur_r - right;
                    cur_r -= cut;
                    cur_l += cut;
                }
            }
            else {
                //наивное расширение с серединкой в i пока слева/справа совпадают
                cur_l = i, cur_r = i;
            }
            //жадное расширение
            while(cur_l - 1 >= 0 && cur_r < size && text[cur_l - 1] == text[cur_r + 1]){
                --cur_l;
                ++cur_r;
            }
            //новое макс радиус палиндрома в этой точке
            max_rad[i] = cur_r - i + 1;

            //обновляем, если улучшили
            if(cur_r > right) {
                left = cur_l;
                right = cur_r;
            }
        }

        for(auto i : max_rad)
            result += (i / 2);

        //вычитаем все палиндромы из 1 буквы
        result -= size / 2;

        std::cout << result;
    }

    void preproc(std::string &text){
        //предобработка для того, чтобы не писать для четных и нечетных палиндромов
        std::vector<char> tmp;
        tmp.push_back('|');

        for(char symbol : text){
            tmp.push_back(symbol);
            tmp.push_back('|');
        }

        text = std::string(tmp.begin(), tmp.end());
    }
};

int main(){
    std::string text; std::cin >> text;
    Solution s;
    s.manaker(text);
}