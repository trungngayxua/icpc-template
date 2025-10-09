// TwoSAT (0-indexed, packaged API) — Kosaraju SCC
// API: add_or, add_imp, add_true/false, add_xor, add_equiv, add_at_most_one

#include <bits/stdc++.h>
using namespace std;

struct TwoSAT {
    int n;
    vector<vector<int>> g, gr;
    vector<int> comp, order, vis;
    vector<int> assignv; // 0 = false, 1 = true

    explicit TwoSAT(int n = 0) { init(n); }

    void init(int _n) {
        n = _n;
        g.assign(2 * n, {});
        gr.assign(2 * n, {});
        comp.assign(2 * n, -1);
        order.clear();
        vis.assign(2 * n, 0);
        assignv.assign(n, 0);
    }

    inline int var(int x, bool is_true) const {
        // node index for literal: x in [0..n-1]
        // (x = true)  -> 2*x
        // (x = false) -> 2*x ^ 1
        return (x << 1) ^ (is_true ? 0 : 1);
    }

    inline int negate(int u) const { return u ^ 1; }

    void add_imp(int u, bool u_true, int v, bool v_true) {
        int a = var(u, u_true);
        int b = var(v, v_true);
        g[a].push_back(b);
        gr[b].push_back(a);
    }

    void add_or(int u, bool u_true, int v, bool v_true) {
        // (u == u_true) OR (v == v_true)
        // => (¬u -> v) and (¬v -> u)
        int a = var(u, u_true);
        int b = var(v, v_true);
        g[negate(a)].push_back(b);
        gr[b].push_back(negate(a));
        g[negate(b)].push_back(a);
        gr[a].push_back(negate(b));
    }

    void add_true(int u)  { add_or(u, true, u, true); }   // u = true
    void add_false(int u) { add_or(u, false, u, false); } // u = false

    void add_equiv(int u, bool ut, int v, bool vt) {
        // (u==ut) ↔ (v==vt)
        add_imp(u, ut, v, vt);
        add_imp(v, vt, u, ut);
        add_imp(u, !ut, v, !vt);
        add_imp(v, !vt, u, !ut);
    }

    void add_xor(int u, bool ut, int v, bool vt) {
        // (u==ut) XOR (v==vt)  ≡  (u==ut ∨ v==vt) ∧ (u!=ut ∨ v!=vt)
        add_or(u, ut, v, vt);
        add_or(u, !ut, v, !vt);
    }

    // Pairwise at-most-one on literals lits = {(x1,b1), (x2,b2), ...}
    void add_at_most_one(const vector<pair<int,bool>>& lits) {
        int k = (int)lits.size();
        for (int i = 0; i < k; i++) {
            for (int j = i + 1; j < k; j++) {
                add_or(lits[i].first, false ^ lits[i].second, lits[j].first, false ^ lits[j].second);
                // i.e., (¬li ∨ ¬lj)
            }
        }
    }

    void dfs1(int u) {
        vis[u] = 1;
        for (int v : g[u]) if (!vis[v]) dfs1(v);
        order.push_back(u);
    }

    void dfs2(int u, int c) {
        comp[u] = c;
        for (int v : gr[u]) if (comp[v] == -1) dfs2(v, c);
    }

    bool solve() {
        int m = 2 * n;
        order.clear();
        fill(vis.begin(), vis.end(), 0);
        fill(comp.begin(), comp.end(), -1);
        for (int i = 0; i < m; i++) if (!vis[i]) dfs1(i);
        int j = 0;
        for (int i = m - 1; i >= 0; i--) {
            int v = order[i];
            if (comp[v] == -1) dfs2(v, j++);
        }
        for (int x = 0; x < n; x++) if (comp[2*x] == comp[2*x ^ 1]) return false;
        for (int x = 0; x < n; x++) assignv[x] = (comp[2*x] > comp[2*x ^ 1]);
        return true;
    }

    const vector<int>& assignment() const { return assignv; }
};

/*
================================================================================
                                  DOCUMENTATION
================================================================================

Mục đích
- Gói TwoSAT tiện dụng, 0-index, API rõ ràng để mô hình nhanh.

API
- TwoSAT ts(n): n biến x ∈ {0..n-1}.
- add_or(u, ut, v, vt): (x_u==ut) ∨ (x_v==vt).
- add_imp(u, ut, v, vt): (x_u==ut) ⇒ (x_v==vt).
- add_true(u) / add_false(u): ràng buộc giá trị biến.
- add_equiv(u,ut,v,vt): tương đương logic.
- add_xor(u,ut,v,vt): XOR hai literal.
- add_at_most_one(lits): pairwise ràng buộc “tối đa một” cho danh sách literal.
- bool solve(): trả về true nếu thỏa; đọc nghiệm qua assignment().

Mẹo mô hình
- A ∨ B: add_or(a, true, b, true).
- A ⇒ B: add_imp(a, true, b, true) và add_imp(b, false, a, false).
- A = B: add_equiv(a, true, b, true).
- A ≠ B: add_xor(a, true, b, true).
- At most one trong {x1,..,xk}: add_at_most_one({(x1,true),...}).
- Ép A true/false: add_true(a) / add_false(a).

Ví dụ dùng nhanh
// int n = 3; TwoSAT ts(n);
// ts.add_or(0,true, 1,false); // x0 ∨ ¬x1
// ts.add_imp(2,true, 0,true); // x2 ⇒ x0
// if (ts.solve()) {
//   auto ans = ts.assignment();
//   // ans[i] ∈ {0,1}
// }

Độ phức tạp
- O(n + m) với Kosaraju (m là số cạnh kéo theo).
================================================================================
*/