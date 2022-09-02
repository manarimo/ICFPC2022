#include <cstdio>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>

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
    for (int i = 0; i < v.size(); i++) {
        int dr = abs(r - v[i].first.r);
        int dg = abs(g - v[i].first.g);
        int db = abs(b - v[i].first.b);
        int da = abs(a - v[i].first.a);
        sum += sq[dr * dr + dg * dg + db * db + da * da] * v[i].second;
        if (sum >= best) return sum;
    }
    return sum;
}

int main() {
    int w, h;
    scanf("%d %d", &w, &h);
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                scanf("%d", &p[i][j][k]);
            }
        }
    }
    
    int min_r = 255, max_r = 0, min_g = 255, max_g = 0, min_b = 255, max_b = 0, min_a = 255, max_a = 0;
    map<color, int> mp;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
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
    fprintf(stderr, "size:%d, r[%d, %d], g[%d, %d], b[%d, %d], a[%d, %d]\n", v.size(), min_r, max_r, min_g, max_g, min_b, max_b, min_a, max_a);
    
    for (int i = 0; i <= 255 * 255 * 4; i++) sq[i] = sqrt(i);
    
    int best = 1e9, br = 0, bg = 0, bb = 0, ba = 0;
    for (int r = min_r; r <= max_r; r++) {
        fprintf(stderr, "now:%d, best:%d, (%d %d %d %d)\n", r, best / 200, br, bg, bb, ba);
        fflush(stderr);
        for (int g = min_g; g <= max_g; g++) {
            for (int b = min_b; b <= max_b; b++) {
                for (int a = min_a; a <= max_a; a++) {
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
    
    printf("color [0] [%d, %d, %d, %d]\n", br, bg, bb, ba);
    
    return 0;
}
