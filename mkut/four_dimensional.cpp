#include <cstdio>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>
#include <set>

using namespace std;

const int MAX_H = 400;
const int MAX_W = 400;
const double CANVAS_SIZE = MAX_H * MAX_W;

struct color {
    int r;
    int g;
    int b;
    int a;
    
    color(int r, int g, int b, int a) : r(r), g(g), b(b), a(a) {}
    
    bool operator<(const color& c) const {
        if (r != c.r) return r < c.r;
        if (g != c.g) return g < c.g;
        if (b != c.b) return b < c.b;
        return a < c.a;
    }

    bool operator!=(const color& c) const {
        return r != c.r || g != c.g || b != c.b || a != c.a;
    }

    bool similar(const color& c) const {
        return (r - c.r) * (r - c.r) + (g - c.g) * (g - c.g) + (b - c.b) * (b - c.b) + (a - c.a) * (a - c.a) < 1600;
    }
};

struct block {
    string id;
    int x;
    int y;
    int left;
    int bottom;

    block(string id, int x, int y, int left, int bottom) : id(id), x(x), y(y), left(left), bottom(bottom) {}

    int size() const {
        return x * y;
    }
};

int p[MAX_H][MAX_W][4];
int q[MAX_H][MAX_W][4];
double sq[255 * 255 * 4 + 1];

int total_cost;

int calc(color c1, color c2) {
    int dr = abs(c1.r - c2.r);
    int dg = abs(c1.g - c2.g);
    int db = abs(c1.b - c2.b);
    int da = abs(c1.a - c2.a);

    return sq[dr * dr + dg * dg + db * db + da * da];
}

int calc(int best, int r, int g, int b, int a, const vector<pair<color, int>>& v) {
    double sum = 0;
    for (int i = 0; i < (int)v.size(); i++) {
        int dr = abs(r - v[i].first.r);
        int dg = abs(g - v[i].first.g);
        int db = abs(b - v[i].first.b);
        int da = abs(a - v[i].first.a);
        sum += sq[dr * dr + dg * dg + db * db + da * da] * v[i].second;
        if (sum >= best) return sum;
    }
    return sum;
}

int current_score(int x1, int y1, int x2, int y2, color current) {
    int sum = 0;

    for (int i = y1; i < y2; i++) {
        for (int j = x1; j < x2; j++) {
            sum += calc(color(p[i][j][0], p[i][j][1], p[i][j][2], p[i][j][3]), current);
        }
    }

    return sum;
}

double similarity_cost(int x1, int y1, int x2, int y2, color new_color) {
    double diff = 0;
    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
            double d = 0;
            d += (p[y][x][0] - new_color.r) * (p[y][x][0] - new_color.r);
            d += (p[y][x][1] - new_color.g) * (p[y][x][1] - new_color.g);
            d += (p[y][x][2] - new_color.b) * (p[y][x][2] - new_color.b);
            d += (p[y][x][3] - new_color.a) * (p[y][x][3] - new_color.a);
            diff += sqrt(d) * 0.005;
        }
    }
    return diff;
}

double similarity_cost(int x1, int y1, int x2, int y2) {
    double diff = 0;
    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
            double d = 0;
            for (int k = 0; k < 4; k++) {
                d += (p[y][x][k] - q[y][x][k]) * (p[y][x][k] - q[y][x][k]);
            }
            diff += sqrt(d) * 0.005;
        }
    }
    return diff;
}

int similarity_cost(int w, int h) {
    double diff = 0;
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            double d = 0;
            for (int k = 0; k < 4; k++) {
                d += (p[y][x][k] - q[y][x][k]) * (p[y][x][k] - q[y][x][k]);
            }
            diff += sqrt(d) * 0.005;
        }
    }
    return round(diff);
}

color fast_get_color(map<color, int>& mp) {
    vector<pair<color, int>> v;
    for (map<color, int>::iterator it = mp.begin(); it != mp.end(); it++) v.emplace_back(it->first, it->second);
    sort(v.begin(), v.end());
    reverse(v.begin(), v.end());
    
    int best = 1e9, br = 0, bg = 0, bb = 0, ba = 0;
    for (auto m : mp) {
        auto c = m.first;
        int now = calc(best, c.r, c.g, c.b, c.a, v);
        if (now < best) {
            best = now;
            br = c.r;
            bg = c.g;
            bb = c.b;
            ba = c.a;
        }
    }

    //fprintf(stderr, "best:%d, (%d %d %d %d)\n", best / 200, br, bg, bb, ba);
    return color(br, bg, bb, ba);
}


