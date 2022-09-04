#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
#include <map>

using namespace std;

const int MAX_H = 400;
const int MAX_W = 400;
const int MAX_AUTO_COLOR = 30;
const int MAX_EXTRACT_COLOR = 20;
const int MAX_COLOR = MAX_AUTO_COLOR + MAX_EXTRACT_COLOR;
const int LIMIT_X = 100;
const int LIMIT_Y = 400;
const double INF = 1e9;

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
    
    bool operator==(const color& c) const {
        return r == c.r && g == c.g && b == c.b && a == c.a;
    }
};

int h, w;
int p[MAX_H][MAX_W][4];
int current[MAX_H + 1][MAX_W][4];
int cut_cost[MAX_H + 1][MAX_W + 1];
int pcut_cost[MAX_H + 1][MAX_W + 1];
int color_cost[MAX_H + 1][MAX_W + 1];
int merge_cost[MAX_H + 1][MAX_W + 1];
double sq[255 * 255 * 4 + 1];
double* similarity_sum[MAX_H + 1][MAX_W + 1];
vector<color> colors;
double dpx[MAX_H + 1];
double* dpy[2][MAX_H][MAX_H + 1];
int parentx[MAX_H + 1];
short parenty[2][MAX_H][MAX_H + 1][MAX_W + 1][2];

unsigned long xor128() {
    static unsigned long x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    unsigned long t = (x ^ (x << 11));
    x = y; y = z; z = w;
    return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
}

double diff(int x, int y, const color& c) {
    int dr = p[x][y][0] - c.r;
    int dg = p[x][y][1] - c.g;
    int db = p[x][y][2] - c.b;
    int da = p[x][y][3] - c.a;
    return sq[dr * dr + dg * dg + db * db + da * da];
}

double diff_current(int x1, int x, int y) {
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        int d = p[x][y][i] - current[x1][y][i];
        sum += d * d;
    }
    return sq[sum];
}

double get_similarity(int x1, int y1, int x2, int y2, int c) {
    return similarity_sum[x2][y2][c] - similarity_sum[x2][y1][c] - similarity_sum[x1][y2][c] + similarity_sum[x1][y1][c];
}

void input() {
    scanf("%d %d", &w, &h);
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                scanf("%d", &p[j][h - i - 1][k]);
            }
        }
    }
}

vector<color> create_color_pallet(int max_color) {
    set<color> color_set;
    for (int x = 0; x < h; x++) {
        for (int y = 0; y < w; y++) {
            color_set.emplace(p[x][y][0], p[x][y][1], p[x][y][2], p[x][y][3]);
        }
    }
    vector<color> colors(color_set.begin(), color_set.end());
    
    mt19937 engine(123456789);
    shuffle(colors.begin(), colors.end(), engine);
    int n = colors.size();
    vector<int> cluster(n);
    
    for (int i = 0; i < n; i++) {
        cluster[i] = i % max_color;
    }
    
    vector<int> prev = cluster;
    vector<color> rep;
    while (true) {
        vector<color> sum(max_color, color(0, 0, 0, 0));
        vector<int> cnt(max_color);
        for (int i = 0; i < n; i++) {
            int cid = cluster[i];
            sum[cid].r += colors[i].r;
            sum[cid].g += colors[i].g;
            sum[cid].b += colors[i].b;
            sum[cid].a += colors[i].a;
            cnt[cid]++;
        }
        
        rep = vector<color>(max_color);
        for (int i = 0; i < max_color; i++) {
            if (cnt[i] > 0) {
                rep[i] = color(round(1.0 * sum[i].r / cnt[i]), round(1.0 * sum[i].g / cnt[i]), round(1.0 * sum[i].b / cnt[i]), round(1.0 * sum[i].a / cnt[i]));
            }
        }
        
        for (int i = 0; i < n; i++) {
            int max_cid = 0;
            double max_d = 1e10;
            for (int cid = 0; cid < max_color; cid++) {
                double d = sqrt((rep[cid].r - colors[i].r) * (rep[cid].r - colors[i].r)
                + (rep[cid].g - colors[i].g) * (rep[cid].g - colors[i].g)
                + (rep[cid].b - colors[i].b) * (rep[cid].b - colors[i].b)
                + (rep[cid].a - colors[i].a) * (rep[cid].a - colors[i].a));
                if (max_d > d) {
                    max_d = d;
                    max_cid = cid;
                }
            }
            cluster[i] = max_cid;
        }
        
        if (prev == cluster) break;
        prev = cluster;
    }
    
    vector<color> ret;
    for (int i = 0; i < rep.size(); i++) {
        if (rep[i].r >= 0) {
            ret.push_back(rep[i]);
        }
    }
    
    return ret;
}

