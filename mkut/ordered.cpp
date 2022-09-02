#include <cstdio>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>
#include <set>

using namespace std;

const int MAX_H = 400;
const int MAX_W = 400;

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
};

int p[MAX_H][MAX_W][4];
double sq[255 * 255 * 4 + 1];

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

color _get_color(map<color, int>& mp) {
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

    fprintf(stderr, "best:%d, (%d %d %d %d)\n", best / 200, br, bg, bb, ba);
    return color(br, bg, bb, ba);
}


color get_color(int x1, int y1, int x2, int y2) {
    fprintf(stderr, "(%d, %d) to (%d, %d)", x1, y1, x2, y2);
    map<color, int> mp;
    for (int i = y1; i < y2; i++) {
        for (int j = x1; j < x2; j++) {
            mp[color(p[i][j][0], p[i][j][1], p[i][j][2], p[i][j][3])]++;
        }
    }
    
    return _get_color(mp);
}

void cut_block(const string &node, char c, int val) {
    printf("cut [%s] [%c] [%d]\n", node.c_str(), c, val);
}

void xcut_block(const string &node, int x) {
    cut_block(node, 'x', x);
}

void ycut_block(const string &node, int y) {
    cut_block(node, 'y', y);
}

void pcut_block(const string &node, int x, int y) {
    printf("cut [%s] [%d, %d]\n", node.c_str(), x, y);
}

void merge_block(const string &node1, const string &node2) {
    printf("merge [%s] [%s]\n", node1.c_str(), node2.c_str());
}

void paint_block(const string &node, color c) {
    printf("color [%s] [%d, %d, %d, %d]\n", node.c_str(), c.r, c.g, c.b, c.a);
}

string paint_column(int left, int right, int top, int bottom, int height, int width, string node, int& id) {
    if (top + height >= bottom) return node;

    // cut top row
    ycut_block(node, top + height);

    // paint cell    
    auto c = get_color(left, top, left + width, top + height);
    paint_block(node + ".1", c);
    
    // merge top row
    merge_block(node + ".0", node + ".1");
    node = to_string(++id);

    return paint_column(left, right, top + height, bottom, height, width, node, id);
}

void _paint(int left, int bottom, string& node, int& id, color& color) {
    string target = node;
    // cut
    if (left == 0 && bottom == 0) {
        // no cut
    } else if (left == 0) {
        // horizontal cut
        ycut_block(node, bottom);
        target += ".1";
    } else if (bottom == 0) {
        // vertical cut
        xcut_block(node, left);
        target += ".1";
    } else {
        // point cut
        //pcut_block(node, left, bottom);
        //target += ".2";
        xcut_block(node, left);
        ycut_block(node + ".1", bottom);
        target += ".1.1";
    }
    
    // color
    paint_block(target, color);

    // merge
    if (left == 0 && bottom == 0) {
        // no merge
    } else if (left == 0 || bottom == 0) {
        // merge once
        merge_block(node + ".0", node + ".1");
        id += 1;
        node = to_string(id);
    } else {
        // merge thrice
        //merge_block(node + ".0", node + ".1");
        //merge_block(node + ".2", node + ".3");
        //merge_block(to_string(id+1), to_string(id+2));
        //id += 3;
        //node = to_string(id);
        merge_block(node + ".1.0", node + ".1.1");
        merge_block(to_string(id+1), node + ".0");
        id += 2;
        node = to_string(id);

    }


}

void paint(int left, int bottom, int width, int height, string& node, int& id) {
    auto c = get_color(left, bottom, left + width, bottom + height);
    _paint(left, bottom, node, id, c);
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

    int id = 0;
    const int size_x = 25;
    const int size_y = 25;
    string node = "0";

    const int limit = 50;

    vector<pair<pair<int,int>,int>> blocks;
    for (int y = 0; y < h; y += size_y) {
        for (int x = 0; x < w; x+= size_x) {
            blocks.push_back(make_pair(make_pair(x, y), (h - y) * (w - x)));
        }
    }
    sort(blocks.begin(), blocks.end(), [](const pair<pair<int,int>,int>& x, const pair<pair<int,int>,int>& y) {
        return x.second > y.second;
    });

    map<color, int> mp;
    set<pair<int,int>> to_draw;
    for (auto b : blocks) {
        int x = b.first.first;
        int y = b.first.second;
        int cost = b.second;
        if (w * h / cost >= limit) {
            for (int i = y; i < min(h, y+size_y); i++) {
                for (int j = x; j < min(w, x+size_x); j++) {
                    mp[color(p[i][j][0], p[i][j][1], p[i][j][2], p[i][j][3])]++;
                }
            }
            if (!to_draw.count(make_pair(x - size_x, y)) && !to_draw.count(make_pair(x, y - size_y))) {
                //to_draw.insert(make_pair(x, y));
            }
        } else {
            fprintf(stderr, "%d, %d: %d\n", x, y, cost);
            paint(x, y, size_x, size_y, node, id);
        }
    }
    auto rem_color = _get_color(mp);
    for (auto b : to_draw) {
        _paint(b.first, b.second, node, id, rem_color);
    }
    
    
    return 0;
}
