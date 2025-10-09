// DSU Rollback (0-index) — hợp/hoàn tác theo checkpoint
// Ghi chú nhanh:
// - Biến: parent[u], sz[u], comps (số thành phần), st (stack thay đổi)
// - Hàm:
//   init(n): khởi tạo n tập rời
//   find(u): tìm gốc (không path compression)
//   unite(u,v): hợp 2 tập, lưu thay đổi để hoàn tác, trả về true nếu gộp được
//   snapshot(): trả về checkpoint hiện tại (kích thước stack)
//   rollback(snap): hoàn tác về checkpoint snap
//   undo(): hoàn tác 1 bước unite gần nhất
//   size(u): kích thước tập chứa u
//   components(): số thành phần hiện tại
// - Cách dùng: int snap = dsu.snapshot(); dsu.unite(u,v); ...; dsu.rollback(snap);

#include <bits/stdc++.h>
using namespace std;

struct DSURollback {
    int n, comps;
    vector<int> parent, sz;
    struct Change { int v, old_parent, u, old_size; };
    vector<Change> st;

    DSURollback(int n = 0) { init(n); }

    void init(int n_) {
        n = n_;
        parent.resize(n);
        iota(parent.begin(), parent.end(), 0);
        sz.assign(n, 1);
        st.clear();
        comps = n;
    }

    int find(int x) {
        while (parent[x] != x) x = parent[x];
        return x;
    }

    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (sz[a] < sz[b]) swap(a, b);
        st.push_back({b, parent[b], a, sz[a]});
        parent[b] = a;
        sz[a] += sz[b];
        comps--;
        return true;
    }

    int snapshot() const { return (int)st.size(); }

    void rollback(int snap) {
        while ((int)st.size() > snap) undo();
    }

    bool undo() {
        if (st.empty()) return false;
        auto c = st.back(); st.pop_back();
        parent[c.v] = c.old_parent;
        sz[c.u] = c.old_size;
        comps++;
        return true;
    }

    int size(int x) { return sz[find(x)]; }
    int components() const { return comps; }
};
