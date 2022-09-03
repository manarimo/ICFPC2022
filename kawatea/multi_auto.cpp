#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

const int MAX_H = 400;
const int MAX_W = 400;
const int SIZE_LIMIT = 150;
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
int sum[MAX_H + 1][MAX_W + 1][4];
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

void fill(int x1, int y1, int x2, int y2) {
    map<color, int> mp;
    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
            mp[color(p[x][y][0], p[x][y][1], p[x][y][2], p[x][y][3])]++;
        }
    }
    vector<pair<color, int>> v;
    for (map<color, int>::iterator it = mp.begin(); it != mp.end(); it++) v.emplace_back(it->first, it->second);
    sort(v.begin(), v.end());
    reverse(v.begin(), v.end());
    
    int best = 1e9, br = 0, bg = 0, bb = 0, ba = 0;
    for (int i = 0; i < v.size(); i++) {
        int r = v[i].first.r;
        int g = v[i].first.g;
        int b = v[i].first.b;
        int a = v[i].first.a;
        int now = calc(best, r, g, b, a, v);
        if (now < best) {
            best = now;
            br = r;
            bg = g;
            bb = b;
            ba = a;
        }
    }
    
    for (int x = x1; x < x2; x++) {
        for (int y = y1; y < y2; y++) {
            p[x][y][0] = br;
            p[x][y][1] = bg;
            p[x][y][2] = bb;
            p[x][y][3] = ba;
        }
    }
}

int diff(int x1, int y1, int x2, int y2) {
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        int d = abs(p[x1][y1][i] - p[x2][y2][i]);
        sum += d * d;
    }
    return sq[sum];
}

void simplify(int x1, int y1, int x2, int y2) {
    int best_diff = 0, best_x = -1, best_y = -1;
    for (int x = x1 + 1; x < x2; x++) {
        if ((x - x1) * (y2 - y1) < SIZE_LIMIT || (x2 - x) * (y2 - y1) < SIZE_LIMIT) continue;
        int sum = 0;
        for (int y = y1; y < y2; y++) sum += diff(x - 1, y, x, y);
        if (sum < DIFF_LIMIT * (y2 - y1)) continue;
        if (sum > best_diff) {
            best_diff = sum;
            best_x = x;
            best_y = -1;
        }
    }
    for (int y = y1 + 1; y < y2; y++) {
        if ((x2 - x1) * (y - y1) < SIZE_LIMIT || (x2 - x1) * (y2 - y) < SIZE_LIMIT) continue;
        int sum = 0;
        for (int x = x1; x < x2; x++) sum += diff(x, y - 1, x, y);
        if (sum < DIFF_LIMIT * (x2 - x1)) continue;
        if (sum > best_diff) {
            best_diff = sum;
            best_x = -1;
            best_y = y;
        }
    }
    
    if (best_diff == 0) {
        fill(x1, y1, x2, y2);
    } else if (best_x != -1) {
        simplify(x1, y1, best_x, y2);
        simplify(best_x, y1, x2, y2);
    } else {
        simplify(x1, y1, x2, best_y);
        simplify(x1, best_y, x2, y2);
    }
}

void simplify(int h, int w) {
    for (int k = 0; k < 4; k++) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                sum[i + 1][j + 1][k] += sum[i][j + 1][k];
                sum[i + 1][j + 1][k] += sum[i + 1][j][k];
                sum[i + 1][j + 1][k] -= sum[i][j][k];
                sum[i + 1][j + 1][k] += p[i][j][k];
            }
        }
    }
    
    for (int i = 0; i <= 255 * 255 * 4; i++) sq[i] = sqrt(i);
    
    simplify(0, 0, h, w);
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