color fast_get_color(int x1, int y1, int x2, int y2) {
    //fprintf(stderr, "(%d, %d) to (%d, %d)", x1, y1, x2, y2);
    map<color, int> mp;
    for (int i = y1; i < y2; i++) {
        for (int j = x1; j < x2; j++) {
            mp[color(p[i][j][0], p[i][j][1], p[i][j][2], p[i][j][3])]++;
        }
    }
    
    return fast_get_color(mp);
}

color get_color(int x1, int y1, int x2, int y2) {
    //fprintf(stderr, "(%d, %d) to (%d, %d)", x1, y1, x2, y2);
    int min_r = 255, max_r = 0, min_g = 255, max_g = 0, min_b = 255, max_b = 0, min_a = 255, max_a = 0;
    map<color, int> mp;
    for (int i = y1; i < y2; i++) {
        for (int j = x1; j < x2; j++) {
            mp[color(p[i][j][0], p[i][j][1], p[i][j][2], p[i][j][3])]++;
            min_r = min(min_r, p[i][j][0]);
            max_r = max(max_r, p[i][j][0]);
            min_g = min(min_g, p[i][j][1]);
            max_g = max(max_g, p[i][j][1]);
            min_b = min(min_b, p[i][j][2]);
            max_b = max(max_b, p[i][j][2]);
            min_a = min(min_a, p[i][j][3]);
            max_a = max(max_a, p[i][j][3]);
        }
    }
    vector<pair<color, int>> v;
    for (map<color, int>::iterator it = mp.begin(); it != mp.end(); it++) v.emplace_back(it->first, it->second);
    sort(v.begin(), v.end());
    reverse(v.begin(), v.end());
    //fprintf(stderr, "size:%d, r[%d, %d], g[%d, %d], b[%d, %d], a[%d, %d]\n", (int)v.size(), min_r, max_r, min_g, max_g, min_b, max_b, min_a, max_a);
    
    int best = 1e9, br = 0, bg = 0, bb = 0, ba = 0;
    const int color_div = 256;
    int r_step = max((max_r - min_r) / color_div, 1);
    int g_step = max((max_g - min_g) / color_div, 1);
    int b_step = max((max_b - min_b) / color_div, 1);
    int a_step = max((max_a - min_a) / color_div, 1);
    for (int r = min_r; r <= max_r; r += r_step) {
        // fprintf(stderr, "now:%d, best:%d, (%d %d %d %d)\n", r, best / 200, br, bg, bb, ba);
        // fflush(stderr);
        for (int g = min_g; g <= max_g; g += g_step) {
            for (int b = min_b; b <= max_b; b += b_step) {
                for (int a = min_a; a <= max_a; a += a_step) {
                    int now = calc(best, r, g, b, a, v);
                    if (now < best) {
                        best = now;
                        br = r;
                        bg = g;
                        bb = b;
                        ba = a;
                    }
                }
            }
        }
    }

    //fprintf(stderr, "best:%d, (%d %d %d %d)\n", best / 200, br, bg, bb, ba);
    return color(br, bg, bb, ba);
}

int cut_block(const block &node, char c, int val, string& ans) {
    // printf("cut [%s] [%c] [%d]\n", node.id.c_str(), c, val);
    char buf[100];
    sprintf(buf, "cut [%s] [%c] [%d]\n", node.id.c_str(), c, val);
    ans += buf;

    //fprintf(stderr, "cut cost: %f\n", round(CANVAS_SIZE / node.size() * 7));

    return round(CANVAS_SIZE / node.size() * 7);
}

int xcut_block(const block &node, int x, string& ans) {
    return cut_block(node, 'x', x, ans);
}

int ycut_block(const block &node, int y, string& ans) {
    return cut_block(node, 'y', y, ans);
}

int merge_block(const block &node1, const block &node2, string& ans) {
    // printf("merge [%s] [%s]\n", node1.id.c_str(), node2.id.c_str());
    char buf[100];
    sprintf(buf, "merge [%s] [%s]\n", node1.id.c_str(), node2.id.c_str());
    ans += buf;

    //fprintf(stderr, "merge cost: %f\n", round(CANVAS_SIZE / max(node1.size(), node2.size())));

    return round(CANVAS_SIZE / max(node1.size(), node2.size()));
}

