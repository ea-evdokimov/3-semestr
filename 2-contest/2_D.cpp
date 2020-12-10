#include <iostream>
#include <vector>
#include <algorithm>

template<typename T>
struct Point {
    T x, y;
};

template<typename T>
struct Vector {
    Point<T> a, b;

    //конструкторы
    Vector(T x1, T y1, T x2, T y2) : a(x1, y1), b(x2, y2) {}

    Vector(const Point<T> &a_, const Point<T> &b_) : a(a_), b(b_) {}

    T cross_product(const Vector &t) {
        //векторное произведение
        T cur_x = b.x - a.x, cur_y = b.y - a.y;
        T t_x = t.b.x - t.a.x, t_y = t.b.y - t.a.y;
        return cur_x * t_y - cur_y * t_x;
    }

    T len() {
        T cur_x = b.x - a.x, cur_y = b.y - a.y;
        return cur_x * cur_x + cur_y * cur_y;
    }

    T dot_product(const Vector &t) {
        T cur_x = b.x - a.x, cur_y = b.y - a.y;
        T t_x = t.b.x - t.a.x, t_y = t.b.y - t.a.y;
        //скалярное произведение
        return cur_x * t_x + cur_y * t_y;
    }

    int quad() const {
        //определение четверти, в которой лежит вектор
        int v_x = b.x - a.x, v_y = b.y - a.y;
        if (v_x < 0 && v_y < 0) return 1;
        if (v_x >= 0 && v_y < 0) return 2;
        if (v_x >= 0 && v_y >= 0) return 3;
        if (v_x < 0 && v_y >= 0) return 4;
        return 0;
    }

    bool operator<(const Vector<T> &v) {
        //определеяем четверть, потом угол
        return quad() == v.quad() ? cross_product(v) > 0 : quad() < v.quad();
    }
};

template<typename T>
class Solution {
private:
    std::vector<Point<T>> a_verts, b_verts;
public:
    Solution(std::vector<Point<T>> &a_v, std::vector<Point<T>> &b_v) :
            a_verts(std::move(a_v)), b_verts(std::move(b_v)) {}

    long double area(const std::vector<Point<T>> &verts);

    long double area_sum_of_mink();
};

template<typename T>
long double Solution<T>::area(const std::vector<Point<T>> &verts) {
    long double res = 0;
    //подсчет методом Гаусса
    for (size_t i = 0; i < verts.size(); ++i) {
        res += (verts[i].x * verts[(i + 1) % verts.size()].y - verts[i].y * verts[(i + 1) % verts.size()].x);
    }
    //если отрицательная, меняем знак
    res = res / 2.0;
    return res < 0 ? -res : res;
}

template<typename T>
long double Solution<T>::area_sum_of_mink() {
    std::vector<Vector<T>> edges;

    for (size_t i = 0; i < a_verts.size(); ++i) {
        if (i == a_verts.size() - 1) {
            edges.push_back(Vector<T>(a_verts[i], a_verts[0]));
        } else
            edges.push_back(Vector<T>(a_verts[i], a_verts[i + 1]));
    }

    for (size_t i = 0; i < b_verts.size(); ++i) {
        if (i == b_verts.size() - 1) {
            edges.push_back(Vector<T>(b_verts[i], b_verts[0]));
        } else
            edges.push_back(Vector<T>(b_verts[i], b_verts[i + 1]));
    }

    std::sort(edges.begin(), edges.end());

    Point<T> cur_v = edges[edges.size() - 1].a;
    std::vector<Point<T>> res;
    res.push_back(cur_v);

    for (auto edge : edges) {
        cur_v.x += (edge.b.x - edge.a.x);
        cur_v.y += (edge.b.y - edge.a.y);
        res.push_back(cur_v);
    }

    long double coefficient = (area(res) - area(a_verts) - area(b_verts)) / 2;
    return coefficient;
}

int main() {
    int64_t x, y;
    size_t N;
    std::cin >> N;
    std::vector<Point<int64_t>> a_v, b_v;
    for (size_t i = 0; i < N; ++i) {
        std::cin >> x >> y;
        a_v.push_back(Point<int64_t>{x, y});
    }

    std::cin >> N;
    for (size_t i = 0; i < N; ++i) {
        std::cin >> x >> y;
        b_v.push_back(Point<int64_t>{x, y});
    }

    Solution<int64_t> s(a_v, b_v);
    printf("%.6Lf", s.area_sum_of_mink());

    return 0;
}