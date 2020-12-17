#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include <cmath>
#include <set>
#include <iomanip>

enum event_type{
    w_left_u = 0,
    w_right_v = 1,
    u_minus_uv_cw = 2,
    uu_plus_v_ccw = 3,
    uvv_pus_cw = 4,
    uv_minus_v_ccw = 5
};

struct Point {
    long double x, y, z;
    static constexpr double INF = 1e50;
    static constexpr double EPSLN = 1e-6;
    int64_t id;
    Point *prev, *next;

    Point() : x(INF), y(INF), z(INF), id(-1), prev(nullptr), next(nullptr) {}

    Point(long double x_, long double y_, long double z_, int id_ = -1, Point *p = nullptr, Point *n = nullptr) :
            x(x_), y(y_), z(z_), id(id_), prev(p), next(n) {}

    bool operator==(const Point &q) const {
        return (std::abs(x - q.x) < EPSLN) && (std::abs(y - q.y) < EPSLN) && (std::abs(z - q.z) < EPSLN);
    }

    friend std::istream& operator>> (std::istream &in, Point &p){
        in >> p.x;
        in >> p.y;
        in >> p.z;
        return in;
    }

    void fix_links() {
        //insert this
        if (prev->next != this) {
            prev->next = this;
            next->prev = this;
        }
            //delete this
        else {
            prev->next = next;
            next->prev = prev;
        }
    }
};

bool compare(const Point *p, const Point *q) {
    return p->x < q->x;
}

struct Plane {
    int64_t p1, p2, p3;
};

class ConvexHull {
private:
    std::vector<Point *> all_points;
    std::vector<Plane> planes;
    static constexpr double INF = 1e50;
    Point PINF = Point(1e9, 1e9, 1e9 * 1e9, -1);
public:
    ConvexHull(const std::vector<Point *> &v) : all_points(v) {
        //infinity point
        all_points.push_back(&PINF);

        //sort by x
        std::sort(all_points.begin(), all_points.end(), compare);

        std::vector<Point *> points_hull_lower = build_hull(0, all_points.size());
        add_planes(points_hull_lower);

        for (Point *p : all_points) {
            p->prev = nullptr;
            p->next = nullptr;
            p->z *= -1;
        }
        //similarly
        std::vector<Point *> points_hull_upper = build_hull(0, all_points.size());
        add_planes(points_hull_upper);
    }

    ~ConvexHull() {
        for (int i = all_points.size() - 1; i >= 0; --i) {
            if(all_points[i]->id != -1)
                delete (all_points[i]);
        }
    }

    std::vector<Point *> build_hull(int64_t left, int64_t right);

    std::vector<Point *>
    hull(const std::vector<Point *> &left, const std::vector<Point *> &right, Point *from_left, Point *from_right);

    friend std::ostream& operator<< (std::ostream &out, ConvexHull &h){
        for (auto p : h.planes)
            std::cout << p.p1 << " " << p.p2 << " " << p.p3 << "\n";
        std::cout << "\n";
        return out;
    }

    void add_planes(const std::vector<Point *> &point_seq) {
        for (Point *p : point_seq) {
            Plane cur = Plane{p->prev->id, p->id, p->next->id};
            p->fix_links();
            planes.push_back(cur);
        }
    }

    //if turn right or left
    static long double angle(const Point *p, const Point *p1, const Point *p2) {
        if (p == nullptr || p1 == nullptr || p2 == nullptr)
            return 1.0;
-
        return (p2->y - p->y) * (p1->x - p->x) - (p2->x - p->x) * (p1->y - p->y);
    }

    //time of event when angle changes
    static long double event(const Point *p, const Point *p1, const Point *p2) {
        if (p == nullptr || p1 == nullptr || p2 == nullptr)
            return INF;
        long double a = angle(p, p1, p2);
        if (a == 0) {
            return INF;
        }
        return ((p2->z - p->z) * (p1->x - p->x) - (p2->x - p->x) * (p1->z - p->z)) / a;
    }

    long double av_edges_voronoi() const;
};

std::vector<Point *> ConvexHull::build_hull(int64_t left, int64_t right) {
    std::vector<Point *> res;
    if (right - left == 1) {
        //base case
        res = {all_points[left]};
    } else {
        int64_t middle = (left + right) / 2;
        std::vector<Point *> left_part = build_hull(left, middle), right_part = build_hull(middle, right);
        res = hull(left_part, right_part, all_points[middle - 1], all_points[middle]);
    }

    return res;
}