void preprocess() {
    for (int i = 0; i < 2; i++) {
        for (int x1 = 0; x1 < h; x1++) {
            for (int x2 = x1 + 1; x2 <= h; x2++) {
                dpy[i][x1][x2] = (double*)malloc(sizeof(double) * (MAX_W + 1));
            }
        }
    }
    for (int x = 0; x <= h; x++) {
        for (int y = 0; y <= w; y++) {
            similarity_sum[x][y] = (double*)malloc(sizeof(double) * MAX_COLOR);
            for (int c = 0; c < colors.size(); c++) similarity_sum[x][y][c] = 0;
        }
    }
    
    for (int y = 0; y < w; y++) {
        for (int i = 0; i < 4; i++) current[w][y][i] = 255;
    }
    
    for (int x = 1; x <= h; x++) {
        for (int y = 1; y <= w; y++) {
            cut_cost[x][y] = round(7.0 * h * w / x / y);
            pcut_cost[x][y] = round(10.0 * h * w / x / y);
            color_cost[x][y] = round(5.0 * h * w / x / y);
            merge_cost[x][y] = round(1.0 * h * w / x / y);
        }
    }
    
    for (int i = 0; i <= 255 * 255 * 4; i++) sq[i] = sqrt(i) * 0.005;
    
    for (int x = 0; x < h; x++) {
        for (int y = 0; y < w; y++) {
            for (int c = 0; c < colors.size(); c++) {
                similarity_sum[x + 1][y + 1][c] += similarity_sum[x + 1][y][c];
                similarity_sum[x + 1][y + 1][c] += similarity_sum[x][y + 1][c];
                similarity_sum[x + 1][y + 1][c] -= similarity_sum[x][y][c];
                similarity_sum[x + 1][y + 1][c] += diff(x, y, colors[c]);
            }
        }
    }
}

double calc_y(int x1, int x2) {
    double sum1 = 0;
    for (int y = 1; y <= w; y++) {
        for (int x = x1; x < x2; x++) sum1 += diff_current(x2, x, y - 1);
        dpy[0][x1][x2][y] = sum1;
        if (x2 < w && y < h) {
            int cost1 = cut_cost[h][w] + merge_cost[max(x2, h - x2)][w] + cut_cost[x2][w] + merge_cost[x2][max(y, w - y)];
            dpy[0][x1][x2][y] += cost1;
        } else if (y < h) {
            dpy[0][x1][x2][y] += cut_cost[h][w] + merge_cost[h][max(y, w - y)];
        }
        parenty[0][x1][x2][y][0] = -1;
    }
    
    dpy[0][x1][x2][0] = 0;
    if (x2 < w) dpy[0][x1][x2][0] += cut_cost[h][w] + merge_cost[max(x2, h - x2)][w];
    parenty[0][x1][x2][0][0] = -1;
    for (int y = 0; y < w; y++) {
        for (int dy = 1; dy <= LIMIT_Y && y + dy <= w; dy++) {
            int cost = color_cost[x2][w - y];
            if (y + dy < w) {
                cost += cut_cost[x2][w];
                cost += merge_cost[x2][max(y + dy, w - y - dy)];
            }
            for (int c = 0; c < colors.size(); c++) {
                double similarity = get_similarity(x1, y, x2, y + dy, c);
                if (dpy[0][x1][x2][y] + cost + similarity < dpy[0][x1][x2][y + dy]) {
                    dpy[0][x1][x2][y + dy] = dpy[0][x1][x2][y] + cost + similarity;
                    parenty[0][x1][x2][y + dy][0] = y;
                    parenty[0][x1][x2][y + dy][1] = c;
                }
            }
        }
    }

    double sum2 = 0;
    for (int y = w - 1; y >= 0; y--) {
        for (int x = x1; x < x2; x++) sum2 += diff_current(x2, x, y);
        dpy[1][x1][x2][y] = sum2;
        if (x2 < w && y > 0) {
            int cost1 = cut_cost[h][w] + merge_cost[max(x2, h - x2)][w] + cut_cost[x2][w] + merge_cost[x2][max(y, w - y)];
            dpy[1][x1][x2][y] += cost1;
        } else if (y > 0) {
            dpy[1][x1][x2][y] += cut_cost[h][w] + merge_cost[h][max(y, w - y)];
        }
        parenty[1][x1][x2][y][0] = -1;
    }
    
    dpy[1][x1][x2][w] = 0;
    if (x2 < w) dpy[1][x1][x2][w] += cut_cost[h][w] + merge_cost[max(x2, h - x2)][w];
    parenty[1][x1][x2][w][0] = -1;
    for (int y = w; y > 0; y--) {
        for (int dy = 1; dy <= LIMIT_Y && y - dy >= 0; dy++) {
            int cost = color_cost[x2][y];
            if (y - dy > 0) {
                cost += cut_cost[x2][w];
                cost += merge_cost[x2][max(y - dy, w - y + dy)];
            }
            for (int c = 0; c < colors.size(); c++) {
                double similarity = get_similarity(x1, y - dy, x2, y, c);
                if (dpy[1][x1][x2][y] + cost + similarity < dpy[1][x1][x2][y - dy]) {
                    dpy[1][x1][x2][y - dy] = dpy[1][x1][x2][y] + cost + similarity;
                    parenty[1][x1][x2][y - dy][0] = y;
                    parenty[1][x1][x2][y - dy][1] = c;
                }
            }
        }
    }
    
    return min(dpy[0][x1][x2][w], dpy[1][x1][x2][0]);
}