int paint_block(const block &node, color c, string& ans) {
    // printf("color [%s] [%d, %d, %d, %d]\n", node.id.c_str(), c.r, c.g, c.b, c.a);
    char buf[100];
    sprintf(buf, "color [%s] [%d, %d, %d, %d]\n", node.id.c_str(), c.r, c.g, c.b, c.a);
    ans += buf;
    for (int x = node.left; x < node.left + node.x; x++) {
        for (int y = node.bottom; y < node.bottom + node.y; y++) {
            q[y][x][0] = c.r;
            q[y][x][1] = c.g;
            q[y][x][2] = c.b;
            q[y][x][3] = c.a;
        }
    }

    //fprintf(stderr, "paint cost: %f\n", round(CANVAS_SIZE / node.size() * 5));

    return round(CANVAS_SIZE / node.size() * 5);
}

pair<block, int> paint_column(int x, int y, vector<int>& cut_x, vector<int>& cut_y, bool asc_x, bool asc_y, block node, int& id, int draw_from, color current_color, string& ans) {
    if (y + 1 >= cut_y.size()) return make_pair(node, 0);

    int cost = 0;

    int l = min(cut_x[x], cut_x[x + 1]), r = max(cut_x[x], cut_x[x + 1]);
    int b = min(cut_y[y], cut_y[y + 1]), t = max(cut_y[y], cut_y[y + 1]);

    auto c = fast_get_color(l, b, r, t);
    if (y > 0 && !c.similar(current_color)) {
        int b2 = min(cut_y[draw_from], cut_y[y]), t2 = max(cut_y[draw_from], cut_y[y]);
        auto new_c = fast_get_color(l, b2, r, t2);
        double expected_cost = (draw_from > 0 ? round(CANVAS_SIZE / node.size() * 7) : 0)
            + round(CANVAS_SIZE / (asc_y ? (node.y - cut_y[draw_from]) : cut_y[draw_from]) / node.x * 5)
            + (draw_from > 0 ? round(CANVAS_SIZE / max(cut_y[draw_from], node.y - cut_y[draw_from]) / node.x * 1) : 0);

        double expected_gain = similarity_cost(l, b2, r, t2)
            - similarity_cost(l, b2, r, t2, new_c);

        if (expected_gain > expected_cost) {
            // cut bottom row
            block node0 = node, node1 = node;
            if (draw_from > 0) {
                cost += ycut_block(node, cut_y[draw_from], ans);
                node0.y = cut_y[draw_from];
                node0.id += ".0";
                node1.bottom = cut_y[draw_from];
                node1.y -= node0.y;
                node1.id += ".1";
            }

            // paint row
            cost += paint_block(asc_y ? node1 : node0, new_c, ans);

            // merge bottom row
            if (draw_from > 0) {
                cost += merge_block(node0, node1, ans);
                node.id = to_string(++id);
            }
        }

        draw_from = y;
    }

    if (y + 2 == cut_y.size()) {
        int b3 = min(cut_y[draw_from], cut_y[y]), t3 = max(cut_y[draw_from], cut_y[y]);
        auto new_c = fast_get_color(l, b3, r, t3);
        double expected_cost = (draw_from > 0 ? round(CANVAS_SIZE / node.size() * 7) : 0)
            + round(CANVAS_SIZE / (asc_y ? (node.y - cut_y[draw_from]) : cut_y[draw_from]) / node.x * 5)
            + (draw_from > 0 ? round(CANVAS_SIZE / max(cut_y[draw_from], (node.y - cut_y[draw_from])) / node.x * 1) : 0);

        double expected_gain = similarity_cost(l, b3, r, t3)
            - similarity_cost(l, b3, r, t3, new_c);

        if (expected_gain > expected_cost) {
            // cut bottom row
            block node0 = node, node1 = node;
            if (draw_from > 0) {
                cost += ycut_block(node, cut_y[draw_from], ans);
                node0.y = cut_y[draw_from];
                node0.id += ".0";
                node1.bottom = cut_y[draw_from];
                node1.y -= node0.y;
                node1.id += ".1";
            }

            // paint row
            cost += paint_block(asc_y ? node1 : node0, new_c, ans);

            // merge bottom row
            if (draw_from > 0) {
                cost += merge_block(node0, node1, ans);
                node.id = to_string(++id);
            }
        }

        draw_from = y;
    }

    auto res = paint_column(x, y + 1, cut_x, cut_y, asc_x, asc_y, node, id, draw_from, c, ans);

    return make_pair(res.first, res.second + cost);
}

