#include <iostream>
#include <set>
#include <vector>
#include <algorithm>

template<typename T>
struct Point {
    T x, y;
};

template<typename T>
struct Cut {
    Point<T> a, b;
    //номер отрезка
    int id;

    Cut(T x1, T y1, T x2, T y2, int id_ = -1) : a{x1, y1}, b{x2, y2}, id(id_) {}

    Cut(const Point<T> &a_, const Point<T> &b_, int id_ = -1) : a(a_), b(b_), id(id_) {}

    //scan line будет вертикальной -> нужно знать коорд y пересечения
    double cut_y(T cur_sl_x) const {
        if (a.x == b.x) {
            return a.y;
        } else {
            double cut_y, dy = b.y - a.y, dx = b.x - a.x;
            cut_y = dy * (cur_sl_x - a.x) / dx + a.y;
            return cut_y;
        }
    }

    T len() const {
        return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
    }

    T cross_product(const Cut &other) {
        //векторное произведение
        return (b.x - a.x) * (other.b.y - other.a.y) - (b.y - a.y) * (other.b.x - other.a.x);
    }

    T dot_product(const Cut &other) {
        //скалярное произведение
        return (b.x - a.x) * (other.b.x - other.a.x) + (b.y - a.y) * (other.b.y - other.a.y);
    }

    bool operator<(const Cut &other) const {
        //сравнение зависит от положения прямой
        T cur_x = std::max(a.x, other.a.x);
        double c = cut_y(cur_x), other_c = other.cut_y(cur_x);

        return c < other_c;
    }
};

//проверка на то, пересекаются ли два конкретных отрзка
template<typename T>
bool intersect(const Cut<T> &cut_1, const Cut<T> &cut_2) {
    Cut<T> v12(cut_2.a, cut_2.b), v34(cut_1.a, cut_1.b), v31(cut_1.a, cut_2.a), v32(cut_1.a, cut_2.b),
            v13(cut_2.a, cut_1.a), v14(cut_2.a, cut_1.b);

    if (v34.cross_product(v31) * v34.cross_product(v32) < 0 && v12.cross_product(v13) * v12.cross_product(v14) < 0) {
        return true;
    }

    if (v34.cross_product(v31) == 0) {
        //проверка на то, что точка 1 принадлежит отрезку 34
        //достаточно чтобы (v31, v34) и (v14, v34) были >= 0
        if (v31.dot_product(v34) >= 0 && v34.dot_product(v14) >= 0) {
            return true;
        }
    }

    if (v34.cross_product(v32) == 0) {
        //проверка на то, что точка 2 принадлежит отрезку 34
        Cut<T> v24{cut_2.b, cut_1.b};
        if (v32.dot_product(v34) >= 0 && v34.dot_product(v24) >= 0) {
            return true;
        }
    }

    if (v12.cross_product(v13) == 0) {
        //проверка на то, что точка 3 принадлежит отрезку 12
        if (v31.dot_product(v12) <= 0 && v32.dot_product(v12) >= 0) {
            return true;
        }
    }

    if (v12.cross_product(v14) == 0) {
        //проверка на то, что точка 4 принадлежит отрезку 12
        Cut<T> v24{cut_2.b, cut_1.b};
        if (v14.dot_product(v12) >= 0 && v24.dot_product(v12) <= 0) {
            return true;
        }
    }
    return false;
}

template<typename T>
struct Event {
    //коорд скан прямой
    T x;
    int type, id;

    Event(T x_, int type_, int id_ = -1) : x(x_), type(type_), id(id_) {}

    bool operator<(const Event &e) const {
        //если по x совпадают, сначала добавление, а потом удаление
        return x != e.x ? x < e.x : type < e.type;
    }
};

enum {
    CUT_START = 0,
    CUT_END = 1
};

template<typename T>
class ScanLine {
private:
    std::vector<Cut<T>> cuts;
    std::vector<Event<T>> event_queue;
    std::set<Cut<T>> cur_cuts;

public:
    ScanLine(const std::vector<Cut<T>> &v) : cuts(std::move(v)) {}

    std::pair<int64_t, int64_t> find_intersects();
};

template<typename T>
std::pair<int64_t, int64_t> ScanLine<T>::find_intersects() {
    std::pair<int64_t, int64_t> result = {-1, -1};
    //заполняем массив все событий
    for (size_t i = 0; i < cuts.size(); ++i) {
        //добавление отрезка
        event_queue.push_back(Event<T>(cuts[i].a.x, CUT_START, i));
        //удаление отрезка
        event_queue.push_back(Event<T>(cuts[i].b.x, CUT_END, i));
    }

    std::sort(event_queue.begin(), event_queue.end());

    //обработка событий
    for (size_t i = 0; i < event_queue.size(); ++i) {
        int cur_id = event_queue[i].id;

        typename std::set<Cut<T>>::iterator above = cur_cuts.lower_bound(cuts[cur_id]),
                below = cur_cuts.upper_bound(cuts[cur_id]);

        if (event_queue[i].type == CUT_START) {
            //рассматриваем предыдущий и следующий относительно рассмотриваемого отрезки
            if (above != cur_cuts.end() && intersect(cuts[cur_id], *above)) {
                result = {above->id, cur_id};
                break;
            } else if (below != cur_cuts.end() && intersect(cuts[cur_id], *below)) {
                result = {below->id, cur_id};
                break;
            }
            cur_cuts.insert(cuts[cur_id]);
        } else if (event_queue[i].type == CUT_END) {
            //рассматриваем предыдущий и следующий относительно рассмотриваемого отрезки
            if (above != cur_cuts.end() && below != cur_cuts.end() && above != below && intersect(*below, *above)) {
                result = {above->id, below->id};
                break;
            }

            cur_cuts.erase(cuts[cur_id]);
        }
    }
    return result;
}

int main() {
    size_t N;
    std::cin >> N;
    int64_t x1, y1, x2, y2;
    std::vector<Cut<int64_t>> v;

    for (size_t i = 0; i < N; ++i) {
        std::cin >> x1 >> y1 >> x2 >> y2;
        if ((x2 < x1) || ((x2 == x1) && (y2 < y1))) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        v.push_back(Cut<int64_t>(x1, y1, x2, y2, i));
    }

    ScanLine<int64_t> s(v);
    auto result = s.find_intersects();
    if (result.first == -1) {
        std::cout << "NO" << '\n';
    } else {
        if (result.first > result.second)
            std::swap(result.first, result.second);

        std::cout << "YES" << '\n';
        std::cout << result.first + 1 << ' ' << result.second + 1 << '\n';
    }

    return 0;
}