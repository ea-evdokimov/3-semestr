#include <iostream>
#include <string>
#include <vector>

class Solution{
private:
    static std::string preproc(const std::string &text){
        //предобработка для того, чтобы не писать для четных и нечетных палиндромов
        std::string new_text;
        new_text.push_back('|');

        for(char symbol : text){
            new_text.push_back(symbol);
            new_text.push_back('|');
        }
        return new_text;
    }
public:
    static void manaker(const std::string &old_text){
        //предобработка (abc -> |a|b|c|)
        std::string text = preproc(old_text);
        //храним и заполняем массивы с первого индекса
        //храним начало и конец найденного на текщий момент палиндрома с самой правой границей
        int32_t left = 0, right = 0;
        uint32_t size = text.size();
        uint64_t result = 0;

        //изначально все 1
        std::vector<int32_t> max_rad(size, 1);

        for(uint32_t i = 1; i < size; ++i){
            int32_t cur_l, cur_r;

            if(i <= right) {
                //воспользуемся тем, что уже знаем
                int32_t reversed = right + left - i;
                cur_l = i + 1 - max_rad[reversed], cur_r = i - 1 + max_rad[reversed];

                //если кусочек вылазит(нет информации)
                if(cur_r > right){
                    int32_t cut = cur_r - right;
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
};

int main(){
    std::string text; std::cin >> text;
    Solution s;
    s.manaker(text);
}
