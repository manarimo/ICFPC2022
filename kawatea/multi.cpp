#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

const int MAX_H = 400;
const int MAX_W = 400;

char buf[10000];
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
                } else {
                    int cost1 = 0, cost2 = 0;
                    vector<string> tmp1, tmp2;
                    if (!same(i, v[0] - 1, i - 1, v[0] - 1)) {
                        color(tmp1, get_id(id, 1), i, v[0] - 1);
                        cost1 += round(5.0 * h / (h - i));
                    }
                    for (int j = 0; j < v.size(); j++) {
                        string sid;
                        if (j == 0) {
                            sid = get_id(id + j, 1);
                        } else {
                            sid = get_id(id + j);
                        }
                        cut(tmp1, sid, 0, v[j]);
                        color(tmp1, get_id(sid, 1), i, v[j]);
                        merge(tmp1, get_id(sid, 0), get_id(sid, 1));
                        cost1 += round(7.0 * h / (h - i)) + round(5.0 * h * w / (h - i) / (w - v[j])) + round(1.0 * h * w / (h - i) / max(v[j], w - v[j]));
                    }
                    
                    reverse(v.begin(), v.end());
                    if (!same(i, v[0], i - 1, v[0])) {
                        color(tmp2, get_id(id, 1), i, v[0]);
                        cost2 += round(5.0 * h / (h - i));
                    }
                    for (int j = 0; j < v.size(); j++) {
                        string sid;
                        if (j == 0) {
                            sid = get_id(id + j, 1);
                        } else {
                            sid = get_id(id + j);
                        }
                        cut(tmp2, sid, 0, v[j]);
                        color(tmp2, get_id(sid, 0), i, v[j] - 1);
                        merge(tmp2, get_id(sid, 0), get_id(sid, 1));
                        cost2 += round(7.0 * h / (h - i)) + round(5.0 * h * w / (h - i) / v[j]) + round(1.0 * h * w / (h - i) / max(v[j], w - v[j]));
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                    }
                    merge(now, get_id(id + v.size()), get_id(id, 0));
                    cost += round(1.0 * h / max(i, h - i));
                }
                id += v.size() + 1;
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
                } else {
                    int cost1 = 0, cost2 = 0;
                    vector<string> tmp1, tmp2;
                    if (!same(i, v[0] - 1, i + 1, v[0] - 1)) {
                        color(tmp1, get_id(id, 0), i, v[0] - 1);
                        cost1 += round(5.0 * h / (i + 1));
                    }
                    for (int j = 0; j < v.size(); j++) {
                        string sid;
                        if (j == 0) {
                            sid = get_id(id + j, 0);
                        } else {
                            sid = get_id(id + j);
                        }
                        cut(tmp1, sid, 0, v[j]);
                        color(tmp1, get_id(sid, 1), i, v[j]);
                        merge(tmp1, get_id(sid, 0), get_id(sid, 1));
                        cost1 += round(7.0 * h / (i + 1)) + round(5.0 * h * w / (i + 1) / (w - v[j])) + round(1.0 * h * w / (i + 1) / max(v[j], w - v[j]));
                    }
                    
                    reverse(v.begin(), v.end());
                    if (!same(i, v[0], i + 1, v[0])) {
                        color(tmp2, get_id(id, 0), i, v[0]);
                        cost2 += round(5.0 * h / (i + 1));
                    }
                    for (int j = 0; j < v.size(); j++) {
                        string sid;
                        if (j == 0) {
                            sid = get_id(id + j, 0);
                        } else {
                            sid = get_id(id + j);
                        }
                        cut(tmp2, sid, 0, v[j]);
                        color(tmp2, get_id(sid, 0), i, v[j] - 1);
                        merge(tmp2, get_id(sid, 0), get_id(sid, 1));
                        cost2 += round(7.0 * h / (i + 1)) + round(5.0 * h * w / (i + 1) / v[j]) + round(1.0 * h * w / (i + 1) / max(v[j], w - v[j]));
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                    }
                    merge(now, get_id(id + v.size()), get_id(id, 1));
                    cost += round(1.0 * h / max(i + 1, h - i - 1));
                }
                id += v.size() + 1;
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
                        color(tmp2, get_id(id + j, 1), v[j] - 1, i);
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
                } else {
                    int cost1 = 0, cost2 = 0;
                    vector<string> tmp1, tmp2;
                    if (!same(v[0] - 1, i, v[0] - 1, i - 1)) {
                        color(tmp1, get_id(id, 1), v[0] - 1, i);
                        cost1 += round(5.0 * w / (w - i));
                    }
                    for (int j = 0; j < v.size(); j++) {
                        string sid;
                        if (j == 0) {
                            sid = get_id(id + j, 1);
                        } else {
                            sid = get_id(id + j);
                        }
                        cut(tmp1, sid, 1, v[j]);
                        color(tmp1, get_id(sid, 1), v[j], i);
                        merge(tmp1, get_id(sid, 0), get_id(sid, 1));
                        cost1 += round(7.0 * w / (w - i)) + round(5.0 * h * w / (w - i) / (h - v[j])) + round(1.0 * h * w / (w - i) / max(v[j], h - v[j]));
                    }
                    
                    reverse(v.begin(), v.end());
                    if (!same(v[0], i, v[0], i - 1)) {
                        color(tmp2, get_id(id, 1), v[0], i);
                        cost2 += round(5.0 * w / (w - i));
                    }
                    for (int j = 0; j < v.size(); j++) {
                        string sid;
                        if (j == 0) {
                            sid = get_id(id + j, 1);
                        } else {
                            sid = get_id(id + j);
                        }
                        cut(tmp2, sid, 1, v[j]);
                        color(tmp2, get_id(sid, 0), v[j] - 1, i);
                        merge(tmp2, get_id(sid, 0), get_id(sid, 1));
                        cost2 += round(7.0 * w / (w - i)) + round(5.0 * h * w / (w - i) / v[j]) + round(1.0 * h * w / (w - i) / max(v[j], h - v[j]));
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                    }
                    merge(now, get_id(id + v.size()), get_id(id, 0));
                    cost += round(1.0 * w / max(i, w - i));
                }
                id += v.size() + 1;
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
                } else {
                    int cost1 = 0, cost2 = 0;
                    vector<string> tmp1, tmp2;
                    if (!same(v[0] - 1, i, v[0] - 1, i + 1)) {
                        color(tmp1, get_id(id, 0), v[0] - 1, i);
                        cost1 += round(5.0 * w / (i + 1));
                    }
                    for (int j = 0; j < v.size(); j++) {
                        string sid;
                        if (j == 0) {
                            sid = get_id(id + j, 0);
                        } else {
                            sid = get_id(id + j);
                        }
                        cut(tmp1, sid, 1, v[j]);
                        color(tmp1, get_id(sid, 1), v[j], i);
                        merge(tmp1, get_id(sid, 0), get_id(sid, 1));
                        cost1 += round(7.0 * w / (i + 1)) + round(5.0 * h * w / (i + 1) / (h - v[j])) + round(1.0 * h * w / (i + 1) / max(v[j], h - v[j]));
                    }
                    
                    reverse(v.begin(), v.end());
                    if (!same(v[0], i, v[0], i + 1)) {
                        color(tmp2, get_id(id, 0), v[0], i);
                        cost2 += round(5.0 * w / (i + 1));
                    }
                    for (int j = 0; j < v.size(); j++) {
                        string sid;
                        if (j == 0) {
                            sid = get_id(id + j, 0);
                        } else {
                            sid = get_id(id + j);
                        }
                        cut(tmp2, sid, 1, v[j]);
                        color(tmp2, get_id(sid, 0), v[j] - 1, i);
                        merge(tmp2, get_id(sid, 0), get_id(sid, 1));
                        cost2 += round(7.0 * w / (i + 1)) + round(5.0 * h * w / (i + 1) / v[j]) + round(1.0 * h * w / (i + 1) / max(v[j], h - v[j]));
                    }
                    
                    if (cost1 <= cost2) {
                        cost += cost1;
                        now.insert(now.end(), tmp1.begin(), tmp1.end());
                    } else {
                        cost += cost2;
                        now.insert(now.end(), tmp2.begin(), tmp2.end());
                    }
                    merge(now, get_id(id + v.size()), get_id(id, 1));
                    cost += round(1.0 * w / max(i + 1, w - i - 1));
                }
                id += v.size() + 1;
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
