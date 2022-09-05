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
int cut_merge_cost[MAX_H][MAX_W + 1][2];
bool pcut_cheap[MAX_H][MAX_W][2];
double sq[255 * 255 * 4 + 1];
double* similarity_sum[MAX_W][MAX_H + 1];
vector<color> colors;
double dpx[MAX_H + 1];
double* dpy[2][MAX_H][MAX_H + 1];
int parentx[MAX_H + 1];
short parenty[2][MAX_H][MAX_H + 1][MAX_W + 1][2];
double worst_score[MAX_COLOR][MAX_W + 1];

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

double get_similarity(int x1, int x2, int y, int c) {
    return similarity_sum[y][x2][c] - similarity_sum[y][x1][c];
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
    for (int y = 0; y < w; y++) {
        for (int x = 0; x <= h; x++) {
            similarity_sum[y][x] = (double*)malloc(sizeof(double) * MAX_COLOR);
            for (int c = 0; c < colors.size(); c++) similarity_sum[y][x][c] = 0;
        }
    }
    
    for (int y = 0; y < w; y++) {
        for (int i = 0; i < 4; i++) current[0][y][i] = 255;
    }
    
    for (int x = 1; x <= h; x++) {
        for (int y = 1; y <= w; y++) {
            cut_cost[x][y] = round(2.0 * h * w / x / y);
            pcut_cost[x][y] = round(3.0 * h * w / x / y);
            color_cost[x][y] = round(5.0 * h * w / x / y);
            merge_cost[x][y] = round(1.0 * h * w / x / y);
        }
    }
    
    for (int x = 0; x < h; x++) {
        for (int y = 1; y < w; y++) {
            for (int i = 0; i < 2; i++) {
                int cost_cut = 0;
                if (i == 0 && x > 0) {
                    cost_cut += cut_cost[h][w];
                    cost_cut += merge_cost[max(x, h - x)][w];
                }
                cost_cut += cut_cost[h - x][w];
                cost_cut += merge_cost[h - x][max(y, w - y)];
                if (x == 0) {
                    pcut_cheap[x][y][i] = false;
                    cut_merge_cost[x][y][i] = cost_cut;
                    continue;
                }
                int cost_pcut = 0;
                cost_pcut += pcut_cost[h][w];
                cost_pcut += merge_cost[x][max(y, w - y)];
                cost_pcut += merge_cost[h - x][max(y, w - y)];
                cost_pcut += merge_cost[max(x, h - x)][w];
                if (cost_pcut < cost_cut) {
                    pcut_cheap[x][y][i] = true;
                    cut_merge_cost[x][y][i] = cost_pcut;
                } else {
                    pcut_cheap[x][y][i] = false;
                    cut_merge_cost[x][y][i] = cost_cut;
                }
            }
        }
    }
    
    for (int i = 0; i <= 255 * 255 * 4; i++) sq[i] = sqrt(i) * 0.005;
    
    for (int y = 0; y < w; y++) {
        for (int x = 0; x < h; x++) {
            for (int c = 0; c < colors.size(); c++) {
                similarity_sum[y][x + 1][c] = similarity_sum[y][x][c] + diff(x, y, colors[c]);
            }
        }
    }
}