int paint(int x, vector<int>& cut_x, vector<int>& cut_y, bool asc_x, bool asc_y, block node, int& id, string& ans) {
    if (x + 1 >= cut_x.size()) return 0;

    int cost = 0;

    // cut target area
    block node0 = node, node1 = node;
    if (x > 0) {
        cost += xcut_block(node, cut_x[x], ans);
        node0.id += ".0"; node1.id += ".1";
        node0.x = cut_x[x]; node1.x -= cut_x[x];
        node1.left = cut_x[x];
    }
    
    // paint_column
    block next_node = x == 0 ? node : asc_x ? node1 : node0;
    block other_node = x == 0 ? node : asc_x ? node0 : node1;
    auto ret = paint_column(x, 0, cut_x, cut_y, asc_x, asc_y, next_node, id, 0, color(0,0,0,0), ans);
    node = ret.first;
    cost += ret.second;

    // merge painted row
    if (x > 0) {
        cost += merge_block(other_node, node, ans);
        node.x = other_node.x + node.x;
        node.left = min(other_node.left, node.left);
        node.id = to_string(++id);
    }

    cost += paint(x + 1, cut_x, cut_y, asc_x, asc_y, node, id, ans);

    return cost;
}

void auto_border(int w, int h, vector<int>& cut_x, vector<int>& cut_y) {
    int min_distance = 10;
    int cut_count = 10000;

    vector<pair<int, double>> diff_x;
    for (int x = 0; x + 1 < w; x++) {
        double diff = 0;
        for (int y = 0; y < h; y++) {
            double d = 0;
            for (int k = 0; k < 4; k++) {
                d += (p[y][x][k] - p[y][x+1][k]) * (p[y][x][k] - p[y][x+1][k]);
            }
            diff += sqrt(d);
        }
        diff_x.push_back(make_pair(x, diff));
    }
    sort(diff_x.begin(), diff_x.end(), [](auto const& lhs, auto const& rhs){
        return lhs.second > rhs.second;
    });

    set<int> xs;
    cut_x.push_back(0); cut_x.push_back(w);
    xs.insert(0); xs.insert(w);
    for (int i = 0; xs.size() < cut_count && i < diff_x.size(); i++) {
        int x = diff_x[i].first + 1;
        bool ng = false;
        for (int j = x - min_distance; j <= x + min_distance; j++) {
            if (xs.count(j)) {
                ng = true;
                break;
            }
        }
        if (ng) {
            continue;
        }
        cut_x.push_back(x);
        xs.insert(x);
    }
    sort(cut_x.begin(), cut_x.end());

    vector<pair<int, double>> diff_y;
    for (int y = 0; y + 1 < h; y++) {
        double diff = 0;
        for (int x = 0; x < w; x++) {
            double d = 0;
            for (int k = 0; k < 4; k++) {
                d += (p[y][x][k] - p[y+1][x][k]) * (p[y][x][k] - p[y+1][x][k]);
            }
            diff += sqrt(d);
        }
        diff_y.push_back(make_pair(y, diff));
    }
    sort(diff_y.begin(), diff_y.end(), [](auto const& lhs, auto const& rhs){
        return lhs.second > rhs.second;
    });
    fprintf(stderr, "cut_x: ");
    for (auto t : cut_x) {
        fprintf(stderr, "%d ", t);
    }
    fprintf(stderr, "\n");

    set<int> ys;
    cut_y.push_back(0); cut_y.push_back(w);
    ys.insert(0);
    ys.insert(w);
    for (int i = 0; ys.size() < cut_count && i < diff_y.size(); i++) {
        int y = diff_y[i].first + 1;
        bool ng = false;
        for (int j = y - min_distance; j <= y + min_distance; j++) {
            if (ys.count(j)) {
                ng = true;
                break;
            }
        }
        if (ng) {
            continue;
        }
        cut_y.push_back(y);
        ys.insert(y);
    }
    sort(cut_y.begin(), cut_y.end());
    fprintf(stderr, "cut_y: ");
    for (auto t : cut_y) {
        fprintf(stderr, "%d ", t);
    }
    fprintf(stderr, "\n");
}

