#include <cstdio>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>

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

    block(string id, int x, int y) : id(id), x(x), y(y) {}

    int size() const {
        return x * y;
    }
};

int p[MAX_H][MAX_W][4];
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

int cut_block(const block &node, char c, int val) {
    printf("cut [%s] [%c] [%d]\n", node.id.c_str(), c, val);

    //fprintf(stderr, "cut cost: %f\n", round(CANVAS_SIZE / node.size() * 7));

    return round(CANVAS_SIZE / node.size() * 7);
}

int xcut_block(const block &node, int x) {
    return cut_block(node, 'x', x);
}

int ycut_block(const block &node, int y) {
    return cut_block(node, 'y', y);
}

int merge_block(const block &node1, const block &node2) {
    printf("merge [%s] [%s]\n", node1.id.c_str(), node2.id.c_str());

    //fprintf(stderr, "merge cost: %f\n", round(CANVAS_SIZE / max(node1.size(), node2.size())));

    return round(CANVAS_SIZE / max(node1.size(), node2.size()));
}

int paint_block(const block &node, color c) {
    printf("color [%s] [%d, %d, %d, %d]\n", node.id.c_str(), c.r, c.g, c.b, c.a);

    //fprintf(stderr, "paint cost: %f\n", round(CANVAS_SIZE / node.size() * 5));

    return round(CANVAS_SIZE / node.size() * 5);
}

pair<block, int> paint_column(int x, int y, vector<int>& cut_x, vector<int>& cut_y, block node, int& id, int draw_from, color current_color) {
    if (y + 1 >= cut_y.size()) return make_pair(node, 0);

    int cost = 0;

    auto c = fast_get_color(cut_x[x], cut_y[y], cut_x[x + 1], cut_y[y + 1]);
    if (y > 0 && !c.similar(current_color)) {
        // cut bottom row
        block node0 = node, node1 = node;
        if (draw_from > 0) {
            cost += ycut_block(node, cut_y[draw_from]);
            node0.y = cut_y[draw_from];
            node0.id += ".0";
            node1.y -= node0.y;
            node1.id += ".1";
        }

        // paint row
        auto new_c = fast_get_color(cut_x[x], cut_y[draw_from], cut_x[x + 1], cut_y[y]);
        cost += paint_block(node1, new_c);

        // merge bottom row
        if (draw_from > 0) {
            cost += merge_block(node0, node1);
            node.id = to_string(++id);
        }

        draw_from = y;
    }

    if (y + 2 == cut_y.size()) {
        // cut bottom row
        block node0 = node, node1 = node;
        if (draw_from > 0) {
            cost += ycut_block(node, cut_y[draw_from]);
            node0.y = cut_y[draw_from];
            node0.id += ".0";
            node1.y -= node0.y;
            node1.id += ".1";
        }

        // paint row
        auto new_c = fast_get_color(cut_x[x], cut_y[draw_from], cut_x[x + 1], cut_y[y + 1]);
        cost += paint_block(node1, new_c);

        // merge bottom row
        if (draw_from > 0) {
            cost += merge_block(node0, node1);
            node.id = to_string(++id);
        }

        draw_from = y;
    }

    auto r = paint_column(x, y + 1, cut_x, cut_y, node, id, draw_from, c);

    return make_pair(r.first, r.second + cost);
}

int paint(int x, vector<int>& cut_x, vector<int>& cut_y, block painted_node, block node, int& id) {
    if (x + 1 >= cut_x.size()) return 0;
    
    // paint_column
    auto ret = paint_column(x, 0, cut_x, cut_y, node, id, 0, color(0,0,0,0));
    node = ret.first;
    int cost = ret.second;

    // merge painted row
    if (!painted_node.id.empty()) {
        cost += merge_block(painted_node, node);
        node.x += painted_node.x;
        node.id = to_string(++id);
    }

    // cut target area
    block next_node = node;
    if (x + 2 < cut_x.size()) {
        cost += xcut_block(node, cut_x[x + 1]);
        node.x = cut_x[x + 1];
        node.id += ".0";

        next_node.x -= node.x;
        next_node.id += ".1";
    }

    cost += paint(x + 1, cut_x, cut_y, node, next_node, id);

    return cost;
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

    printf("XXXXXXXXXXXXX\n");
    for (int i = 0; i < 20; i++) {
        printf("%d: %.4f\n", diff_x[i].first + 1, diff_x[i].second);
    }

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

    printf("YYYYYYYYYYYY\n");
    for (int i = 0; i < 30; i++) {
        printf("%d: %.4f\n", diff_y[i].first + 1, diff_y[i].second);
    }
    
    return 0;
}
