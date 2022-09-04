#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <random>
using namespace std;

unsigned long xor128() {
    static unsigned long x=123456789, y=362436069, z=521288629, w=88675123;
    unsigned long t=(x^(x<<11));
    x=y; y=z; z=w;
    return (w=(w^(w>>19))^(t^(t>>8)));
}

const int MAX_H = 400;
const int MAX_W = 400;

int target[MAX_H][MAX_W][4];


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

    friend istream& operator>>(istream& is, color& c) {
        return is >> c.r >> c.g >> c.b >> c.a;
    }

    friend ostream& operator<<(ostream& os, const color& c) {
        return os << "[" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << "]";
    }
};

vector<color> create_color_pallet(vector<color> colors) {
    const int cluster_num = 100;

    mt19937 engine(123456789);
    shuffle(colors.begin(), colors.end(), engine);
    int n = colors.size();
    vector<int> cluster(n);

    for (int i = 0; i < n; i++) {
        cluster[i] = i % cluster_num;
    }

    vector<int> prev = cluster;
    vector<color> rep;
    while (true) {
        vector<color> sum(cluster_num, color(0, 0, 0, 0));
        vector<int> cnt(cluster_num);
        for (int i = 0; i < n; i++) {
            int cid = cluster[i];
            sum[cid].r += colors[i].r;
            sum[cid].g += colors[i].g;
            sum[cid].b += colors[i].b;
            sum[cid].a += colors[i].a;
            cnt[cid]++;
        }

        rep = vector<color>(cluster_num);
        for (int i = 0; i < cluster_num; i++) {
            if (cnt[i] > 0) {
                rep[i] = color(round(1.0 * sum[i].r / cnt[i]), round(1.0 * sum[i].g / cnt[i]), round(1.0 * sum[i].b / cnt[i]), round(1.0 * sum[i].a / cnt[i]));
            }
        }

        for (int i = 0; i < n; i++) {
            int max_cid = 0;
            double max_d = 1e10;
            for (int cid = 0; cid < cluster_num; cid++) {
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

int main() {
    int w, h; cin >> w >> h;
    for (int k = 0; k < 4; k++) {
        for (int y = h - 1; y >= 0; y--) {
            for (int x = 0; x < w; x++) {
                cin >> target[y][x][k];
            }
        }
    }

    set<color> color_set;
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            color_set.insert(color(target[y][x][0], target[y][x][1], target[y][x][2], target[y][x][3]));
        }
    }
    vector<color> colors(color_set.begin(), color_set.end());

    vector<color> color_pallet = create_color_pallet(colors);

    cout << "P3" << endl;
    cout << 1 << " " << color_pallet.size() << endl;
    cout << 255 << endl;
    for (color c : color_pallet) {
        // cout << c << endl;
        cout << c.r << " " << c.g << " " << c.b << endl;
    }
}
