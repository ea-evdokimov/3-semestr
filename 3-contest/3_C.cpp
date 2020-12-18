#include <vector>
#include <algorithm>
#include <iostream>

int mex(std::vector<int> &curs){
    int res = 0;
    std::sort(curs.begin(), curs.end());
    
    for(int i = 0; i < curs.size(); ++i){
        if(res == curs[i]){
            while(i < curs.size() && curs[i] == res)
                ++i;
            ++res, --i;
            continue;
        }
        else
            return res;
    }
    return res;
}

void algo(int n){
    std::vector<int> SG(n + 1, 0), curs;

    for(int i = 2; i <= n; ++i){
        curs.push_back(SG[i-1]);
        for(int j = 1; j <= i - 2; ++j)
            curs.push_back(SG[j] ^ SG[i - j - 1]);
        SG[i] = mex(curs);
        curs.clear();
    }
    
    if(SG[n] != 0){
        std::cout << "Schtirlitz" << "\n";
        for(int t = 1; t <= n; ++t){
            if((SG[t - 1] ^ SG[n - t]) == 0)
                std::cout << t << "\n";
        }
    }
    else
        std::cout << "Mueller" << "\n";
}

int main(){
    int n; std::cin >> n;
    algo(n);
    return 0;
}
