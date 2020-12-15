#include <iostream>
#include <vector>

template<typename T>
struct Point{
    T x, y;
    friend std::istream& operator>> (std::istream &in, Point<T> &p)
    {
        in >> p.x;
        in >> p.y;
        return in;
    }
};

template<typename T>
struct Vector{
    T x, y;

    Vector(T x1, T y1, T x2, T y2){
        Vector(Point<T>{x1, y1}, Point<T>{x2, y2});
    }
    Vector(const Point<T> &a, const Point<T> &b) : x(b.x - a.x), y(b.y - a.y) {}

    T cross_product(const Vector &b){
        //векторное произведение
        return x * b.y - y * b.x;
    }

    T len(){
        return x * x + y * y;
    }

    T dot_product(const Vector &b){
        return x * b.x  + y * b.y;
    }
};

template<typename T>
struct Cut{
    Point<T> a, b;
};

template<typename T>
class Cuts{
private:
    Cut<T> trace;
    std::vector<Cut<T>> rivers;
public:
    Cuts(const Cut<T> &first, const std::vector<Cut<T>> &others) : trace(first), rivers(std::move(others)) {}

    int intersects();
};

template<typename T>
int Cuts<T>::intersects(){
    Vector<T> v12{trace.a, trace.b};
    int res = 0;

    for(auto cur_cut : rivers){
        Vector<T> v34{cur_cut.a, cur_cut.b}, v31{cur_cut.a, trace.a}, v32{cur_cut.a, trace.b},
                v13{trace.a, cur_cut.a}, v14{trace.a, cur_cut.b};

        //отдельно случай, где река вдоль реки
        if(v12.cross_product(v34) == 0){
            continue;
        }

        if(v34.cross_product(v31) * v34.cross_product(v32) < 0 && v12.cross_product(v13) * v12.cross_product(v14) < 0) {
            ++res;
            continue;
        }

        if(v34.cross_product(v31) == 0) {
            //проверка на то, что точка 1 принадлежит отрезку 34
            //достаточно чтобы (v31, v34) и (v14, v34) были >= 0
            if(v31.dot_product(v34) >= 0 && v34.dot_product(v14) >= 0){
                ++res;
                continue;
            }
        }

        if(v34.cross_product(v32) == 0) {
            //проверка на то, что точка 2 принадлежит отрезку 34
            Vector<T> v24{trace.b, cur_cut.b};
            if(v32.dot_product(v34) >= 0 && v34.dot_product(v24) >= 0){
                ++res;
                continue;
            }
        }

        if(v12.cross_product(v13) == 0) {
            //проверка на то, что точка 3 принадлежит отрезку 12
            if(v31.dot_product(v12) <= 0 && v32.dot_product(v12) >= 0){
                ++res;
                continue;
            }
        }

        if(v12.cross_product(v14) == 0) {
            //проверка на то, что точка 4 принадлежит отрезку 12
            Vector<T> v24{trace.b, cur_cut.b};
            if(v14.dot_product(v12) >= 0 && v24.dot_product(v12) <= 0){
                ++res;
                continue;
            }
        }
    }
    return res;
}


int main() {
    std::vector<Cut<int64_t>> v;
    size_t N;
    Point<int64_t> a, b;
    std::cin >> a >> b;
    std::cin >> N;

    Cut<int64_t> trace{a, b};
    for(size_t i = 0; i < N; ++i){
        std::cin >> a >> b;
        v.push_back(Cut<int64_t>{a, b});
    }

    Cuts<int64_t> C(trace, v);
    std::cout << C.intersects() << '\n';

    return 0;
}
