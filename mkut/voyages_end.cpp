#include <cstdio>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <iostream>

using namespace std;

const int MAX_H = 400;
const int MAX_W = 400;
const double CANVAS_SIZE = MAX_H * MAX_W;

double sq[255 * 255 * 4 + 1];
void init_sq() {
    for (int i = 0; i <= 255 * 255 * 4; i++) sq[i] = sqrt(i);
}

struct color {
    int r;
    int g;
    int b;
    int a;
    
    color() : r(-1), g(-1), b(-1), a(-1) {}
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

    friend istream& operator>>(istream& is, color& c) {
        return is >> c.r >> c.g >> c.b >> c.a;
    }

    friend ostream& operator<<(ostream& os, const color& c) {
        return os << "[" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << "]";
    }
};

struct block {
    string id;
    int left;
    int bottom;
    int width;
    int height;

    block() : id(""), left(-1), bottom(-1), width(-1), height(-1) {}
    block(string id, int left, int bottom, int width, int height) : id(id), left(left), bottom(bottom), width(width), height(height) {}

    int size() const {
        return width * height;
    }

    pair<block, block> xcut(int x) const {
        block node0(id + ".0", left, bottom, x, height);
        block node1(id + ".1", x, bottom, width - x, height);
        return make_pair(node0, node1);
    }

    pair<block, block> ycut(int y) const {
        block node0(id + ".0",left, bottom, width, y);
        block node1(id + ".1",left, y, width, height - y);
        return make_pair(node0, node1);
    }

    friend istream& operator>>(istream& is, block& c) {
        int right, top;
        is >> c.id >> c.left >> c.bottom >> right >> top;
        c.width = right - c.left;
        c.height = top - c.bottom;
        return is;
    }

    friend ostream& operator<<(ostream& os, const block& b) {
        return os << "[#" << b.id << " (" << b.left << ", " << b.bottom << ") to (" << (b.left + b.width) << ", " << (b.bottom + b.height) << ")]";
    }
};

struct grid {
    vector<vector<block>> B;
    vector<vector<color>> C;
    vector<int> cut_x;
    vector<int> cut_y;
    int id;

    void paint_block(const block& node, const color& c) {
        int xid_from = to_xid(node.left);
        int xid_to = to_xid(node.left + node.width);
        int yid_from = to_yid(node.bottom);
        int yid_to = to_yid(node.bottom + node.height);
        for (int xid = xid_from; xid < xid_to; xid++) {
            for (int yid = yid_from; yid < yid_to; yid++) {
                C[xid][yid] = c;
            }
        }
    }

    pair<block, block> xcut_block(const block& node, int x) {
        int xid_mid = to_xid(x);
        if (cut_x[xid_mid] != x) {
            xid_mid++;
            cut_x.insert(cut_x.begin() + xid_mid, x);

            vector<block> new_column;
            vector<color> new_color_column;
            for (int yid = 0; yid < cut_y.size(); yid++) {
                new_column.push_back(B[xid_mid - 1][yid]);
                new_color_column.push_back(C[xid_mid - 1][yid]);
            }
            B.insert(B.begin() + xid_mid, new_column);
            C.insert(C.begin() + xid_mid, new_color_column);
        }

        auto children = node.xcut(x);
        int xid_from = to_xid(node.left);
        int xid_to = to_xid(node.left + node.width);
        int yid_from = to_yid(node.bottom);
        int yid_to = to_yid(node.bottom + node.height);
        for (int yid = yid_from; yid < yid_to; yid++) {
            for (int xid = xid_from; xid < xid_mid; xid++)  {
                B[xid][yid] = children.first;
            }
            for (int xid = xid_mid; xid < xid_to; xid++)  {
                B[xid][yid] = children.second;
            }
        }
        return children;
    }