double calc_y(int x1, int x2) {
    double sum1 = 0;
    for (int y = 1; y <= w; y++) {
        for (int x = x1; x < x2; x++) sum1 += diff_current(x1, x, y - 1);
        dpy[0][x1][x2][y] = sum1 + cut_merge_cost[x1][y][0];
        parenty[0][x1][x2][y][0] = -1;
    }
    
    for (int c = 0; c < colors.size(); c++) {
        worst_score[c][w] = dpy[0][x1][x2][w];
        for (int y = w - 1; y >= 0; y--) {
            worst_score[c][y] = worst_score[c][y + 1] - get_similarity(x1, x2, y, c);
            worst_score[c][y] = max(worst_score[c][y], dpy[0][x1][x2][y] - cut_merge_cost[x1][y][1]);
        }
    }
    
    dpy[0][x1][x2][0] = 0;
    if (x1 > 0) dpy[0][x1][x2][0] += cut_cost[h][w] + merge_cost[max(x1, h - x1)][w];
    parenty[0][x1][x2][0][0] = -1;
    for (int y = 0; y < w; y++) {
        for (int c = 0; c < colors.size(); c++) {
            double score = dpy[0][x1][x2][y] + color_cost[h - x1][w - y];
            for (int dy = 1; dy <= LIMIT_Y && y + dy <= w; dy++) {
                score += get_similarity(x1, x2, y + dy - 1, c);
                if (score >= worst_score[c][y + dy]) break;
                int cost = cut_merge_cost[x1][y + dy][1];
                if (score + cost < dpy[0][x1][x2][y + dy]) {
                    dpy[0][x1][x2][y + dy] = score + cost;
                    parenty[0][x1][x2][y + dy][0] = y;
                    parenty[0][x1][x2][y + dy][1] = c;
                }
            }
        }
    }
    
    double sum2 = 0;
    for (int y = w - 1; y >= 0; y--) {
        for (int x = x1; x < x2; x++) sum2 += diff_current(x1, x, y);
        dpy[1][x1][x2][y] = sum2 + cut_merge_cost[x1][y][0];
        parenty[1][x1][x2][y][0] = -1;
    }
    
    for (int c = 0; c < colors.size(); c++) {
        worst_score[c][0] = dpy[1][x1][x2][0];
        for (int y = 1; y <= w; y++) {
            worst_score[c][y] = worst_score[c][y - 1] - get_similarity(x1, x2, y - 1, c);
            worst_score[c][y] = max(worst_score[c][y], dpy[1][x1][x2][y] - cut_merge_cost[x1][y][1]);
        }
    }
    
    dpy[1][x1][x2][w] = 0;
    if (x1 > 0) dpy[1][x1][x2][w] += cut_cost[h][w] + merge_cost[max(x1, h - x1)][w];
    parenty[1][x1][x2][w][0] = -1;
    for (int y = w; y > 0; y--) {
        for (int c = 0; c < colors.size(); c++) {
            double score = dpy[1][x1][x2][y] + color_cost[h - x1][y];
            for (int dy = 1; dy <= LIMIT_Y && y - dy >= 0; dy++) {
                score += get_similarity(x1, x2, y - dy, c);
                if (score >= worst_score[c][y - dy]) break;
                int cost = cut_merge_cost[x1][y - dy][1];
                if (score + cost < dpy[1][x1][x2][y - dy]) {
                    dpy[1][x1][x2][y - dy] = score + cost;
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
                    for (int i = 0; i < 4; i++) current[x2][y][i] = current[x1][y][i];
                }
                break;
            } else {
                int c = parenty[0][x1][x2][y2][1];
                for (int y = y1; y < y2; y++) {
                    current[x2][y][0] = colors[c].r;
                    current[x2][y][1] = colors[c].g;
                    current[x2][y][2] = colors[c].b;
                    current[x2][y][3] = colors[c].a;
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
                    for (int i = 0; i < 4; i++) current[x2][y][i] = current[x1][y][i];
                }
                break;
            } else {
                int c = parenty[1][x1][x2][y1][1];
                for (int y = y1; y < y2; y++) {
                    current[x2][y][0] = colors[c].r;
                    current[x2][y][1] = colors[c].g;
                    current[x2][y][2] = colors[c].b;
                    current[x2][y][3] = colors[c].a;
                }
                y1 = y2;
            }
        }
    }
}

