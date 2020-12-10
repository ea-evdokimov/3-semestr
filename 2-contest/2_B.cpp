#include <iostream>
#include <vector>
#include <algorithm>
#include <stack>
#include <cmath>
#include <set>

template<typename T>
struct Point {
    T x, y;

    T len(const Point &a) {
        return (a.x - x) * (a.x - x) + (a.y - y) * (a.y - y);
    }
};

template<typename T>
class Hull2D {
private:
    Point<T> p0;
    std::vector<Point<T>> points, convex_hull;

    T angle(const Point<T> &p, const Point<T> &p1, const Point<T> &p2) {
        return (p1.y - p.y) * (p2.x - p1.x) - (p1.x - p.x) * (p2.y - p1.y);
    }

public:
    Hull2D(const std::vector<Point<T>> &v);

    void print_convex_hull() {
        for (auto i : convex_hull)
            std::cout << "(" << i.x << ";" << i.y << ")" << " ";
    }

    double sum_len() {
        double res = 0;
        for (size_t i = 0; i < convex_hull.size() - 1; ++i) {
            T len = convex_hull[i].len(convex_hull[i + 1]);
            res += sqrt(len);
        }
        return res;
    }
};

template<typename T>
Hull2D<T>::Hull2D(const std::vector<Point<T>> &v) : points(std::move(v)) {
    //сортируем по y, берем самую нижнюю правую
    std::sort(points.begin(), points.end(), [](const Point<T> &a, const Point<T> &b) {
        return (a.y < b.y || (a.y == b.y && a.x > b.x));
    });
    p0 = points[0];

    std::sort(points.begin() + 1, points.end(), [&](const Point<T> &q, const Point<T> &r) {
        //если на одной прямой, берем ту точку, которая дальше
        if (angle(p0, q, r) == 0)
            return p0.len(q) <= p0.len(r);
        return angle(p0, q, r) < 0;
    });

    std::stack<Point<T>> s;
    //добавляем последнюю и 0 точки
    s.push(points[points.size() - 1]);
    s.push(points[0]);

    size_t i = 1;

    if (points.size() > 3) {
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
    } else {
        Point<T> p_t0 = s.top();
        s.pop();
        Point<T> p_t1 = s.top();
        s.pop();
        s.push(points[points.size() - 2]);
        s.push(p_t1);
        s.push(p_t0);
        s.push(points[points.size() - 2]);
    }

    while (!s.empty()) {
        convex_hull.push_back(s.top());
        s.pop();
    }
}

int main() {
    std::vector<Point<int64_t>> v;
    size_t N;
    int64_t ax, ay;
    std::cin >> N;
    std::set<std::pair<int64_t, int64_t>> s;

    for (size_t i = 0; i < N; ++i) {
        std::cin >> ax >> ay;
        s.insert({ax, ay});
    }

    for (auto i : s)
        v.push_back(Point<int64_t>{i.first, i.second});

    Hull2D<int64_t> p(v);

    printf("%.10lf", p.sum_len());
    return 0;
}