    pair<block, block> ycut_block(const block& node, int y) {
        int yid_mid = to_yid(y);
        if (cut_y[yid_mid] != y) {
            yid_mid++;
            cut_y.insert(cut_y.begin() + yid_mid, y);

            for (int xid = 0; xid < cut_x.size(); xid++) {
                B[xid].insert(B[xid].begin() + yid_mid, B[xid][yid_mid - 1]);
                C[xid].insert(C[xid].begin() + yid_mid, C[xid][yid_mid - 1]);
            }
        }

        auto children = node.ycut(y);
        int xid_from = to_xid(node.left);
        int xid_to = to_xid(node.left + node.width);
        int yid_from = to_yid(node.bottom);
        int yid_to = to_yid(node.bottom + node.height);
        for (int xid = xid_from; xid < xid_to; xid++) {
            for (int yid = yid_from; yid < yid_mid; yid++)  {
                B[xid][yid] = children.first;
            }
            for (int yid = yid_mid; yid < yid_to; yid++)  {
                B[xid][yid] = children.second;
            }
        }
        return children;
    }

    block merge_block(const block& node1, const block& node2) {
        int left = min(node1.left, node2.left);
        int bottom = min(node1.bottom, node2.bottom);
        int width = node1.left == node2.left ? node1.width : node1.width + node2.width;
        int height = node1.bottom == node2.bottom ? node1.height : node1.height + node2.height;
        block new_block(to_string(++id), left, bottom, width, height);

        int xid_from = to_xid(left);
        int xid_to = to_xid(left + width);
        int yid_from = to_yid(bottom);
        int yid_to = to_yid(bottom + height);

        for (int xid = xid_from; xid < xid_to; xid++) {
            for (int yid = yid_from; yid < yid_to; yid++)  {
                B[xid][yid] = new_block;
            }
        }

        return new_block;
    }

    int to_xid(int x) const {
        return upper_bound(cut_x.begin(), cut_x.end(), x) - cut_x.begin() - 1;
    }
    int to_yid(int y) const {
        return upper_bound(cut_y.begin(), cut_y.end(), y) - cut_y.begin() - 1;
    }

    const color& get_color(int x, int y) const {
        return C[to_xid(x)][to_yid(y)];
    }
};

struct setting {
    vector<int> cut_x;
    vector<int> cut_y;

    setting(vector<int>& cut_x, vector<int>& cut_y) : cut_x(cut_x), cut_y(cut_y) {}
};

struct problem {
    int target[MAX_H][MAX_W][4];
    int initial[MAX_H][MAX_W][4];
    int width;
    int height;
    int initial_id = 0;
    vector<pair<block, color>> initial_blocks;

    friend istream& operator>> (istream& is, problem& P) {
        // target image
        is >> P.width >> P.height;
        for (int k = 0; k < 4; k++) {
            for (int i = 0; i < P.height; i++) {
                for (int j = 0; j < P.width; j++) {
                    is >> P.target[P.height - 1 - i][j][k];
                }
            }
        }

        // initial blocks
        int n; is >> n;
        if (is.eof()) {
            // lightning format
            P.initial_blocks.push_back(make_pair(block("0", 0, 0, P.width, P.height), color(255, 255, 255, 255)));
        } else {
            for (int i = 0; i < n; i++) {
                block b("", -1, -1, -1, -1);
                color c(-1, -1, -1, -1);
                is >> b >> c;
                P.initial_blocks.push_back(make_pair(b, c));
                P.initial_id = max(P.initial_id, atoi(b.id.c_str()));
            }
        }

        // initialize
        for (auto bb : P.initial_blocks) {
            block b = bb.first;
            color c = bb.second;

            for (int x = b.left; x < b.left + b.width; x++) {
                for (int y = b.bottom; y < b.bottom + b.height; y++) {
                    P.initial[y][x][0] = c.r;
                    P.initial[y][x][1] = c.g;
                    P.initial[y][x][2] = c.b;
                    P.initial[y][x][3] = c.a;
                }
            }
        }

        return is;
    }
};

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

struct solver {
    const problem& P;
    const setting& S;

    grid G;

    ostream& out;
    int cost = 0;