void update_current(int x1, int x2) {
    if (dpy[0][x1][x2][w] <= dpy[1][x1][x2][0]) {
        int y2 = w;
        while (true) {
            int y1 = parenty[0][x1][x2][y2][0];
            if (y1 == -1) {
                for (int y = 0; y < y2; y++) {
                    for (int i = 0; i < 4; i++) current[x1][y][i] = current[x2][y][i];
                }
                break;
            } else {
                int c = parenty[0][x1][x2][y2][1];
                for (int y = y1; y < y2; y++) {
                    current[x1][y][0] = colors[c].r;
                    current[x1][y][1] = colors[c].g;
                    current[x1][y][2] = colors[c].b;
                    current[x1][y][3] = colors[c].a;
                }
                y2 = y1;
            }
        }
    } else {
        int y1 = 0;
        while (true) {
            int y2 = parenty[1][x1][x2][y1][0];
            if (y2 == -1) {
                for (int y = y1; y < w; y++) {
                    for (int i = 0; i < 4; i++) current[x1][y][i] = current[x2][y][i];
                }
                break;
            } else {
                int c = parenty[1][x1][x2][y1][1];
                for (int y = y1; y < y2; y++) {
                    current[x1][y][0] = colors[c].r;
                    current[x1][y][1] = colors[c].g;
                    current[x1][y][2] = colors[c].b;
                    current[x1][y][3] = colors[c].a;
                }
                y1 = y2;
            }
        }
    }
}

