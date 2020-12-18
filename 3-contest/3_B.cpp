#include <vector>
#include <iostream>
#include <cassert>
#include <map>

const int HEIGHT = 8, WIDTH = 8;
const int ALL_POS = WIDTH * HEIGHT;
//pos = y * width + x

const std::vector<std::pair<int, int>> shift = {{-1, -1}, {-1, 0}, {-1, 1},{0, 1},
                                                {1, 1}, {1, 0}, {1, -1}, {0, -1}};

struct Step{
    int term_x, term_y, run_x, run_y;
    bool is_terminator_step;
};

struct Graph{
    struct State{
        bool used, win, loss;
        std::vector<int> prev_ind;
        int degree;
    };

    //array of all states if field
    std::vector<State> states;
    std::vector<std::string> field;

    Graph(std::vector<std::string> input_field);

    //getting x, y, and is_terminator_step step info
    Step get_step(size_t index) {
        int term_x = (index / (2 * ALL_POS)) % WIDTH,
            term_y = (index / (2 * ALL_POS * WIDTH)),
            run_x = (index % (2 * ALL_POS * WIDTH)) % (2 * WIDTH) / 2 ,
            run_y = (index % (2 * ALL_POS)) / (2 * WIDTH);

        bool is_term_step = (index % 2) != 0;
        return Step{term_x, term_y, run_x, run_y, is_term_step};
    }

    static size_t get_index(const Step &t){
        size_t res = 0;
        res += t.term_y * ALL_POS * WIDTH * 2;
        res += t.term_x * ALL_POS * 2;
        res += t.run_y * WIDTH * 2;
        res += t.run_x * 2;
        res += t.is_terminator_step;
        return res;
    }

    //check if terminator and runner on the same line without walls
    bool on_line(Step t){
        if(t.run_y == t.term_y){
            if(t.run_x > t.term_x)
                std::swap(t.run_x, t.term_x);
            while(t.run_x < t.term_x){
                if(field[t.run_y][t.run_x] == '1')
                    return false;
                ++t.run_x;
            }
            return true;
        }
        if(t.run_x == t.term_x){
            if(t.run_y > t.term_y)
                std::swap(t.run_y, t.term_y);
            while(t.run_y < t.term_y){
                if(field[t.run_y][t.run_x] == '1')
                    return false;
                ++t.run_y;
            }
            return true;
        }
        if((t.run_x - t.term_x) == (t.run_y - t.term_y)){
            if(t.run_x > t.term_x) {
                std::swap(t.run_x, t.term_x);
                std::swap(t.run_y, t.term_y);
            }

            while(t.run_x < t.term_x){
                if(field[t.run_y][t.run_x] == '1')
                    return false;
                ++t.run_x;
                ++t.run_y;
            }
            return true;
        }
        if((t.run_x - t.term_x) == -(t.run_y - t.term_y)){
            if(t.run_x > t.term_x) {
                std::swap(t.run_x, t.term_x);
                std::swap(t.run_y, t.term_y);
            }

            while(t.run_x < t.term_x){
                if(field[t.run_y][t.run_x] == '1')
                    return false;
                ++t.run_x;
                --t.run_y;
            }
            return true;
        }
        return false;
    }

    bool check(const Step &t){
        size_t i = get_index(t);
        if (!t.is_terminator_step) {
            //if it is runner
            states[i].win = (t.run_y == HEIGHT - 1) && !on_line(t);
            states[i].loss = on_line(t);
        }
        else{
            //terminator
            states[i].win = on_line(t);
        }
        return (states[i].win || states[i].loss);
    }

    void search(size_t cur) {
        states[cur].used = true;

        for (auto prev : states[cur].prev_ind) {
            if (!states[prev].used) {
                if (states[cur].loss) {
                    //assert(!states[prev].loss);
                    states[prev].win = true;
                } else if (--states[prev].degree == 0) {
                    //assert(states[cur].win);
                    states[prev].loss = true;
                }
                else
                    continue;

                search(prev);
            }
        }
    }

    void answer() {
        int term_st_x, term_st_y, run_st_x, run_st_y;
        //find
        for (int i = 0; i < WIDTH; ++i){
            for (int j = 0; j < HEIGHT; ++j) {
                if (field[j][i] == '3') {
                    term_st_x = i;
                    term_st_y = j;
                    continue;
                } else if (field[j][i] == '2') {
                    run_st_x = i;
                    run_st_y = j;
                }
            }
        }

        size_t start_ind = get_index(Step{term_st_x, term_st_y, run_st_x, run_st_y, false});
        //assert(states[start_ind].win || states[start_ind].loss);
        if (states[start_ind].win)
            std::cout << "1";
        else
            std::cout << "-1";
    }
};

Graph::Graph(std::vector<std::string> input_field) : field(input_field), states(ALL_POS * ALL_POS * 2){
    //pos of terminator * pos of runner * 2(whose step)
    for(size_t i = 0; i < states.size(); ++i){
        Step t = get_step(i);
        //if it is not real state
        if (field[t.term_y][t.term_x] == '1' || field[t.run_y][t.run_x] == '1')  continue;

        check(t);

        if((t.run_y == HEIGHT - 1) && !t.is_terminator_step)
            ++states[i].degree;

        for(auto d : shift){
            int prev_term_x = t.term_x, prev_term_y = t.term_y, prev_run_x = t.run_x, prev_run_y = t.run_y;

            if(!t.is_terminator_step)
                prev_term_x += d.first, prev_term_y += d.second;
            else
                prev_run_x += d.first, prev_run_y += d.second;

            if((0 <= prev_term_x && prev_term_x < WIDTH && 0 <= prev_term_y && prev_term_y < HEIGHT) &&
               (0 <= prev_run_x && prev_run_x < WIDTH && 0 <= prev_run_y && prev_run_y < HEIGHT) &&
               field[prev_run_y][prev_run_x] != '1' && field[prev_term_y][prev_term_x] != '1') {
                //if step in field
                size_t prev = get_index(Step{prev_term_x, prev_term_y, prev_run_x, prev_run_y, !t.is_terminator_step});

                states[i].prev_ind.push_back(prev);
                ++states[prev].degree;
            }
        }
    }

    //dfs
    for(size_t i = 0; i < states.size(); ++i) {
        if(!states[i].used && (states[i].win || states[i].loss)){
            search(i);
        }
    }
}

int main(){
    std::vector<std::string> field(HEIGHT);
    for (int i = 0; i < HEIGHT; ++i) {
        std::cin >> field[i];
    }

    Graph g(field);
    g.answer();

    return 0;
}