    solver(const problem& P, const setting& S, ostream& out) : P(P), S(S), out(out) {
        set<int> cut_x, cut_y;
        cut_x.insert(P.width); cut_y.insert(P.height);
        for (auto bc : P.initial_blocks) {
            cut_x.insert(bc.first.left);
            cut_y.insert(bc.first.bottom);
        }
        G.id = P.initial_id;
        G.cut_x = vector<int>(cut_x.begin(), cut_x.end());
        G.cut_y = vector<int>(cut_y.begin(), cut_y.end());
        G.B = vector<vector<block>>(G.cut_x.size(), vector<block>(G.cut_y.size()));
        G.C = vector<vector<color>>(G.cut_x.size(), vector<color>(G.cut_y.size()));
        for (auto bc : P.initial_blocks) {
            int xid = G.to_xid(bc.first.left);
            int yid = G.to_yid(bc.first.bottom);
            G.B[xid][yid] = bc.first;
            G.C[xid][yid] = bc.second;
        }
    }

    void solve() {
        if (P.initial_blocks.size() == 1) {
            // lightning
            block root("0", 0, 0, P.width, P.height);
            for (int xid = 0; xid + 1 < S.cut_x.size(); xid++) {
                root = paint_column(xid, root);
            }
            int similarity = similarity_cost();

            fprintf(stderr, "manipulation cost: %d\n", cost);
            fprintf(stderr, "similarity cost: %d\n", similarity);
            fprintf(stderr, "total cost: %d\n", cost + similarity);
        } else {
            vector<vector<color>> new_target(G.cut_x.size(), vector<color>(G.cut_y.size()));
            set<color> colors;
            for (int xid = 0; xid + 1 < G.cut_x.size(); xid++) {
                for (int yid = 0; yid + 1 < G.cut_y.size(); yid++) {
                    block node = G.B[xid][yid];
                    color c = fast_get_color(node.left, node.bottom, node.left + node.width, node.bottom + node.height);
                    new_target[xid][yid] = c;
                    colors.insert(c);
                }
            }

            double grid_size = G.B[0][0].size();
            double expected_cost_table[21][21] = {};
            for (int shorter = 0; shorter <= 20; shorter++) {
                for (int longer = shorter; longer <= 20; longer++) {
                    double expected_cost = 0;
                    for (int i = 0; i < longer - 1; i++) {
                        expected_cost += round(CANVAS_SIZE / grid_size / (i+1) * 1);
                    }
                    expected_cost *= shorter;
                    for (int i = 0; i < shorter - 1; i++) {
                        expected_cost += round(CANVAS_SIZE / grid_size / longer / (i+1) * 1);
                    }
                    expected_cost += round(CANVAS_SIZE / grid_size / longer / shorter * 5);
                    expected_cost_table[shorter][longer] = expected_cost;
                }
            }

            vector<vector<bool>> painted(G.cut_x.size(), vector<bool>(G.cut_y.size()));
            while(true) {
                double max_revenue = 0;
                int max_xid_from, max_xid_to, max_yid_from, max_yid_to;
                color max_c;
                for (int xid_from = 0; xid_from < G.cut_x.size(); xid_from++) {
                    for (int xid_to = xid_from + 1; xid_to - xid_from <= 1000 && xid_to < G.cut_x.size(); xid_to++) {
                        for (int yid_from = 0; yid_from < G.cut_y.size(); yid_from++) {
                            for (int yid_to = yid_from + 1; yid_to - yid_from <= 1000 && yid_to < G.cut_y.size(); yid_to++) {
                                bool ng = false;
                                for (int i = xid_from; i < xid_to; i++) {
                                    if (painted[i][yid_to - 1]) {
                                        ng = true;
                                        break;
                                    }
                                }
                                if (ng) break;
                                int shorter = min(xid_to - xid_from, yid_to - yid_from);
                                int longer = max(xid_to - xid_from, yid_to - yid_from);
                                color c = fast_get_color(G.cut_x[xid_from], G.cut_y[yid_from], G.cut_x[xid_to], G.cut_y[yid_to]);
                                double expected_cost = expected_cost_table[shorter][longer];
                                double expected_gain = similarity_cost(G.cut_x[xid_from], G.cut_y[yid_from], G.cut_x[xid_to], G.cut_y[yid_to])
                                    - similarity_cost(G.cut_x[xid_from], G.cut_y[yid_from], G.cut_x[xid_to], G.cut_y[yid_to], c);
                                double revenue_efficiency = expected_gain - expected_cost;
                                if (revenue_efficiency > max_revenue) {
                                    max_revenue = revenue_efficiency;
                                    max_xid_from = xid_from;
                                    max_xid_to = xid_to;
                                    max_yid_from = yid_from;
                                    max_yid_to = yid_to;
                                    max_c = c;
                                }
                            }
                        }
                    }
                }
                if (max_revenue == 0) break;

                // merge and color
                bool x_first = max_xid_to - max_xid_from > max_yid_to - max_yid_from;
                block main1;
                if (x_first) {
                    for (int yid = max_yid_from; yid < max_yid_to; yid++) {
                        block main2 = G.B[max_xid_from][yid];
                        for (int xid = max_xid_from + 1; xid < max_xid_to; xid++) {
                            main2 = merge_block(main2, G.B[xid][yid]);
                        }
                        if (yid != max_yid_from) {
                            main1 = merge_block(main1, main2);
                        } else {
                            main1 = main2;
                        }
                    }
                } else {
                    for (int xid = max_xid_from; xid < max_xid_to; xid++) {
                        block main2 = G.B[xid][max_yid_from];
                        for (int yid = max_yid_from + 1; yid < max_yid_to; yid++) {
                            main2 = merge_block(main2, G.B[xid][yid]);
                        }
                        if (xid != max_xid_from) {
                            main1 = merge_block(main1, main2);
                        } else {
                            main1 = main2;
                        }
                    }
                }
                for (int xid = max_xid_from; xid < max_xid_to; xid++) {
                    for (int yid = max_yid_from; yid < max_yid_to; yid++) {
                        painted[xid][yid] = true;
                    }
                }

                paint_block(main1, max_c);
                // cerr << "(" << max_xid_from << "," << max_yid_from << ") (" << max_xid_to << "," << max_yid_to << ") :" << max_c << endl;
                int similarity = similarity_cost();

                fprintf(stderr, "manipulation cost: %d\n", cost);
                fprintf(stderr, "similarity cost: %d\n", similarity);
                fprintf(stderr, "total cost: %d\n", cost + similarity);
            }

            int similarity = similarity_cost();

            fprintf(stderr, "manipulation cost: %d\n", cost);
            fprintf(stderr, "similarity cost: %d\n", similarity);
            fprintf(stderr, "total cost: %d\n", cost + similarity);
        }
    }

