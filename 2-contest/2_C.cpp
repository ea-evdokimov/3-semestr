#include <iostream>
#include <unordered_map>
#include <list>
#include <set>
#include <vector>
#include <algorithm>

template<typename T>
struct Point {
    T x, y, z;
    int id;

    Point(T x, T y, T z, int id_ = -1) : x(x), y(y), z(z), id(id_) {}

    bool operator==(const Point &q) const {
        return std::tie(x, y, z) == std::tie(q.x, q.y, q.z);
    }
};

template<typename T>
struct Vector {
    T x, y, z;

    Vector(T x1, T y1, T z1, T x2, T y2, T z2) : x(x2 - x1), y(y2 - y1), z(z2 - z1) {}

    Vector(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}

    Vector(const Point<T> &a, const Point<T> &b) : x(b.x - a.x), y(b.y - a.y), z(b.z - a.z) {}

    Vector<T> cross_product(const Vector &b) const {
        //векторное произведение
        return Vector<T>(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
    }

    T sq_len() const {
        return x * x + y * y + z * z;
    }

    T dot_product(const Vector &b) const {
        return x * b.x + y * b.y + z * b.z;
    }
};

template<typename T>
struct Plane {
    //порядок важен
    Point<T> p1, p2, p3;
    bool visible = false;

    Plane(const Point<T> p, const Point<T> q, const Point<T> r) : p1(p), p2(q), p3(r) {}

    //ориентированная внешняя нормаль
    Vector<T> normal() const {
        Vector<T> v12(p1, p2), v23(p2, p3);
        return v12.cross_product(v23);
    }

    bool consist(const Point<T> p) const {
        Vector<T> norm = normal(), v1(p, p1);
        return norm.dot_product(v1) == 0;
    }

    //видимость есть внешняя нормаль и вектор к точке направлены в одну сторону
    bool is_visible(const Point<T> &p) {
        Vector<T> v1(p1, p), norm = normal();
        //равно быть не может, так как любые 4 точки не лежат в одной плоскости
        visible = norm.dot_product(v1) > 0;
        return norm.dot_product(v1) > 0;
    }

    void print() const {
        std::cout << "3 " << p1.id << " " << p2.id << " " << p3.id << "\n";
    }

    void normalize() {
        //чтобы наименьшая точка стояла первой
        if (p1.id == std::min({p1.id, p2.id, p3.id})) {
            return;
        } else if (p2.id == std::min({p1.id, p2.id, p3.id})) {
            Point<T> tmp = p1;
            p1 = p2;
            p2 = p3;
            p3 = tmp;
        } else {
            Point<T> tmp = p1;
            p1 = p3;
            p3 = p2;
            p2 = tmp;
        }
    }

    //совпадает ли направление ребра с обходом грани
    bool direct(const Point<T> &q1, const Point<T> &q2) {
        if ((p1 == q1 && p2 == q2) || (p2 == q1 && p3 == q2) || (p3 == q1 && p1 == q2)) {
            return true;
        }
        return false;
    }
};

template<typename T>
struct Edge {
    //это пересечение двух каких-то граней
    //номера вершин в массиве points_
    int p1, p2;
    //их номера
    Plane<T> *first_ptr, *second_ptr;

    Edge(int p, int q, Plane<T> *ptr1 = nullptr, Plane<T> *ptr2 = nullptr) : p1(p), p2(q), first_ptr(ptr1),
                                                                             second_ptr(ptr2) {}
    bool operator==(const Edge &e){
        return (p1 == e.p1 && p2 == e.p2) || (p1 == e.p2 && p2 == e.p1);
    }
};

//для инкрементирования итератора
template<typename T>
typename std::list<Plane<T>>::iterator incr_iter(int i, typename std::list<Plane<T>>::iterator it) {
    for (int j = 0; j < i; ++j)
        ++it;
    return it;
}

bool comp(const Plane<int32_t> &p, const Plane<int32_t> &q) {
    if (p.p1.id == q.p1.id) {
        return p.p2.id == q.p2.id ? p.p3.id < q.p3.id : p.p2.id < q.p2.id;
    } else
        return p.p1.id < q.p1.id;
}



template<typename T>
class ConvexHull {
private:
    std::vector<Point<T>> points_;
    std::list<Plane<T>> planes_;
    std::list<Edge<T>> edges_;
    std::unordered_map<size_t, Edge<T> *> map_edges_;

