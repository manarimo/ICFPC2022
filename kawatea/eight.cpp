#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

const int MAX_H = 400;
const int MAX_W = 400;
const int SIZE_LIMIT = 300;
const int DIFF_LIMIT = 10;

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

char buf[10000];
int p[MAX_H][MAX_W][4];
bool updated[MAX_H][MAX_W];
double sum[MAX_H + 1][MAX_W + 1][2];
double sq[255 * 255 * 4 + 1];

double dist(int x, int y, color& c) {
    int dr = abs(p[x][y][0] - c.r);
    int dg = abs(p[x][y][1] - c.g);
    int db = abs(p[x][y][2] - c.b);
    int da = abs(p[x][y][3] - c.a);
    return sq[dr * dr + dg * dg + db * db + da * da];
}

double dist(color& c1, color& c2) {
    int dr = abs(c1.r - c2.r);
    int dg = abs(c1.g - c2.g);
    int db = abs(c1.b - c2.b);
    int da = abs(c1.a - c2.a);
    return sq[dr * dr + dg * dg + db * db + da * da];
}

int get(int x1, int y1, int x2, int y2, int t) {
    return sum[x2][y2][t] - sum[x2][y1][t] - sum[x1][y2][t] + sum[x1][y1][t];
}

int sim(int min_x, int min_y, int max_x, int max_y, int x1, int y1, int x2, int y2) {
    double diff = get(min_x, min_y, max_x, max_y, 1);
    diff -= get(x1, y1, x2, y2, 1);
    diff += get(x1, y1, x2, y2, 0);
    return diff;
}

void simplify(int h, int w) {
    for (int i = 0; i <= 255 * 255 * 4; i++) sq[i] = sqrt(i);
    
    map<color, int> mp;
    for (int x = 0; x < h; x++) {
        for (int y = 0; y < w; y++) {
            mp[color(p[x][y][0], p[x][y][1], p[x][y][2], p[x][y][3])]++;
        }
    }
    vector<pair<int, color>> v;
    for (map<color, int>::iterator it = mp.begin(); it != mp.end(); it++) v.emplace_back(it->second, it->first);
    sort(v.begin(), v.end());
    reverse(v.begin(), v.end());
    
    vector<color> checked;
    for (int i = 1, cnt = 0; cnt < 12; i++) {
        color c = v[i].second;
        if (dist(0, 0, c) < 10) continue;
        bool ok = true;
        for (int j = 0; j < checked.size(); j++) {
            if (dist(checked[j], c) < 10) {
                ok = false;
                break;
            }
        }
        int min_x = h, max_x = 0, min_y = w, max_y = 0;
        for (int x = 0; x < h; x++) {
            for (int y = 0; y < w; y++) {
                if (p[x][y][0] == c.r && p[x][y][1] == c.g && p[x][y][2] == c.b && p[x][y][3] == c.a) {
                    min_x = min(min_x, x);
                    max_x = max(max_x, x);
                    min_y = min(min_y, y);
                    max_y = max(max_y, y);
                }
            }
        }
        checked.push_back(c);
        if (!ok || min_x == h) continue;
        min_x = max(min_x - 3, 0);
        max_x = min(max_x + 4, h);
        min_y = max(min_y - 3, 0);
        max_y = min(max_y + 4, w);
        
        for (int x = min_x; x <= max_x; x++) {
            for (int y = min_y; y <= max_y; y++) {
                sum[x][y][0] = sum[x][y][1] = 0;
            }
        }
        
        color white = color(255, 255, 255, 255);
        for (int x = min_x; x < max_x; x++) {
            for (int y = min_y; y < max_y; y++) {
                sum[x + 1][y + 1][0] += sum[x][y + 1][0];
                sum[x + 1][y + 1][0] += sum[x + 1][y][0];
                sum[x + 1][y + 1][0] -= sum[x][y][0];
                sum[x + 1][y + 1][0] += dist(x, y, c);
                sum[x + 1][y + 1][1] += sum[x][y + 1][1];
                sum[x + 1][y + 1][1] += sum[x + 1][y][1];
                sum[x + 1][y + 1][1] -= sum[x][y][1];
                sum[x + 1][y + 1][1] += dist(x, y, white);
            }
        }
        
        int best = get(min_x, min_y, max_x, max_y, 1), bx1 = -1, by1 = -1, bx2 = -1, by2 = -1;
        for (int x1 = min_x; x1 < max_x; x1++) {
            for (int y1 = min_y; y1 < max_y; y1++) {
                for (int x2 = x1 + 1; x2 <= max_x; x2++) {
                    for (int y2 = y1 + 1; y2 <= max_y; y2++) {
                        int now = sim(min_x, min_y, max_x, max_y, x1, y1, x2, y2);
                        if (now < best) {
                            best = now;
                            bx1 = x1;
                            by1 = y1;
                            bx2 = x2;
                            by2 = y2;
                        }
                    }
                }
            }
        }
        
        for (int x = min_x; x < max_x; x++) {
            for (int y = min_y; y < max_y; y++) {
                updated[x][y] = true;
                if (x >= bx1 && x < bx2 && y >= by1 && y < by2) {
                    p[x][y][0] = c.r;
                    p[x][y][1] = c.g;
                    p[x][y][2] = c.b;
                    p[x][y][3] = c.a;
                } else {
                    p[x][y][0] = p[x][y][1] = p[x][y][2] = p[x][y][3] = 255;
                }
            }
        }
        cnt++;
        
        fprintf(stderr, "(%d %d) (%d %d) : (%d %d) (%d %d) -> [%d, %d, %d, %d]\n", min_x, min_y, max_x, max_y, bx1, by1, bx2, by2, c.r, c.g, c.b, c.a);
    }
    
    for (int x = 0; x < h; x++) {
        for (int y = 0; y < w; y++) {
            if (!updated[x][y]) p[x][y][0] = p[x][y][1] = p[x][y][2] = p[x][y][3] = 255;
        }
    }
}

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