    block paint_column(int xid, const block& node) {
        block to_draw = node, rest = node;
        // cut target area
        if (xid > 0) {
            auto res = xcut_block(node, S.cut_x[xid]);
            to_draw = res.second;
            rest = res.first;
        }

        // paint_column
        int yid_draw_from = 0;
        color current_color(-1, -1, -1, -1);
        for (int yid = 0; yid + 1 < S.cut_y.size(); yid++) {
            color c = fast_get_color(S.cut_x[xid], S.cut_y[yid], S.cut_x[xid + 1], S.cut_y[yid + 1]);
            if (current_color.a == -1) {
                current_color = c;
                yid_draw_from = yid;
            } else if (!current_color.similar(c)) {
                to_draw = paint_cell(xid, yid_draw_from, yid, to_draw);
                current_color = c;
                yid_draw_from = yid;
            } else {
                current_color = c; // only use last color?
            }
        }
        to_draw = paint_cell(xid, yid_draw_from, S.cut_y.size() - 1, to_draw);

        // merge painted row
        if (xid > 0) {
            to_draw = merge_block(to_draw, rest);
        }

        return to_draw;
    }

    block paint_cell(int xid, int yid_from, int yid_to, const block& node) {
        color c = fast_get_color(S.cut_x[xid], S.cut_y[yid_from], S.cut_x[xid + 1], S.cut_y[yid_to]);
        double expected_cost = (yid_from > 0 ? round(CANVAS_SIZE / node.size() * 7) : 0)
            + round(CANVAS_SIZE / (node.height - S.cut_y[yid_from]) / node.width * 5)
            + (yid_from > 0 ? round(CANVAS_SIZE / max(S.cut_y[yid_from], (node.height - S.cut_y[yid_from])) / node.width * 1) : 0);

        double expected_gain = similarity_cost(S.cut_x[xid], S.cut_y[yid_from], S.cut_x[xid + 1], S.cut_y[yid_to])
            - similarity_cost(S.cut_x[xid], S.cut_y[yid_from], S.cut_x[xid + 1], S.cut_y[yid_to], c);

        if (expected_gain > expected_cost) {
            block to_draw = node, rest = node;
            // cut bottom row
            if (yid_from > 0) {
                auto res = ycut_block(node, S.cut_y[yid_from]);
                to_draw = res.second;
                rest = res.first;
            }

            // paint row
            paint_block(to_draw, c);

            // merge bottom row
            if (yid_from > 0) {
                to_draw = merge_block(to_draw, rest);
            }
            return to_draw;
        } else {
            return node;
        }
    }

