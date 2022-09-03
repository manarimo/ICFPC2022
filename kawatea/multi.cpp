#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

const int MAX_H = 400;
const int MAX_W = 400;

int p[MAX_H][MAX_W][4];

bool same(int x1, int y1, int x2, int y2) {
    for (int i = 0; i < 4; i++) {
        if (p[x1][y1][i] != p[x2][y2][i]) return false;
    }
    return true;
}

bool white(int x, int y) {
    for (int i = 0; i < 4; i++) {
        if (p[x][y][i] != 255) return false;
    }
    return true;
}

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
    
    int id = 0;
    for (int i = 0; i < h; i++) {
        bool update = false;
        vector<int> v;
        for (int j = 0; j < w; j++) {
            if (i == 0 || !same(i, j, i - 1, j)) update = true;
            if (j > 0 && !same(i, j, i, j - 1)) v.push_back(j);
        }
        if (!update) continue;
        
        if (i == 0) {
            if (v.size() == 0) {
                if (!white(0, 0)) printf("color [%d] [%d, %d, %d, %d]\n", id, p[0][0][0], p[0][0][1], p[0][0][2], p[0][0][3]);
            } else {
                int left = 0, right = 0, dir = 1;
                for (int j = 0; j < v.size(); j++) {
                    left += round(5.0 * h * w / (h - i) / (w - v[j]));
                    right += round(5.0 * h * w / (h - i) / v[j]);
                }
                if (left <= right) {
                    if (!white(0, 0)) printf("color [%d] [%d, %d, %d, %d]\n", id, p[0][0][0], p[0][0][1], p[0][0][2], p[0][0][3]);
                } else {
                    dir = 0;
                    reverse(v.begin(), v.end());
                    if (!white(0, w - 1)) printf("color [%d] [%d, %d, %d, %d]\n", id, p[0][w - 1][0], p[0][w - 1][1], p[0][w - 1][2], p[0][w - 1][3]);
                }
                for (int j = 0; j < v.size(); j++) {
                    printf("cut [%d] [x] [%d]\n", id + j, v[j]);
                    printf("color [%d.%d] [%d, %d, %d, %d]\n", id + j, dir, p[i][v[j] + dir - 1][0], p[i][v[j] + dir - 1][1], p[i][v[j] + dir - 1][2], p[i][v[j] + dir - 1][3]);
                    printf("merge [%d.0] [%d.1]\n", id + j, id + j);
                }
            }
            id += v.size();
        } else {
            printf("cut [%d] [y] [%d]\n", id, i);
            if (v.size() == 0) {
                printf("color [%d.1] [%d, %d, %d, %d]\n", id, p[i][0][0], p[i][0][1], p[i][0][2], p[i][0][3]);
                printf("merge [%d.0] [%d.1]\n", id, id);
            } else {
                int left = 0, right = 0, dir = 1;
                for (int j = 0; j < v.size(); j++) {
                    left += round(5.0 * h * w / (h - i) / (w - v[j]));
                    right += round(5.0 * h * w / (h - i) / v[j]);
                }
                if (left <= right) {
                    if (!same(i, v[0] - 1, i - 1, v[0] - 1)) printf("color [%d.1] [%d, %d, %d, %d]\n", id, p[i][v[0] - 1][0], p[i][v[0] - 1][1], p[i][v[0] - 1][2], p[i][v[0] - 1][3]);
                } else {
                    dir = 0;
                    reverse(v.begin(), v.end());
                    if (!same(i, v[0], i - 1, v[0])) printf("color [%d.1] [%d, %d, %d, %d]\n", id, p[i][v[0]][0], p[i][v[0]][1], p[i][v[0]][2], p[i][v[0]][3]);
                }
                for (int j = 0; j < v.size(); j++) {
                    if (j == 0) {
                        printf("cut [%d.1] [x] [%d]\n", id, v[j]);
                        printf("color [%d.1.%d] [%d, %d, %d, %d]\n", id + j, dir, p[i][v[j] + dir - 1][0], p[i][v[j] + dir - 1][1], p[i][v[j] + dir - 1][2], p[i][v[j] + dir - 1][3]);
                        printf("merge [%d.1.0] [%d.1.1]\n", id, id);
                    } else {
                        printf("cut [%d] [x] [%d]\n", id + j, v[j]);
                        printf("color [%d.%d] [%d, %d, %d, %d]\n", id + j, dir, p[i][v[j] + dir - 1][0], p[i][v[j] + dir - 1][1], p[i][v[j] + dir - 1][2], p[i][v[j] + dir - 1][3]);
                        printf("merge [%d.0] [%d.1]\n", id + j, id + j);
                    }
                }
                printf("merge [%d] [%d.0]\n", id + v.size(), id);
            }
            id += v.size() + 1;
        }
    }
    
    return 0;
}
