#include <iostream>
#include <vector>
#include <algorithm>
#include <stack>
#include <cmath>
#include <iomanip>
#include <set>

template<typename T>
struct Point{
    T x, y;

    T sq_len(const Point &a){
        return (a.x - x) * (a.x - x) + (a.y - y) * (a.y - y);
    }
    friend std::istream& operator>> (std::istream &in, Point<T> &p){
        in >> p.x;
        in >> p.y;
        return in;
    }

};

template<typename T>
class Hull2D{
private:
    Point<T> p0;
    std::vector<Point<T>> points, convex_hull;

    T angle(const Point<T>& p, const Point<T>& p1, const Point<T>& p2){
        return (p1.y - p.y) * (p2.x - p1.x) - (p1.x - p.x) * (p2.y - p1.y);
    }

public:
    Hull2D(const std::vector<Point<T>> &v) : points(std::move(v)) {}

    void build_convex_hull();

    void print_convex_hull(){
        for(auto i : convex_hull)
            std::cout << "(" << i.x << ";" <<  i.y << ")" << " ";
    }

    double sum_len(){
        double res = 0;
        for(size_t i = 0; i < convex_hull.size() - 1; ++i){
            T len = convex_hull[i].sq_len(convex_hull[i + 1]);
            res += sqrt(len);
        }
        return res;
    }
};


template<typename T>
void Hull2D<T>::build_convex_hull(){
    //сортируем по y, берем самую нижнюю правую
    std::sort(points.begin(), points.end(), [](const Point<T>& a, const Point<T>& b){
        return std::tie(a.y, a.x) < std::tie(b.y, b.x);
    });
    p0 = points[0];

    std::sort(points.begin() + 1, points.end(), [&p0=p0](const Point<T>& q, const Point<T>& r){
        //если на одной прямой, берем ту точку, которая дальше
        T a = (q.y - p0.y) * (r.x - q.x) - (q.x - p0.x) * (r.y - q.y);
        if (a == 0)
            return p0.sq_len(q) <= p0.sq_len(r);
        return a < 0;
    });

    std::stack<Point<T>> s;
    //добавляем последнюю и 0 точки
    s.push(points[points.size() - 1]);
    s.push(points[0]);

    size_t i = 1;

    if(points.size() <= 3){
        for(auto p : points){
            convex_hull.push_back(p);
        }
        convex_hull.push_back(points[0]);
    }
    else{
        while (i < points.size()) {
            Point<T> p_t0, p_t1;
            p_t0 = s.top(), s.pop();
            p_t1 = s.top(), s.pop();
            //берем последние две, проверяем угол
            if (angle(p_t1, p_t0, points[i]) <= 0) {
                //если имеем поворот и точки лежат на прямой, добавляем
                s.push(p_t1);
                s.push(p_t0);
                s.push(points[i]);
                ++i;
            } else {
                s.push(p_t1);
            }
        }
        while(!s.empty()){
            convex_hull.push_back(s.top());
            s.pop();
        }
    }
}

int main() {
    std::vector<Point<int64_t>> v;
    size_t N;
    Point<int64_t> a;
    std::cin >> N;
    auto cmp = [](Point<int64_t> a, Point<int64_t> b) { return std::tie(a.x, a.y) < std::tie(b.x, b.y); };
    std::set<Point<int64_t>, decltype(cmp)> s(cmp);

    for(size_t i = 0; i < N; ++i){
        std::cin >> a;
        s.insert(a);
    }

    for(auto i : s)
        v.push_back(i);

    Hull2D<int64_t> p(v);
    p.build_convex_hull();

    std::cout << std::setprecision(10) << p.sum_len() << '\n';
    return 0;
}