    pair<block, block> xcut_block(const block& node, int x) {
        // validation
        if (x < node.left || node.left + node.width <= x) {
            cerr << "cannot xcut: out of bounds: " << x << " not in (" << node.left << ", " << (node.left + node.width) << ")" << endl;
        }

        // output move
        out << "cut [" << node.id << "] [x] [" << x << "]" << endl;

        // add cost
        cost += round(CANVAS_SIZE / node.size() * 7);

        return G.xcut_block(node, x);
    }

    pair<block, block> ycut_block(const block& node, int y) {
        // validation
        if (y < node.bottom || node.bottom + node.height <= y) {
            cerr << "cannot ycut: out of bounds: " << y << " not in (" << node.bottom << ", " << (node.bottom + node.height) << ")" << endl;
        }

        // output move
        out << "cut [" << node.id << "] [y] [" << y << "]" << endl;

        // add cost
        cost += round(CANVAS_SIZE / node.size() * 7);

        return G.ycut_block(node, y);
    }

    block merge_block(const block &node1, const block &node2) {
        // validation
        if (node1.left == node2.left && node1.width == node2.width && (node1.bottom + node1.height == node2.bottom || node2.bottom + node2.height == node1.bottom)) {
            // merge horizontal
        } else if (node1.bottom == node2.bottom && node1.height == node2.height && (node1.left + node1.width == node2.left || node2.left + node2.width == node1.left)) {
            // merge vertical
        } else {
            cerr << "cannot merge" << endl;
        }

        // output move
        out << "merge [" << node1.id << "] [" << node2.id << "]" << endl;

        // add cost
        cost += round(CANVAS_SIZE / max(node1.size(), node2.size()));

        return G.merge_block(node1, node2);
    }

    void paint_block(const block& node, const color& c) {
        // output move
        out << "color [" << node.id << "] " << c << endl;

        // add cost
        cost += round(CANVAS_SIZE / node.size() * 5);

        G.paint_block(node, c);
    }

    double similarity_cost(int x1, int y1, int x2, int y2, color new_color) const {
        double diff = 0;
        for (int x = x1; x < x2; x++) {
            for (int y = y1; y < y2; y++) {
                double d = 0;
                d += (P.target[y][x][0] - new_color.r) * (P.target[y][x][0] - new_color.r);
                d += (P.target[y][x][1] - new_color.g) * (P.target[y][x][1] - new_color.g);
                d += (P.target[y][x][2] - new_color.b) * (P.target[y][x][2] - new_color.b);
                d += (P.target[y][x][3] - new_color.a) * (P.target[y][x][3] - new_color.a);
                diff += sqrt(d) * 0.005;
            }
        }
        return diff;
    }