void general(int w, int h, vector<int>& cut_x, vector<int>& cut_y) {
    double target_cost = 4000;
    double min_inv = 40;
    cut_x.push_back(w);
    cut_y.push_back(h);
    for (double t = 1; t > 0; ) {
        double dt = 1 / sqrt(target_cost * (1 + 1.0 / pow(target_cost, 0.25) - t) * (1 + 1.0 / pow(target_cost, 0.25) - t));
        if (dt < 1 / min_inv) {
            dt = 1.0 / min_inv;
        }
        t -= dt;
        if (t < 0 || (int)(w * t) == 0 || (int)(h * t) == 0) {
            t = 0;
        }
        cut_x.push_back(w * t);
        cut_y.push_back(h * t);
        fprintf(stderr, "%.4f\n", t);
    }
    reverse(cut_x.begin(), cut_x.end());
    reverse(cut_y.begin(), cut_y.end());

    for (int a : cut_x) {
        fprintf(stderr, "%d ", a);
    }
    fprintf(stderr, "\n");
}

int main() {
    int w, h;
    scanf("%d %d", &w, &h);
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                scanf("%d", &p[h - 1 - i][j][k]);
            }
        }
    }
    
    for (int i = 0; i <= 255 * 255 * 4; i++) sq[i] = sqrt(i);

    // #3
    // vector<int> cut_x = {0, 16, 41, 67, 102, 118, 144, 169, 195, 200, 221, 237, 263, 272, 289, 298, 314, 323, 341, 349, 375, 400};
    // vector<int> cut_y = {0, 25, 51, 77, 102, 128, 142, 153, 167, 179, 182, 193, 205, 208, 226, 233, 252, 259, 285, 303, 310, 336, 400};

    // #9
    // vector<int> cut_x = {0, 31, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240, 250, 270, 290, 310, 330, 350, 377, 400};
    // vector<int> cut_y = {0, 32, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240, 250, 280, 300, 320, 340, 360, 383, 400};

    // #15
    // vector<int> cut_x = {0, 19, 40, 87, 97, 107, 117, 127, 137, 147, 157, 167, 177, 187, 197, 207, 217, 227, 237, 247, 257, 267, 277, 400};
    // vector<int> cut_y = {0, 13, 42, 76, 87, 98, 109, 120, 131, 142, 152, 162, 172, 182, 192, 202, 212, 222, 232, 242, 252, 262, 272, 282, 292, 302, 312, 322, 332, 342, 400};

    // #21
    // vector<int> cut_x = {0, 8, 42, 48, 93, 100, 144, 151, 247, 254, 311, 319, 375, 383, 400};
    // vector<int> cut_y = {0, 8, 14, 19, 24, 47, 53, 95, 103, 147, 154, 249, 257, 350, 358, 400};

    // #23
    // vector<int> cut_x = {0, 42, 126, 160, 196, 205, 208, 223, 227, 305, 358, 400};
    // vector<int> cut_y = {0, 171, 180, 192, 211, 214, 242, 320, 324, 329, 334, 369, 400};

    vector<int> cut_x, cut_y;
    auto_border(w, h, cut_x, cut_y);

    string best_ans;
    int best_score = 1e8;

    for (int k = 0; k < 4; k++) {
        bool asc_x = !(k % 2);
        bool asc_y = !(k / 2);
        for (int x = 0; x < w; x++) {
            for (int y = 0; y < h; y++) {
                for (int j = 0; j < 4; j++) {
                    q[y][x][j] = 255;
                }
            }
        }
        string ans;
        int id = 0;
        int cost = paint(0, cut_x, cut_y, asc_x, asc_y, block("0", MAX_W, MAX_H, 0, 0), id, ans);
        int similarity = round(similarity_cost(w, h));

        fprintf(stderr, "manipulation cost: %d\n", cost);
        fprintf(stderr, "similarity cost: %d\n", similarity);
        fprintf(stderr, "total cost: %d\n", cost + similarity);

        if (best_score > cost + similarity) {
            best_score = cost + similarity;
            best_ans = ans;
        }

        reverse(cut_x.begin(), cut_x.end());
        if (k % 2) {
            reverse(cut_y.begin(), cut_y.end());
        }
    }

    printf("%s", best_ans.c_str());
    
    return 0;
}