int output(const int id, const int x1, const int x2) {
    int cost = 0;
    int cnt = 0;
    
    if (dpy[0][x1][x2][w] <= dpy[1][x1][x2][0]) {
        int y2 = h;
        vector<pair<int, int>> paints;
        while (true) {
            int y1 = parenty[0][x1][x2][y2][0];
            if (y1 == -1) break;
            int c = parenty[0][x1][x2][y2][1];
            paints.emplace_back(y1, c);
            y2 = y1;
        }
        if (paints.empty()) goto endif;
        reverse(paints.begin(), paints.end());
        
        string start_id = to_string(id), end_id = start_id;
        if (x2 < w) { // horizontal line between previous band
            cost += cut_cost[h][w];
            printf("cut [%d] [x] [%d]\n", id, x2);
            start_id += ".0";
            end_id = start_id;
        }
        for (int i = 0; i < paints.size(); i++) {
            int y = paints[i].first;
            int c = paints[i].second;
            string target_id = end_id;
            if (y > 0) {
                cost += cut_cost[x2][h];
                printf("cut [%s] [y] [%d]\n", end_id.c_str(), y);
                target_id += ".1";
            }
            cost += color_cost[x2][w - y];
            printf("color [%s] [%d, %d, %d, %d]\n", target_id.c_str(), colors[c].r, colors[c].g, colors[c].b, colors[c].a);
            if (y > 0) {
                cost += merge_cost[x2][max(y, w - y)];
                printf("merge [%s.0] [%s.1]\n", end_id.c_str(), end_id.c_str());
                cnt++;
                end_id = to_string(id + cnt);
            }
        }
        if (x2 < w) {
            cost += merge_cost[max(x2, w - x2)][h];
            printf("merge [%d.1] [%s]\n", id, end_id.c_str());
            cnt++;
        }
    } else {
        int y1 = 0;
        vector<pair<int, int>> paints;
        while (true) {
            int y2 = parenty[1][x1][x2][y1][0];
            if (y2 == -1) break;
            int c = parenty[1][x1][x2][y1][1];
            paints.emplace_back(y2, c);
            y1 = y2;
        }
        if (paints.empty()) goto endif;
        reverse(paints.begin(), paints.end());
        
        string start_id = to_string(id), end_id = start_id;
        if (x2 < w) {
            cost += cut_cost[h][w];
            printf("cut [%d] [x] [%d]\n", id, x2);
            start_id += ".0";
            end_id = start_id;
        }
        for (int i = 0; i < paints.size(); i++) {
            int y = paints[i].first;
            int c = paints[i].second;
            string target_id = end_id;
            if (y < w) {
                cost += cut_cost[x2][w];
                printf("cut [%s] [y] [%d]\n", end_id.c_str(), y);
                target_id += ".0";
            }
            cost += color_cost[x2][y];
            printf("color [%s] [%d, %d, %d, %d]\n", target_id.c_str(), colors[c].r, colors[c].g, colors[c].b, colors[c].a);
            if (y < w) {
                cost += merge_cost[x2][max(y, w - y)];
                printf("merge [%s.0] [%s.1]\n", end_id.c_str(), end_id.c_str());
                cnt++;
                end_id = to_string(id + cnt);
            }
        }
        if (x2 < w) {
            cost += merge_cost[max(x2, w - x2)][h];
            printf("merge [%d.1] [%s]\n", id, end_id.c_str());
            cnt++;
        }
    }
endif:
    if (parentx[x1] >= 0) {
        cost += output(id + cnt, parentx[x1], x1);
    }
    return cost;
}

color get_color(int x, int y) {
    return color(p[y][x][0], p[y][x][1], p[y][x][2], p[y][x][3]);
}

vector<color> additional_color(const vector<color>& pallet, int to_add) {
    set<color> current_pallet(pallet.begin(), pallet.end());

    map<color, int> color_map;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            color_map[get_color(x, y)]++;
        }
    }

    vector<pair<int, color>> v;
    for (auto &e: color_map) {
        v.emplace_back(e.second, e.first);
    }

    sort(v.rbegin(), v.rend());

    vector<color> additional_color;
    int rem = to_add;
    for (auto &e : v) {
        color &c = e.second;
        if (current_pallet.find(c) != current_pallet.end()) continue;
        if (e.first < 100) break;
        
        if (rem-- > 0) {
            additional_color.push_back(c);
        }
    }

    return additional_color;
}

int main() {
    input();
    
    colors = create_color_pallet(MAX_AUTO_COLOR);

    for (auto c : additional_color(colors, MAX_EXTRACT_COLOR)) {
        colors.push_back(c);
    }
    
    preprocess();
    
    for (int x = 0; x < h; x++) dpx[x] = INF;

    vector<int> rev_parentx(w + 1, w);

    for (int frx = w; frx > 0; frx--) {
        for (int dx = 1; dx <= LIMIT_X && frx - dx >= 0; dx++) {
            const int tox = frx - dx;
            const double score = calc_y(tox, frx);
            if (dpx[frx] + score < dpx[tox]) {
                dpx[tox] = dpx[frx] + score;
                rev_parentx[tox] = frx;
                update_current(tox, frx); // current[tox][y][c]: color channel c's value of color at y when dp is from tox
            }
        }
    }

    int x = 0;
    while (x < w) {
        const int next = rev_parentx[x];
        parentx[next] = x;
        x = next;
    }
    parentx[0] = -1;
    
    int id = 0;
    int cost = output(id, parentx[w], w);
    fprintf(stderr, "score: %d (cost: %d)\n", (int)round(dpx[0]), cost);
    
    return 0;
}