    void build();

    void try_add_edges(Plane<T> *plane, int num1, int num2, int num3);

    void link_plane(Edge<T> *edge, Plane<T> *plane);

    size_t hash_edge(Point<T> p1, Point<T> p2);
public:
    ConvexHull(std::vector<Point<T>> &v) : points_(std::move(v)) {
        build();
    }

    void print_hull();
};

template<typename T>
size_t ConvexHull<T>::hash_edge(Point<T> p1, Point<T> p2) {
    //ребро это 4 числа от 0 до 1000
    if (p1.id > p2.id) {
        std::swap(p1, p2);
    }

    size_t res =
            1e15 * (p1.x + 500) + 1e12 * (p1.y + 500) + 1e9 * (p1.z + 500) + 1e6 * (p2.x + 500) + 1e3 * (p2.y + 500) +
            (p2.z + 500);

    return res;
}

template<typename T>
void ConvexHull<T>::build() {
    //проверка первых точек на правильность нормали
    Plane<T> base{points_[0], points_[1], points_[2]};
    if (base.is_visible(points_[3]) == true)
        std::swap(points_[3], points_[2]);

    std::vector<int> tetr_points = {0, 1, 2, 0, 3, 1, 0, 2, 3, 1, 3, 2};
    for (size_t i = 0; i < tetr_points.size(); i += 3) {
        planes_.push_back(Plane<T>{points_[tetr_points[i]], points_[tetr_points[i + 1]], points_[tetr_points[i + 2]]});
    }

    //мучительно и внимательно собираем тетраэдр из первых четырех точек
    std::vector<int> tetr_edges = {0, 1, 0, 1, 1, 2, 0, 3, 1, 3, 3, 1, 0, 3, 1, 2, 0, 2, 2, 0, 2, 3, 2, 3};
    for (size_t i = 0; i < tetr_edges.size(); i += 4) {
        edges_.push_back(Edge<T>(tetr_edges[i], tetr_edges[i + 1],
                                &(*(incr_iter<T>(tetr_edges[i + 2], planes_.begin()))),
                                &(*(incr_iter<T>(tetr_edges[i + 3], planes_.begin())))));
        map_edges_.insert({hash_edge(points_[tetr_edges[i]], points_[tetr_edges[i + 1]]), &edges_.back()});

    }

    size_t n = points_.size();
    for (size_t i = 4; i < n; ++i) {
        //проход по всем граням
        for (auto &plane : planes_) {
            //устанавливаем видимость каждой грани
            plane.is_visible(points_[i]);
        }

        //проход по всем ребрам
        auto it = edges_.begin();
        while (it != edges_.end()) {
            auto &cur_e = *it;
            //скипаем недоработанные ребра(которые были добавлены на эом шаге)
            if (cur_e.first_ptr == nullptr || cur_e.second_ptr == nullptr) {
                ++it;
                continue;
            }
            if (cur_e.first_ptr->visible == true && cur_e.second_ptr->visible == false) {
                int num1, num2, num3;
                //направление текщушего ребра в новой должно совпадать с направлением ребра в видимой грани
                if (cur_e.first_ptr->direct(points_[cur_e.p1], points_[cur_e.p2])) {
                    planes_.push_back(Plane<T>{points_[cur_e.p1], points_[cur_e.p2], points_[i]});
                    num1 = cur_e.p1;
                    num2 = cur_e.p2;
                    num3 = i;
                } else {
                    planes_.push_back(Plane<T>{points_[cur_e.p2], points_[cur_e.p1], points_[i]});
                    num1 = cur_e.p2;
                    num2 = cur_e.p1;
                    num3 = i;
                }
                //добавили грань, изменяем текущее ребро
                cur_e.first_ptr = &(planes_.back());
                //пытаемся добавить новые потенциальные ребра
                try_add_edges(&planes_.back(), num1, num2, num3);

                ++it;
            } else if (cur_e.first_ptr->visible == false && cur_e.second_ptr->visible == true) {
                //аналогично
                int num1, num2, num3;
                //направление текщушего ребра в новой должно совпадать с направлением ребра в видимой грани
                if (cur_e.second_ptr->direct(points_[cur_e.p1], points_[cur_e.p2])) {
                    planes_.push_back(Plane<T>{points_[cur_e.p1], points_[cur_e.p2], points_[i]});
                    num1 = cur_e.p1;
                    num2 = cur_e.p2;
                    num3 = i;
                } else {
                    planes_.push_back(Plane<T>{points_[cur_e.p2], points_[cur_e.p1], points_[i]});
                    num1 = cur_e.p2;
                    num2 = cur_e.p1;
                    num3 = i;
                }
                //добавили грань, изменяем текущее ребро
                cur_e.second_ptr = &(planes_.back());
                //пытаемся добавить новые потенциальные ребра
                try_add_edges(&planes_.back(), num1, num2, num3);

                ++it;
            } else if (cur_e.first_ptr->visible == true && cur_e.second_ptr->visible == true) {
                //иначе это две видимые грани и это ребро не нужно
                it = edges_.erase(it);
                map_edges_.erase(hash_edge(points_[cur_e.p1], points_[cur_e.p2]));
                continue;
            } else
                ++it;
            //случай, когда две невидимые грани - ничего не делаем
        }

        //за линию удаляем все видимые грани
        auto iti = planes_.begin();
        while (iti != planes_.end()) {
            //если грань была видна, удаляем ее за O(1)
            if ((*iti).visible == true)
                iti = planes_.erase(iti);
            else
                ++iti;
        }
    }
}

template<typename T>
void ConvexHull<T>::try_add_edges(Plane<T> *plane, int num1, int num2, int num3) {
    //проверка на то, добавлены ли эти ребра
    size_t hash1 = hash_edge(plane->p1, plane->p2),
            hash2 = hash_edge(plane->p2, plane->p3),
            hash3 = hash_edge(plane->p3, plane->p1);

    if (map_edges_.find(hash1) != map_edges_.end()) {
        link_plane(map_edges_[hash1], plane);
    } else {
        edges_.push_back(Edge<T>(num1, num2, plane));
        map_edges_.insert({hash1, &edges_.back()});
    }

    if (map_edges_.find(hash2) != map_edges_.end()) {
        link_plane(map_edges_[hash2], plane);
    } else {
        edges_.push_back(Edge<T>(num2, num3, plane));
        map_edges_.insert({hash2, &edges_.back()});
    }

    if (map_edges_.find(hash3) != map_edges_.end()) {
        link_plane(map_edges_[hash3], plane);
    } else {
        edges_.push_back(Edge<T>(num3, num1, plane));
        map_edges_.insert({hash3, &edges_.back()});
    }
}

template<typename T>
void ConvexHull<T>::link_plane(Edge<T> *edge, Plane<T> *plane) {
    //связываем ребра с плосокстью
    if (edge->first_ptr == nullptr && edge->second_ptr == nullptr) {
        std::cout << "ERROR" << "\n";
        return;
    } else if (edge->first_ptr == nullptr) {
        edge->first_ptr = plane;
    } else if (edge->second_ptr == nullptr) {
        edge->second_ptr = plane;
    } else {
        return;
    }
}

template<typename T>
void ConvexHull<T>::print_hull() {
    int num = 0;

    for (auto &plane : planes_) {
        plane.normalize();
        ++num;
    }
    planes_.sort(comp);

    std::cout << num << "\n";
    for (auto plane : planes_) {
        plane.print();
    }
}

int main() {
    size_t N, M;
    std::cin >> M;
    int32_t x, y, z;

    for (size_t i = 0; i < M; ++i) {
        std::cin >> N;
        std::vector<Point<int32_t>> v;
        for (size_t j = 0; j < N; ++j) {
            std::cin >> x >> y >> z;
            v.emplace_back(x, y, z, j);
        }

        ConvexHull<int32_t> ch(v);
        ch.print_hull();
    }

    return 0;
}