    double similarity_cost(int x1, int y1, int x2, int y2) const {
        double diff = 0;
        for (int x = x1; x < x2; x++) {
            for (int y = y1; y < y2; y++) {
                color current = G.get_color(x, y);
                double d = 0;
                d += (P.target[y][x][0] - current.r) * (P.target[y][x][0] - current.r);
                d += (P.target[y][x][1] - current.g) * (P.target[y][x][1] - current.g);
                d += (P.target[y][x][2] - current.b) * (P.target[y][x][2] - current.b);
                d += (P.target[y][x][3] - current.a) * (P.target[y][x][3] - current.a);
                diff += sqrt(d) * 0.005;
            }
        }
        return diff;
    }

    int similarity_cost() const {
        return round(similarity_cost(0, 0, P.width, P.height));
    }

    color fast_get_color(const map<color, int>& mp) const {
        vector<pair<color, int>> v;
        for (auto it = mp.begin(); it != mp.end(); it++) v.emplace_back(it->first, it->second);
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

        return color(br, bg, bb, ba);
    }
    
    color fast_get_color(int x1, int y1, int x2, int y2) const {
        //fprintf(stderr, "(%d, %d) to (%d, %d)", x1, y1, x2, y2);
        map<color, int> mp;
        for (int i = y1; i < y2; i++) {
            for (int j = x1; j < x2; j++) {
                mp[color(P.target[i][j][0], P.target[i][j][1], P.target[i][j][2], P.target[i][j][3])]++;
            }
        }
        
        return fast_get_color(mp);
    }
};

setting auto_border(problem& P, int min_distance = 10, int cut_count = 10000) {
    vector<int> cut_x, cut_y;

    vector<pair<int, double>> diff_x;
    for (int x = 0; x + 1 < P.width; x++) {
        double diff = 0;
        for (int y = 0; y < P.height; y++) {
            double d = 0;
            for (int k = 0; k < 4; k++) {
                d += (P.target[y][x][k] - P.target[y][x+1][k]) * (P.target[y][x][k] - P.target[y][x+1][k]);
            }
            diff += sqrt(d);
        }
        diff_x.push_back(make_pair(x, diff));
    }
    sort(diff_x.begin(), diff_x.end(), [](auto const& lhs, auto const& rhs){
        return lhs.second > rhs.second;
    });

    set<int> xs;
    cut_x.push_back(0); cut_x.push_back(P.width);
    xs.insert(0); xs.insert(P.width);
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
    fprintf(stderr, "cut_x: ");
    for (auto t : cut_x) {
        fprintf(stderr, "%d ", t);
    }
    fprintf(stderr, "\n");

    vector<pair<int, double>> diff_y;
    for (int y = 0; y + 1 < P.height; y++) {
        double diff = 0;
        for (int x = 0; x < P.width; x++) {
            double d = 0;
            for (int k = 0; k < 4; k++) {
                d += (P.target[y][x][k] - P.target[y+1][x][k]) * (P.target[y][x][k] - P.target[y+1][x][k]);
            }
            diff += sqrt(d);
        }
        diff_y.push_back(make_pair(y, diff));
    }
    sort(diff_y.begin(), diff_y.end(), [](auto const& lhs, auto const& rhs){
        return lhs.second > rhs.second;
    });

    set<int> ys;
    cut_y.push_back(0); cut_y.push_back(P.width);
    ys.insert(0); ys.insert(P.width);
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

    return setting(cut_x, cut_y);
}

setting grid_setting(int w, int h, int size) {
    vector<int> cut_x, cut_y;
    for (int i = 0; i < w; i+= size) {
        cut_x.push_back(i);
    }
    cut_x.push_back(w);
    for (int i = 0; i < h; i+= size) {
        cut_y.push_back(i);
    }
    cut_y.push_back(h);
    return setting(cut_x, cut_y);
}

int main() {
    init_sq();

    problem P; cin >> P;
    
    setting setting = auto_border(P);
    solver S(P, setting, cout);

    S.solve();
    
    return 0;
}