string get_id(int id) {
    sprintf(buf, "%d", id);
    string s = buf;
    return s;
}

string get_id(int id, int suffix) {
    sprintf(buf, "%d.%d", id, suffix);
    string s = buf;
    return s;
}

string get_id(int id, int suffix1, int suffix2) {
    sprintf(buf, "%d.%d.%d", id, suffix1, suffix2);
    string s = buf;
    return s;
}

string get_id(const string& id, int suffix) {
    sprintf(buf, "%s.%d", id.c_str(), suffix);
    string s = buf;
    return s;
}

void cut(vector<string>& v, const string& id, int dir, int pos) {
    sprintf(buf, "cut [%s] [%c] [%d]", id.c_str(), dir == 0 ? 'x' : 'y', pos);
    string s = buf;
    v.push_back(s);
}

void color(vector<string>& v, const string& id, int x, int y) {
    sprintf(buf, "color [%s] [%d, %d, %d, %d]", id.c_str(), p[x][y][0], p[x][y][1], p[x][y][2], p[x][y][3]);
    string s = buf;
    v.push_back(s);
}

void merge(vector<string>& v, const string& id1, const string& id2) {
    sprintf(buf, "merge [%s] [%s]", id1.c_str(), id2.c_str());
    string s = buf;
    v.push_back(s);
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
    
    simplify(h, w);
    
    int best = 1e9;
    vector<string> ans;
    {
        int id = 0, cost = 0;
        vector<string> now;
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
                    if (!white(0, 0)) {
                        color(now, get_id(id), 0, 0);
                        cost += 5;
                    }
                } else {
                    int cost1 = 0, cost2 = 0;
                    vector<string> tmp1, tmp2;
                    if (!white(0, 0)) {
                        color(tmp1, get_id(id), 0, 0);
                        cost1 += 5;
                    }
                    for (int j = 0; j < v.size(); j++) {
                        cut(tmp1, get_id(id + j), 0, v[j]);
                        color(tmp1, get_id(id + j, 1), i, v[j]);
                        merge(tmp1, get_id(id + j, 0), get_id(id + j, 1));
                        cost1 += round(7.0 * h / (h - i)) + round(5.0 * h * w / (h - i) / (w - v[j])) + round(1.0 * h * w / (h - i) / max(v[j], w - v[j]));
                    }
                    
                    reverse(v.begin(), v.end());
                    if (!white(0, w - 1)) {
                        color(tmp2, get_id(id), 0, w - 1);
                        cost2 += 5;
                    }
                    for (int j = 0; j < v.size(); j++) {
                        cut(tmp2, get_id(id + j), 0, v[j]);
                        color(tmp2, get_id(id + j, 0), i, v[j] - 1);
                        merge(tmp2, get_id(id + j, 0), get_id(id + j, 1));
                        cost2 += round(7.0 * h / (h - i)) + round(5.0 * h * w / (h - i) / v[j]) + round(1.0 * h * w / (h - i) / max(v[j], w - v[j]));
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                    }
                }
                id += v.size();
            } else {
                cut(now, get_id(id), 1, i);
                cost += 7;
                if (v.size() == 0) {
                    color(now, get_id(id, 1), i, 0);
                    merge(now, get_id(id, 0), get_id(id, 1));
                    cost += round(5.0 * h / (h - i)) + round(1.0 * h / max(i, h - i));
                    id++;
                } else {
                    int cost1 = 0, cost2 = 0, cnt1 = 0, cnt2 = 0;
                    bool first1 = false, first2 = false;
                    vector<string> tmp1, tmp2;
                    for (int k = 0; k < v[0]; k++) {
                        if (!same(i, k, i - 1, k)) {
                            first1 = true;
                            color(tmp1, get_id(id, 1), i, v[0] - 1);
                            cost1 += round(5.0 * h / (h - i));
                            break;
                        }
                    }
                    for (int j = 0; j < v.size(); j++) {
                        if (!first1 && cnt1 == 0) {
                            bool ok = true;
                            int next = ((j + 1 == v.size()) ? w : v[j + 1]);
                            for (int k = v[j] - 1; k < next; k++) {
                                if (!same(i, k, i - 1, k)) {
                                    ok = false;
                                    break;
                                }
                            }
                            if (ok) continue;
                        }
                        string sid;
                        if (cnt1 == 0) {
                            sid = get_id(id + cnt1, 1);
                        } else {
                            sid = get_id(id + cnt1);
                        }
                        cut(tmp1, sid, 0, v[j]);
                        color(tmp1, get_id(sid, 1), i, v[j]);
                        merge(tmp1, get_id(sid, 0), get_id(sid, 1));
                        cost1 += round(7.0 * h / (h - i)) + round(5.0 * h * w / (h - i) / (w - v[j])) + round(1.0 * h * w / (h - i) / max(v[j], w - v[j]));
                        cnt1++;
                    }
                    
                    reverse(v.begin(), v.end());
                    for (int k = v[0]; k < w; k++) {
                        if (!same(i, k, i - 1, k)) {
                            first2 = true;
                            color(tmp2, get_id(id, 1), i, v[0]);
                            cost2 += round(5.0 * h / (h - i));
                            break;
                        }
                    }
                    for (int j = 0; j < v.size(); j++) {
                        if (!first2 && cnt2 == 0) {
                            bool ok = true;
                            int next = ((j + 1 == v.size()) ? 0 : v[j + 1]);
                            for (int k = next; k <= v[j]; k++) {
                                if (!same(i, k, i - 1, k)) {
                                    ok = false;
                                    break;
                                }
                            }
                            if (ok) continue;
                        }
                        string sid;
                        if (cnt2 == 0) {
                            sid = get_id(id + cnt2, 1);
                        } else {
                            sid = get_id(id + cnt2);
                        }
                        cut(tmp2, sid, 0, v[j]);
                        color(tmp2, get_id(sid, 0), i, v[j] - 1);
                        merge(tmp2, get_id(sid, 0), get_id(sid, 1));
                        cost2 += round(7.0 * h / (h - i)) + round(5.0 * h * w / (h - i) / v[j]) + round(1.0 * h * w / (h - i) / max(v[j], w - v[j]));
                        cnt2++;
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                        merge(now, get_id(id + cnt1), get_id(id, 0));
                        cost += round(1.0 * h / max(i, h - i));
                        id += cnt1 + 1;
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                        merge(now, get_id(id + cnt2), get_id(id, 0));
                        cost += round(1.0 * h / max(i, h - i));
                        id += cnt2 + 1;
                    }
                }
            }
        }
        
        if (cost < best) {
            best = cost;
            ans = now;
        }
    }
    {
        int id = 0, cost = 0;
        vector<string> now;
        for (int i = h - 1; i >= 0; i--) {
            bool update = false;
            vector<int> v;
            for (int j = 0; j < w; j++) {
                if (i == h - 1 || !same(i, j, i + 1, j)) update = true;
                if (j > 0 && !same(i, j, i, j - 1)) v.push_back(j);
            }
            if (!update) continue;
            
            if (i == h - 1) {
                if (v.size() == 0) {
                    if (!white(h - 1, 0)) {
                        color(now, get_id(id), h - 1, 0);
                        cost += 5;
                    }
                } else {
                    int cost1 = 0, cost2 = 0;
                    vector<string> tmp1, tmp2;
                    if (!white(h - 1, 0)) {
                        color(tmp1, get_id(id), h - 1, 0);
                        cost1 += 5;
                    }
                    for (int j = 0; j < v.size(); j++) {
                        cut(tmp1, get_id(id + j), 0, v[j]);
                        color(tmp1, get_id(id + j, 1), i, v[j]);
                        merge(tmp1, get_id(id + j, 0), get_id(id + j, 1));
                        cost1 += round(7.0 * h / (i + 1)) + round(5.0 * h * w / (i + 1) / (w - v[j])) + round(1.0 * h * w / (i + 1) / max(v[j], w - v[j]));
                    }
                    
                    reverse(v.begin(), v.end());
                    if (!white(h - 1, w - 1)) {
                        color(tmp2, get_id(id), h - 1, w - 1);
                        cost2 += 5;
                    }
                    for (int j = 0; j < v.size(); j++) {
                        cut(tmp2, get_id(id + j), 0, v[j]);
                        color(tmp2, get_id(id + j, 0), i, v[j] - 1);
                        merge(tmp2, get_id(id + j, 0), get_id(id + j, 1));
                        cost2 += round(7.0 * h / (i + 1)) + round(5.0 * h * w / (i + 1) / v[j]) + round(1.0 * h * w / (i + 1) / max(v[j], w - v[j]));
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                    }
                }
                id += v.size();
            } else {
                cut(now, get_id(id), 1, i + 1);
                cost += 7;
                if (v.size() == 0) {
                    color(now, get_id(id, 0), i, 0);
                    merge(now, get_id(id, 0), get_id(id, 1));
                    cost += round(5.0 * h / (i + 1)) + round(1.0 * h / max(i + 1, h - i - 1));
                    id++;
                } else {
                    int cost1 = 0, cost2 = 0, cnt1 = 0, cnt2 = 0;
                    bool first1 = false, first2 = false;
                    vector<string> tmp1, tmp2;
                    for (int k = 0; k < v[0]; k++) {
                        if (!same(i, k, i + 1, k)) {
                            first1 = true;
                            color(tmp1, get_id(id, 0), i, v[0] - 1);
                            cost1 += round(5.0 * h / (i + 1));
                            break;
                        }
                    }
                    for (int j = 0; j < v.size(); j++) {
                        if (!first1 && cnt1 == 0) {
                            bool ok = true;
                            int next = ((j + 1 == v.size()) ? w : v[j + 1]);
                            for (int k = v[j] - 1; k < next; k++) {
                                if (!same(i, k, i + 1, k)) {
                                    ok = false;
                                    break;
                                }
                            }
                            if (ok) continue;
                        }
                        string sid;
                        if (cnt1 == 0) {
                            sid = get_id(id + cnt1, 0);
                        } else {
                            sid = get_id(id + cnt1);
                        }
                        cut(tmp1, sid, 0, v[j]);
                        color(tmp1, get_id(sid, 1), i, v[j]);
                        merge(tmp1, get_id(sid, 0), get_id(sid, 1));
                        cost1 += round(7.0 * h / (i + 1)) + round(5.0 * h * w / (i + 1) / (w - v[j])) + round(1.0 * h * w / (i + 1) / max(v[j], w - v[j]));
                        cnt1++;
                    }
                    
                    reverse(v.begin(), v.end());
                    for (int k = v[0]; k < w; k++) {
                        if (!same(i, k, i + 1, k)) {
                            first2 = true;
                            color(tmp2, get_id(id, 0), i, v[0]);
                            cost2 += round(5.0 * h / (i + 1));
                            break;
                        }
                    }
                    for (int j = 0; j < v.size(); j++) {
                        if (!first2 && cnt2 == 0) {
                            bool ok = true;
                            int next = ((j + 1 == v.size()) ? 0 : v[j + 1]);
                            for (int k = next; k <= v[j]; k++) {
                                if (!same(i, k, i + 1, k)) {
                                    ok = false;
                                    break;
                                }
                            }
                            if (ok) continue;
                        }
                        string sid;
                        if (cnt2 == 0) {
                            sid = get_id(id + cnt2, 0);
                        } else {
                            sid = get_id(id + cnt2);
                        }
                        cut(tmp2, sid, 0, v[j]);
                        color(tmp2, get_id(sid, 0), i, v[j] - 1);
                        merge(tmp2, get_id(sid, 0), get_id(sid, 1));
                        cost2 += round(7.0 * h / (i + 1)) + round(5.0 * h * w / (i + 1) / v[j]) + round(1.0 * h * w / (i + 1) / max(v[j], w - v[j]));
                        cnt2++;
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                        merge(now, get_id(id + cnt1), get_id(id, 1));
                        cost += round(1.0 * h / max(i + 1, h - i - 1));
                        id += cnt1 + 1;
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                        merge(now, get_id(id + cnt2), get_id(id, 1));
                        cost += round(1.0 * h / max(i + 1, h - i - 1));
                        id += cnt2 + 1;
                    }
                }
            }
        }
        
        if (cost < best) {
            best = cost;
            ans = now;
        }
    }
    {
        int id = 0, cost = 0;
        vector<string> now;
        for (int i = 0; i < w; i++) {
            bool update = false;
            vector<int> v;
            for (int j = 0; j < h; j++) {
                if (i == 0 || !same(j, i, j, i - 1)) update = true;
                if (j > 0 && !same(j, i, j - 1, i)) v.push_back(j);
            }
            if (!update) continue;
            
            if (i == 0) {
                if (v.size() == 0) {
                    if (!white(0, 0)) {
                        color(now, get_id(id), 0, 0);
                        cost += 5;
                    }
                } else {
                    int cost1 = 0, cost2 = 0;
                    vector<string> tmp1, tmp2;
                    if (!white(0, 0)) {
                        color(tmp1, get_id(id), 0, 0);
                        cost1 += 5;
                    }
                    for (int j = 0; j < v.size(); j++) {
                        cut(tmp1, get_id(id + j), 1, v[j]);
                        color(tmp1, get_id(id + j, 1), v[j], i);
                        merge(tmp1, get_id(id + j, 0), get_id(id + j, 1));
                        cost1 += round(7.0 * w / (w - i)) + round(5.0 * h * w / (w - i) / (h - v[j])) + round(1.0 * h * w / (w - i) / max(v[j], h - v[j]));
                    }
                    
                    reverse(v.begin(), v.end());
                    if (!white(h - 1, 0)) {
                        color(tmp2, get_id(id), h - 1, 0);
                        cost2 += 5;
                    }
                    for (int j = 0; j < v.size(); j++) {
                        cut(tmp2, get_id(id + j), 1, v[j]);
                        color(tmp2, get_id(id + j, 0), v[j] - 1, i);
                        merge(tmp2, get_id(id + j, 0), get_id(id + j, 1));
                        cost2 += round(7.0 * w / (w - i)) + round(5.0 * h * w / (w - i) / v[j]) + round(1.0 * h * w / (w - i) / max(v[j], h - v[j]));
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                    }
                }
                id += v.size();
            } else {
                cut(now, get_id(id), 0, i);
                cost += 7;
                if (v.size() == 0) {
                    color(now, get_id(id, 1), 0, i);
                    merge(now, get_id(id, 0), get_id(id, 1));
                    cost += round(5.0 * w / (w - i)) + round(1.0 * w / max(i, w - i));
                    id++;
                } else {
                    int cost1 = 0, cost2 = 0, cnt1 = 0, cnt2 = 0;
                    bool first1 = false, first2 = false;
                    vector<string> tmp1, tmp2;
                    for (int k = 0; k < v[0]; k++) {
                        if (!same(k, i, k, i - 1)) {
                            first1 = true;
                            color(tmp1, get_id(id, 1), v[0] - 1, i);
                            cost1 += round(5.0 * w / (w - i));
                            break;
                        }
                    }
                    for (int j = 0; j < v.size(); j++) {
                        if (!first1 && cnt1 == 0) {
                            bool ok = true;
                            int next = ((j + 1 == v.size()) ? h : v[j + 1]);
                            for (int k = v[j] - 1; k < next; k++) {
                                if (!same(k, i, k, i - 1)) {
                                    ok = false;
                                    break;
                                }
                            }
                            if (ok) continue;
                        }
                        string sid;
                        if (cnt1 == 0) {
                            sid = get_id(id + cnt1, 1);
                        } else {
                            sid = get_id(id + cnt1);
                        }
                        cut(tmp1, sid, 1, v[j]);
                        color(tmp1, get_id(sid, 1), v[j], i);
                        merge(tmp1, get_id(sid, 0), get_id(sid, 1));
                        cost1 += round(7.0 * w / (w - i)) + round(5.0 * h * w / (w - i) / (h - v[j])) + round(1.0 * h * w / (w - i) / max(v[j], h - v[j]));
                        cnt1++;
                    }
                    
                    reverse(v.begin(), v.end());
                    for (int k = v[0]; k < w; k++) {
                        if (!same(k, i, k, i - 1)) {
                            first2 = true;
                            color(tmp2, get_id(id, 1), v[0], i);
                            cost2 += round(5.0 * w / (w - i));
                            break;
                        }
                    }
                    for (int j = 0; j < v.size(); j++) {
                        if (!first2 && cnt2 == 0) {
                            bool ok = true;
                            int next = ((j + 1 == v.size()) ? 0 : v[j + 1]);
                            for (int k = next; k <= v[j]; k++) {
                                if (!same(k, i, k, i - 1)) {
                                    ok = false;
                                    break;
                                }
                            }
                            if (ok) continue;
                        }
                        string sid;
                        if (cnt2 == 0) {
                            sid = get_id(id + cnt2, 1);
                        } else {
                            sid = get_id(id + cnt2);
                        }
                        cut(tmp2, sid, 1, v[j]);
                        color(tmp2, get_id(sid, 0), v[j] - 1, i);
                        merge(tmp2, get_id(sid, 0), get_id(sid, 1));
                        cost2 += round(7.0 * w / (w - i)) + round(5.0 * h * w / (w - i) / v[j]) + round(1.0 * h * w / (w - i) / max(v[j], h - v[j]));
                        cnt2++;
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                        merge(now, get_id(id + cnt1), get_id(id, 0));
                        cost += round(1.0 * w / max(i, w - i));
                        id += cnt1 + 1;
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                        merge(now, get_id(id + cnt2), get_id(id, 0));
                        cost += round(1.0 * w / max(i, w - i));
                        id += cnt2 + 1;
                    }
                }
            }
        }
        
        if (cost < best) {
            best = cost;
            ans = now;
        }
    }
    {
        int id = 0, cost = 0;
        vector<string> now;
        for (int i = w - 1; i >= 0; i--) {
            bool update = false;
            vector<int> v;
            for (int j = 0; j < h; j++) {
                if (i == w - 1 || !same(j, i, j, i + 1)) update = true;
                if (j > 0 && !same(j, i, j - 1, i)) v.push_back(j);
            }
            if (!update) continue;
            
            if (i == w - 1) {
                if (v.size() == 0) {
                    if (!white(0, w - 1)) {
                        color(now, get_id(id), 0, w - 1);
                        cost += 5;
                    }
                } else {
                    int cost1 = 0, cost2 = 0;
                    vector<string> tmp1, tmp2;
                    if (!white(0, w - 1)) {
                        color(tmp1, get_id(id), 0, w - 1);
                        cost1 += 5;
                    }
                    for (int j = 0; j < v.size(); j++) {
                        cut(tmp1, get_id(id + j), 1, v[j]);
                        color(tmp1, get_id(id + j, 1), v[j], i);
                        merge(tmp1, get_id(id + j, 0), get_id(id + j, 1));
                        cost1 += round(7.0 * w / (i + 1)) + round(5.0 * h * w / (i + 1) / (h - v[j])) + round(1.0 * h * w / (i + 1) / max(v[j], h - v[j]));
                    }
                    
                    reverse(v.begin(), v.end());
                    if (!white(h - 1, w - 1)) {
                        color(tmp2, get_id(id), h - 1, w - 1);
                        cost2 += 5;
                    }
                    for (int j = 0; j < v.size(); j++) {
                        cut(tmp2, get_id(id + j), 1, v[j]);
                        color(tmp2, get_id(id + j, 0), v[j] - 1, i);
                        merge(tmp2, get_id(id + j, 0), get_id(id + j, 1));
                        cost2 += round(7.0 * w / (i + 1)) + round(5.0 * h * w / (i + 1) / v[j]) + round(1.0 * h * w / (i + 1) / max(v[j], h - v[j]));
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                    }
                }
                id += v.size();
            } else {
                cut(now, get_id(id), 0, i + 1);
                cost += 7;
                if (v.size() == 0) {
                    color(now, get_id(id, 0), 0, i);
                    merge(now, get_id(id, 0), get_id(id, 1));
                    cost += round(5.0 * w / (i + 1)) + round(1.0 * w / max(i + 1, w - i - 1));
                    id++;
                } else {
                    int cost1 = 0, cost2 = 0, cnt1 = 0, cnt2 = 0;
                    bool first1 = false, first2 = false;
                    vector<string> tmp1, tmp2;
                    for (int k = 0; k < v[0]; k++) {
                        if (!same(k, i, k, i + 1)) {
                            first1 = true;
                            color(tmp1, get_id(id, 0), v[0] - 1, i);
                            cost1 += round(5.0 * w / (i + 1));
                            break;
                        }
                    }
                    for (int j = 0; j < v.size(); j++) {
                        if (!first1 && cnt1 == 0) {
                            bool ok = true;
                            int next = ((j + 1 == v.size()) ? h : v[j + 1]);
                            for (int k = v[j] - 1; k < next; k++) {
                                if (!same(k, i, k, i + 1)) {
                                    ok = false;
                                    break;
                                }
                            }
                            if (ok) continue;
                        }
                        string sid;
                        if (cnt1 == 0) {
                            sid = get_id(id + cnt1, 0);
                        } else {
                            sid = get_id(id + cnt1);
                        }
                        cut(tmp1, sid, 1, v[j]);
                        color(tmp1, get_id(sid, 1), v[j], i);
                        merge(tmp1, get_id(sid, 0), get_id(sid, 1));
                        cost1 += round(7.0 * w / (i + 1)) + round(5.0 * h * w / (i + 1) / (h - v[j])) + round(1.0 * h * w / (i + 1) / max(v[j], h - v[j]));
                        cnt1++;
                    }
                    
                    reverse(v.begin(), v.end());
                    for (int k = v[0]; k < w; k++) {
                        if (!same(k, i, k, i + 1)) {
                            first2 = true;
                            color(tmp2, get_id(id, 0), v[0], i);
                            cost2 += round(5.0 * w / (i + 1));
                            break;
                        }
                    }
                    for (int j = 0; j < v.size(); j++) {
                        if (!first2 && cnt2 == 0) {
                            bool ok = true;
                            int next = ((j + 1 == v.size()) ? 0 : v[j + 1]);
                            for (int k = next; k <= v[j]; k++) {
                                if (!same(k, i, k, i + 1)) {
                                    ok = false;
                                    break;
                                }
                            }
                            if (ok) continue;
                        }
                        string sid;
                        if (cnt2 == 0) {
                            sid = get_id(id + cnt2, 0);
                        } else {
                            sid = get_id(id + cnt2);
                        }
                        cut(tmp2, sid, 1, v[j]);
                        color(tmp2, get_id(sid, 0), v[j] - 1, i);
                        merge(tmp2, get_id(sid, 0), get_id(sid, 1));
                        cost2 += round(7.0 * w / (i + 1)) + round(5.0 * h * w / (i + 1) / v[j]) + round(1.0 * h * w / (i + 1) / max(v[j], h - v[j]));
                        cnt2++;
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                        merge(now, get_id(id + cnt1), get_id(id, 1));
                        cost += round(1.0 * w / max(i + 1, w - i - 1));
                        id += cnt1 + 1;
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                        merge(now, get_id(id + cnt2), get_id(id, 1));
                        cost += round(1.0 * w / max(i + 1, w - i - 1));
                        id += cnt2 + 1;
                    }
                }
            }
        }
        
        if (cost < best) {
            best = cost;
            ans = now;
        }
    }
    
    fprintf(stderr, "%d\n", best);
    for (int i = 0; i < ans.size(); i++) printf("%s\n", ans[i].c_str());
    
    return 0;
}