int output(int& id, int x1, int x2) {
    int cost = 0;
    if (parentx[x1] >= 0) cost += output(id, parentx[x1], x1);
    
    if (dpy[0][x1][x2][w] <= dpy[1][x1][x2][0]) {
        int y2 = w;
        vector<pair<int, int>> paints;
        while (true) {
            int y1 = parenty[0][x1][x2][y2][0];
            if (y1 == -1) break;
            int c = parenty[0][x1][x2][y2][1];
            paints.emplace_back(y1, c);
            y2 = y1;
        }
        if (paints.empty()) return cost;
        reverse(paints.begin(), paints.end());
        
        int cnt = 0;
        bool pcut = false;
        string left_id = to_string(id), right_id = left_id;
        if (x1 > 0) {
            int y = paints[0].first;
            if (y > 0 && pcut_cheap[x1][y][0]) {
                pcut = true;
                cost += pcut_cost[h][w];
                printf("cut [%d] [%d, %d]\n", id, x1, y);
                right_id += ".2";
                cost += merge_cost[x1][max(y, w - y)];
                printf("merge [%d.0] [%d.3]\n", id, id);
                cnt++;
                left_id = to_string(id + cnt);
            } else {
                cost += cut_cost[h][w];
                printf("cut [%d] [x] [%d]\n", id, x1);
                left_id += ".0";
                right_id += ".1";
            }
        }
        for (int i = 0; i < paints.size(); i++) {
            int y = paints[i].first;
            int c = paints[i].second;
            string target_id = right_id, cut_id = "";
            if (pcut) {
                pcut = false;
                cut_id = to_string(id) + ".1";
            } else if (y > 0) {
                if (x1 > 0 && pcut_cheap[x1][y][1]) {
                    cost += merge_cost[max(x1, h - x1)][w];
                    printf("merge [%s] [%s]\n", left_id.c_str(), right_id.c_str());
                    cnt++;
                    cost += pcut_cost[h][w];
                    printf("cut [%d] [%d, %d]\n", id + cnt, x1, y);
                    target_id = to_string(id + cnt) + ".2";
                    cut_id = to_string(id + cnt) + ".1";
                    cost += merge_cost[x1][max(y, w - y)];
                    printf("merge [%d.0] [%d.3]\n", id + cnt, id + cnt);
                    cnt++;
                    left_id = to_string(id + cnt);
                } else {
                    cost += cut_cost[h - x1][w];
                    printf("cut [%s] [y] [%d]\n", right_id.c_str(), y);
                    cut_id = target_id + ".0";
                    target_id += ".1";
                }
            }
            cost += color_cost[h - x1][w - y];
            printf("color [%s] [%d, %d, %d, %d]\n", target_id.c_str(), colors[c].r, colors[c].g, colors[c].b, colors[c].a);
            if (cut_id != "") {
                cost += merge_cost[h - x1][max(y, w - y)];
                printf("merge [%s] [%s]\n", target_id.c_str(), cut_id.c_str());
                cnt++;
                right_id = to_string(id + cnt);
            }
        }
        if (x1 > 0) {
            cost += merge_cost[max(x1, h - x1)][w];
            printf("merge [%s] [%s]\n", left_id.c_str(), right_id.c_str());
            cnt++;
        }
        id += cnt;
        return cost;
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
        if (paints.empty()) return cost;
        reverse(paints.begin(), paints.end());
        
        int cnt = 0;
        bool pcut = false;
        string left_id = to_string(id), right_id = left_id;
        if (x1 > 0) {
            int y = paints[0].first;
            if (y < w && pcut_cheap[x1][y][0]) {
                pcut = true;
                cost += pcut_cost[h][w];
                printf("cut [%d] [%d, %d]\n", id, x1, y);
                right_id += ".1";
                cost += merge_cost[x1][max(y, w - y)];
                printf("merge [%d.0] [%d.3]\n", id, id);
                cnt++;
                left_id = to_string(id + cnt);
            } else {
                cost += cut_cost[h][w];
                printf("cut [%d] [x] [%d]\n", id, x1);
                left_id += ".0";
                right_id += ".1";
            }
        }
        for (int i = 0; i < paints.size(); i++) {
            int y = paints[i].first;
            int c = paints[i].second;
            string target_id = right_id, cut_id = "";
            if (pcut) {
                pcut = false;
                cut_id = to_string(id) + ".2";
            } else if (y < w) {
                if (x1 > 0 && pcut_cheap[x1][y][1]) {
                    cost += merge_cost[max(x1, h - x1)][w];
                    printf("merge [%s] [%s]\n", left_id.c_str(), right_id.c_str());
                    cnt++;
                    cost += pcut_cost[h][w];
                    printf("cut [%d] [%d, %d]\n", id + cnt, x1, y);
                    target_id = to_string(id + cnt) + ".1";
                    cut_id = to_string(id + cnt) + ".2";
                    cost += merge_cost[x1][max(y, w - y)];
                    printf("merge [%d.0] [%d.3]\n", id + cnt, id + cnt);
                    cnt++;
                    left_id = to_string(id + cnt);
                } else {
                    cost += cut_cost[h - x1][w];
                    printf("cut [%s] [y] [%d]\n", right_id.c_str(), y);
                    cut_id = target_id + ".1";
                    target_id += ".0";
                }
            }
            cost += color_cost[h - x1][y];
            printf("color [%s] [%d, %d, %d, %d]\n", target_id.c_str(), colors[c].r, colors[c].g, colors[c].b, colors[c].a);
            if (cut_id != "") {
                cost += merge_cost[h - x1][max(y, w - y)];
                printf("merge [%s] [%s]\n", target_id.c_str(), cut_id.c_str());
                cnt++;
                right_id = to_string(id + cnt);
            }
        }
        if (x1 > 0) {
            cost += merge_cost[max(x1, h - x1)][w];
            printf("merge [%s] [%s]\n", left_id.c_str(), right_id.c_str());
            cnt++;
        }
        id += cnt;
        return cost;
    }
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
    
    for (int x = 1; x <= h; x++) dpx[x] = INF;
    parentx[0] = -1;
    for (int x = 0; x < h; x++) {
        for (int dx = 1; dx <= LIMIT_X && x + dx <= h; dx++) {
            double score = calc_y(x, x + dx);
            if (dpx[x] + score < dpx[x + dx]) {
                dpx[x + dx] = dpx[x] + score;
                parentx[x + dx] = x;
                update_current(x, x + dx);
            }
        }
    }
    
    int id = 0;
    int cost = output(id, parentx[h], h);
    fprintf(stderr, "score: %d (cost: %d)\n", (int)round(dpx[h]), cost);
    
    return 0;
}
