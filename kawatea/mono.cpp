#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

const int MAX_H = 400;
const int MAX_W = 400;

int p[MAX_H][MAX_W][4];
bool white[MAX_H][MAX_W];

int main() {
    int w, h;
    scanf("%d %d", &w, &h);
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                scanf("%d", &p[h - i - 1][j][k]);
            }
        }
    }
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            white[i][j] = (p[i][j][0] == 255);
        }
    }
    
    int id = 0;
    for (int i = 1; i < h; i++) {
        bool update = false;
        int start = -1, last = -1;
        vector<int> v;
        for (int j = 0; j < w; j++) {
            if (!white[i - 1][j]) {
                if (start == -1) start = j;
                last = j;
            }
            if (white[i][j] != white[i - 1][j]) update = true;
            if (j > 0 && white[i][j] != white[i][j - 1]) v.push_back(j);
        }
        if (!update) continue;
        printf("cut [%d] [y] [%d]\n", id, i);
        if (v.size() == 0) {
            printf("color [%d.1] [255, 255, 255, 255]\n", id);
            printf("merge [%d.0] [%d.1]\n", id, id);
        } else {
            int left = 0, right = 0;
            for (int j = 0; j < v.size(); j++) {
                left += round(5.0 * h * w / (h - i) / (w - v[j]));
                right += round(5.0 * h * w / (h - i) / v[j]);
            }
            if (left <= right) {
                if (start < v[0]) printf("color [%d.1] [255, 255, 255, 255]\n", id);
                for (int j = 0; j < v.size(); j++) {
                    if (j == 0) {
                        printf("cut [%d.1] [x] [%d]\n", id, v[j]);
                        printf("color [%d.1.1] [0, 0, 0, 255]\n", id);
                        printf("merge [%d.1.0] [%d.1.1]\n", id, id);
                    } else {
                        printf("cut [%d] [x] [%d]\n", id + j, v[j]);
                        if (j % 2 == 0) {
                            printf("color [%d.1] [0, 0, 0, 255]\n", id + j);
                        } else {
                            printf("color [%d.1] [255, 255, 255, 255]\n", id + j);
                        }
                        printf("merge [%d.0] [%d.1]\n", id + j, id + j);
                    }
                }
            } else {
                reverse(v.begin(), v.end());
                if (last > v[0]) printf("color [%d.1] [255, 255, 255, 255]\n", id);
                for (int j = 0; j < v.size(); j++) {
                    if (j == 0) {
                        printf("cut [%d.1] [x] [%d]\n", id, v[j]);
                        printf("color [%d.1.0] [0, 0, 0, 255]\n", id);
                        printf("merge [%d.1.0] [%d.1.1]\n", id, id);
                    } else {
                        printf("cut [%d] [x] [%d]\n", id + j, v[j]);
                        if (j % 2 == 0) {
                            printf("color [%d.0] [0, 0, 0, 255]\n", id + j);
                        } else {
                            printf("color [%d.0] [255, 255, 255, 255]\n", id + j);
                        }
                        printf("merge [%d.0] [%d.1]\n", id + j, id + j);
                    }
                }
            }
            printf("merge [%d] [%d.0]\n", id + v.size(), id);
        }
        id += v.size() + 1;
    }
    
    return 0;
}
