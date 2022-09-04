#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const int MAX_H = 400;
const int MAX_W = 400;
const int MAX_COLOR = 100;
const int LIMIT_X = 200;
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
    int n;
    scanf("%d", &n);
    for (int i = 0; i < n; i++) scanf("%*d %*d %*d %*d %*d %*d %*d %*d %*d");
    int c;
    scanf("%d", &c);
    for (int i = 0; i < c; i++) {
        int r, g, b, a;
        scanf("%d %d %d %d", &r, &g, &b, &a);
        colors.emplace_back(r, g, b, a);
    }
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
        for (int i = 0; i < 4; i++) current[0][y][i] = 255;
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
        for (int x = x1; x < x2; x++) sum1 += diff_current(x1, x, y - 1);
        dpy[0][x1][x2][y] = sum1;
        if (x1 > 0 && y < w) {
            int cost1 = cut_cost[h][w] + merge_cost[max(x1, h - x1)][w] + cut_cost[h - x1][w] + merge_cost[h - x1][max(y, w - y)];
            int cost2 = pcut_cost[h][w] + merge_cost[max(x1, h - x1)][w] + merge_cost[x1][max(y, w - y)] + merge_cost[h - x1][max(y, w - y)];
            dpy[0][x1][x2][y] += min(cost1, cost2);
        } else if (y < w) {
            dpy[0][x1][x2][y] += cut_cost[h][w] + merge_cost[h][max(y, w - y)];
        }
        parenty[0][x1][x2][y][0] = -1;
    }
    
    dpy[0][x1][x2][0] = 0;
    if (x1 > 0) dpy[0][x1][x2][0] += cut_cost[h][w] + merge_cost[max(x1, h - x1)][w];
    parenty[0][x1][x2][0][0] = -1;
    for (int y = 0; y < w; y++) {
        for (int dy = 1; dy <= LIMIT_Y && y + dy <= w; dy++) {
            int cost = color_cost[h - x1][w - y];
            if (y + dy < w) {
                cost += cut_cost[h - x1][w];
                cost += merge_cost[h - x1][max(y + dy, w - y - dy)];
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
        for (int x = x1; x < x2; x++) sum2 += diff_current(x1, x, y);
        dpy[1][x1][x2][y] = sum2;
        if (x1 > 0 && y > 0) {
            int cost1 = cut_cost[h][w] + merge_cost[max(x1, h - x1)][w] + cut_cost[h - x1][w] + merge_cost[h - x1][max(y, w - y)];
            int cost2 = pcut_cost[h][w] + merge_cost[max(x1, h - x1)][w] + merge_cost[x1][max(y, w - y)] + merge_cost[h - x1][max(y, w - y)];
            dpy[1][x1][x2][y] += min(cost1, cost2);
        } else if (y > 0) {
            dpy[1][x1][x2][y] += cut_cost[h][w] + merge_cost[h][max(y, w - y)];
        }
        parenty[1][x1][x2][y][0] = -1;
    }
    
    dpy[1][x1][x2][w] = 0;
    if (x1 > 0) dpy[1][x1][x2][w] += cut_cost[h][w] + merge_cost[max(x1, h - x1)][w];
    parenty[1][x1][x2][w][0] = -1;
    for (int y = w; y > 0; y--) {
        for (int dy = 1; dy <= LIMIT_Y && y - dy >= 0; dy++) {
            int cost = color_cost[h - x1][y];
            if (y - dy > 0) {
                cost += cut_cost[h - x1][w];
                cost += merge_cost[h - x1][max(y - dy, w - y + dy)];
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
        string start_id = to_string(id), end_id = start_id;
        if (x1 > 0) {
            int y = paints[0].first;
            if (y > 0) {
                int cost1 = cut_cost[h][w] + merge_cost[max(x1, h - x1)][w] + cut_cost[h - x1][w] + merge_cost[h - x1][max(y, w - y)];
                int cost2 = pcut_cost[h][w] + merge_cost[max(x1, h - x1)][w] + merge_cost[x1][max(y, w - y)] + merge_cost[h - x1][max(y, w - y)];
                if (cost2 < cost1) {
                    pcut = true;
                    cost += pcut_cost[h][w];
                    printf("cut [%d] [%d, %d]\n", id, x1, y);
                    start_id += ".2";
                    end_id = start_id;
                }
            }
            if (!pcut) {
                cost += cut_cost[h][w];
                printf("cut [%d] [x] [%d]\n", id, x1);
                start_id += ".1";
                end_id = start_id;
            }
        }
        for (int i = 0; i < paints.size(); i++) {
            int y = paints[i].first;
            int c = paints[i].second;
            string target_id = end_id;
            if (!(i == 0 && pcut) && y > 0) {
                cost += cut_cost[h - x1][w];
                printf("cut [%s] [y] [%d]\n", end_id.c_str(), y);
                target_id += ".1";
            }
            cost += color_cost[h - x1][w - y];
            printf("color [%s] [%d, %d, %d, %d]\n", target_id.c_str(), colors[c].r, colors[c].g, colors[c].b, colors[c].a);
            if (i == 0 && pcut) {
                cost += merge_cost[h - x1][max(y, w - y)];
                printf("merge [%d.1] [%d.2]\n", id, id);
                cnt++;
                end_id = to_string(id + cnt);
            } else if (y > 0) {
                cost += merge_cost[h - x1][max(y, w - y)];
                printf("merge [%s.0] [%s.1]\n", end_id.c_str(), end_id.c_str());
                cnt++;
                end_id = to_string(id + cnt);
            }
        }
        if (x1 > 0) {
            if (pcut) {
                int y = paints[0].first;
                cost += merge_cost[x1][max(y, w - y)];
                printf("merge [%d.0] [%d.3]\n", id, id);
                cnt++;
                cost += merge_cost[max(x1, h - x1)][w];
                printf("merge [%d] [%s]\n", id + cnt, end_id.c_str());
                cnt++;
            } else {
                cost += merge_cost[max(x1, h - x1)][w];
                printf("merge [%d.0] [%s]\n", id, end_id.c_str());
                cnt++;
            }
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
        string start_id = to_string(id), end_id = start_id;
        if (x1 > 0) {
            int y = paints[0].first;
            if (y < w) {
                int cost1 = cut_cost[h][w] + merge_cost[max(x1, h - x1)][w] + cut_cost[h - x1][w] + merge_cost[h - x1][max(y, w - y)];
                int cost2 = pcut_cost[h][w] + merge_cost[max(x1, h - x1)][w] + merge_cost[x1][max(y, w - y)] + merge_cost[h - x1][max(y, w - y)];
                if (cost2 < cost1) {
                    pcut = true;
                    cost += pcut_cost[h][w];
                    printf("cut [%d] [%d, %d]\n", id, x1, y);
                    start_id += ".1";
                    end_id = start_id;
                }
            }
            if (!pcut) {
                cost += cut_cost[h][w];
                printf("cut [%d] [x] [%d]\n", id, x1);
                start_id += ".1";
                end_id = start_id;
            }
        }
        for (int i = 0; i < paints.size(); i++) {
            int y = paints[i].first;
            int c = paints[i].second;
            string target_id = end_id;
            if (!(i == 0 && pcut) && y < w) {
                cost += cut_cost[h - x1][w];
                printf("cut [%s] [y] [%d]\n", end_id.c_str(), y);
                target_id += ".0";
            }
            cost += color_cost[h - x1][y];
            printf("color [%s] [%d, %d, %d, %d]\n", target_id.c_str(), colors[c].r, colors[c].g, colors[c].b, colors[c].a);
            if (i == 0 && pcut) {
                cost += merge_cost[h - x1][max(y, w - y)];
                printf("merge [%d.1] [%d.2]\n", id, id);
                cnt++;
                end_id = to_string(id + cnt);
            } else if (y < w) {
                cost += merge_cost[h - x1][max(y, w - y)];
                printf("merge [%s.0] [%s.1]\n", end_id.c_str(), end_id.c_str());
                cnt++;
                end_id = to_string(id + cnt);
            }
        }
        if (x1 > 0) {
            if (pcut) {
                int y = paints[0].first;
                cost += merge_cost[x1][max(y, w - y)];
                printf("merge [%d.0] [%d.3]\n", id, id);
                cnt++;
                cost += merge_cost[max(x1, h - x1)][w];
                printf("merge [%d] [%s]\n", id + cnt, end_id.c_str());
                cnt++;
            } else {
                cost += merge_cost[max(x1, h - x1)][w];
                printf("merge [%d.0] [%s]\n", id, end_id.c_str());
                cnt++;
            }
        }
        id += cnt;
        return cost;
    }
}

double remain(int x1) {
    double sum = 0;
    for (int x = x1; x < h; x++) {
        for (int y = 0; y < w; y++) {
            sum += diff_current(x1, x, y);
        }
    }
    return sum;
}

int main() {
    input();
    
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