std::vector<Point *>
ConvexHull::hull(const std::vector<Point *> &left, const std::vector<Point *> &right, Point *from_left,
                 Point *from_right) {
    std::vector<Point *> res;
    Point *u = from_left, *v = from_right;
    while (true) {
        if (angle(u, v, v->next) <= 0)
            v = v->next;
        else if (angle(u->prev, u, v) <= 0)
            u = u->prev;
        else
            break;
    }

    //now uv is first bridge
    size_t i = 0, j = 0, type;

    long double old_time = -INF, new_time;
    std::vector<long double> t(6, INF);
    while (1) {
        //according to original article 6 types of events when we need change
        if (i < left.size())
            t[0] = event(left[i]->prev, left[i], left[i]->next);
        if (j < right.size())
            t[1] = event(right[j]->prev, right[j], right[j]->next);
        t[2] = event(u->prev, u, v);
        t[3] = event(u, u->next, v);
        t[4] = event(u, v, v->next);
        t[5] = event(u, v->prev, v);

        new_time = INF;

        //finding min time when we need changes
        for (int ti = 0; ti < 6; ++ti) {
            if (t[ti] > old_time && t[ti] < new_time) {
                type = ti;
                new_time = t[ti];
            }
        }
        //if no such, break
        if (new_time == INF)
            break;
        //merge
        switch (type) {
            // 6 types of events
            case w_left_u:
                //if w is to the left of u, A undergoes the same event
                if (left[i]->x < u->x) {
                    res.push_back(left[i]);
                }
                left[i]->fix_links();
                ++i;
                break;
            case w_right_v:
                //if w is to the right of v, B undergoes the same event
                if (right[j]->x > v->x) {
                    res.push_back(right[j]);
                }
                right[j]->fix_links();
                ++j;
                break;
            case u_minus_uv_cw:
                res.push_back(u);
                u = u->prev;
                break;
            case uu_plus_v_ccw:
                u = u->next;
                res.push_back(u);
                break;
            case uvv_pus_cw:
                res.push_back(v);
                v = v->next;
                break;
            case uv_minus_v_ccw:
                v = v->prev;
                res.push_back(v);
                break;
        }

        old_time = new_time;
    }

    long double middle = from_left->x;
    //fixing
    u->next = v;
    v->prev = u;

    int64_t k = res.size() - 1;
    while (k >= 0) {
        Point *cur = res[k];

        if (cur->x <= u->x || cur->x >= v->x) {
            cur->fix_links();
            if (cur == u)
                u = u->prev;
            else if (cur == v)
                v = v->next;

        } else {
            u->next = cur;
            v->prev = cur;
            cur->prev = u;
            cur->next = v;

            if (cur->x <= middle)
                u = cur;
            else
                v = cur;
        }
        --k;
    }
    return res;
}

long double ConvexHull::av_edges_voronoi() const {
    std::set<std::pair<int64_t, int64_t>> edges;
    //getting points connected to infinity
    std::vector<bool> extreme(all_points.size(), false);

    for (auto p : planes) {
        if (p.p1 == -1) {
            extreme[p.p2] = true;
            extreme[p.p3] = true;
        } else if (p.p2 == -1) {
            extreme[p.p1] = true;
            extreme[p.p3] = true;
        } else if (p.p3 == -1) {
            extreme[p.p2] = true;
            extreme[p.p1] = true;
        }
    }

    //adding normal edges
    for (auto p : planes) {
        //if this plane is inf
        if (p.p1 == -1 || p.p2 == -1 || p.p3 == -1)
            continue;
        else {
            if (!extreme[p.p1] || !extreme[p.p2])
                edges.insert({std::min(p.p1, p.p2), std::max(p.p1, p.p2)});
            if (!extreme[p.p1] || !extreme[p.p3])
                edges.insert({std::min(p.p1, p.p3), std::max(p.p1, p.p3)});
            if (!extreme[p.p3] || !extreme[p.p2])
                edges.insert({std::min(p.p3, p.p2), std::max(p.p3, p.p2)});
        }
    }
    //for every end of normal edge
    std::vector<int64_t> count(all_points.size(), 0);

    for (auto p : edges) {
        if (!extreme[p.first])
            ++count[p.first];
        if (!extreme[p.second])
            ++count[p.second];
    }

    long double res = 0.0;
    //num of sights
    int64_t num = 0;

    for (auto c : count) {
        if (c > 0) {
            res += c;
            num++;
        }
    }

    res = num == 0 ? 0 : res / num;
    return res;
}

int main() {
    long double x, y;
    std::vector<Point *> v;

    int64_t i = 0;
    while (std::cin >> x >> y) {
        Point *p = new Point(x, y, x * x + y * y, i);
        v.push_back(p);
        ++i;
    }

    ConvexHull ch(v);
    std::cout << std::setprecision(9) << ch.av_edges_voronoi() << '\n';
    return 0;
}
