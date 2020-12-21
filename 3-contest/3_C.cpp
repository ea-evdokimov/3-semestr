#include <vector>
#include <iostream>

size_t mex(const std::vector<int> &cur){
    std::vector<int> exist(cur.size() + 1, 0);
    for(auto i : cur){
        if (i <= cur.size())
            exist[i] = 1;
    }
    for(size_t j = 0; j < exist.size(); ++j){
        if(!exist[j])
            return j;
    }
}

void algo(size_t n){
    std::vector<int> SG(n + 1, 0), cur;

    for(size_t i = 2; i <= n; ++i){
        cur.push_back(SG[i-1]);
        for(size_t j = 1; j <= i - 2; ++j)
            cur.push_back(SG[j] ^ SG[i - j - 1]);
        SG[i] = mex(cur);
        cur.clear();
    }

    if(SG[n] != 0){
        std::cout << "Schtirlitz" << "\n";
        for(size_t t = 1; t <= n; ++t){
            if((SG[t - 1] ^ SG[n - t]) == 0)
                std::cout << t << "\n";
        }
    }
    else
        std::cout << "Mueller" << "\n";
}

int main(){
    size_t n; std::cin >> n;
    algo(n);
    return 0;
}
