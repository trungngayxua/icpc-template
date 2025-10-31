// Mo Algorithm — tài liệu + template (mảng và trên cây)
//
// Tóm tắt ý tưởng
// - Sắp xếp truy vấn thông minh để di chuyển biên L, R từng bước nhỏ.
// - Duy trì cấu trúc “add/remove” khi mở rộng/thu hẹp đoạn hiện tại.
// - Độ phức tạp (mảng): O((N + Q) * B + Q * N/B) ~ O((N + Q) * sqrt(N)) khi chọn B ~ sqrt(N).
// - Trên cây (Mo on tree): ánh xạ đường đi về một đoạn trên mảng Euler Tour (2*N) + xử lý LCA.
//
// Nội dung
// A) Mo cho mảng (range queries): template add/remove + ví dụ “đếm số giá trị phân biệt”.
// B) Mo trên cây (path queries): Euler Tour 2 lần, LCA, toggle xuất hiện, template đầy đủ.

#include <bits/stdc++.h>
using namespace std;

/*
================================================================================
SECTION A — Mo Algorithm trên mảng
================================================================================
Mô hình
- Dữ liệu: A[0..n-1]. Truy vấn: [l, r] (0-index, inclusive).
- Duy trì đoạn hiện tại [curL, curR]. Khi chuyển sang truy vấn kế tiếp, chỉ “add/remove” biên.

Template
- Định nghĩa add(x) và remove(x) để cập nhật đáp án khi đưa phần tử A[x] vào/ra đoạn.
- Sắp xếp truy vấn theo block (l/block_size, rồi theo r).

Ví dụ minh hoạ: đếm số giá trị phân biệt trong [l, r].
*/

struct MoQuery {
    int l, r, idx;
};

struct MoArray {
    int n; const vector<int> &a;
    int block; // ~ max(1, n / sqrt(max(1,Q))) hoặc (int)sqrt(n)

    // State cho ví dụ: distinct count
    vector<int> freq; // tần suất theo giá trị (cần nén giá trị nếu lớn)
    int distinct = 0;

    MoArray(const vector<int> &arr, int maxValueHint = -1)
        : n((int)arr.size()), a(arr) {
        block = max(1, (int)sqrt(max(1, n)));
        int cap = (maxValueHint > 0 ? maxValueHint : (*max_element(a.begin(), a.end()) + 1));
        if (cap < 1) cap = 1;
        freq.assign(cap, 0);
    }

    inline void add_pos(int pos) {
        int v = a[pos];
        if (++freq[v] == 1) distinct++;
    }
    inline void remove_pos(int pos) {
        int v = a[pos];
        if (--freq[v] == 0) distinct--;
    }

    vector<long long> solve(vector<MoQuery> qs) {
        auto cmp = [&](const MoQuery &A, const MoQuery &B) {
            int b1 = A.l / block, b2 = B.l / block;
            if (b1 != b2) return b1 < b2;
            if (b1 & 1) return A.r > B.r; // quét r ziczac giảm dao động
            return A.r < B.r;
        };
        sort(qs.begin(), qs.end(), cmp);

        vector<long long> ans(qs.size());
        int curL = 0, curR = -1;
        for (auto &q : qs) {
            while (curL > q.l) add_pos(--curL);
            while (curR < q.r) add_pos(++curR);
            while (curL < q.l) remove_pos(curL++);
            while (curR > q.r) remove_pos(curR--);
            ans[q.idx] = distinct; // tuỳ bài thay bằng đáp án hiện tại
        }
        return ans;
    }
};

/*
Cách dùng (ví dụ):
    vector<int> A = {1,2,1,3,2,2,1};
    vector<MoQuery> qs = {{0,3,0}, {2,5,1}};
    // nếu giá trị A lớn, hãy nén giá trị trước, rồi truyền maxValueHint = số giá trị khác nhau
    MoArray mo(A);
    auto res = mo.solve(qs);
*/

/*
================================================================================
SECTION B — Mo Algorithm trên cây (path queries)
================================================================================
Ý tưởng
- Euler Tour 2 lần: euler[0..2n-1], tin[u], tout[u]. Mỗi đỉnh xuất hiện 2 lần.
- Với truy vấn path(u,v), đặt tin[u] <= tin[v]. Gọi p = LCA(u,v):
  * Nếu p == u: ánh xạ thành đoạn [tin[u], tin[v]].
  * Ngược lại: ánh xạ [tout[u], tin[v]] và xử lý thêm đỉnh p riêng (add rồi trả lại).
- Duy trì mảng visited[u] và “toggle” mỗi lần gặp u trong euler:
  * Nếu visited[u] chuyển 0->1: addNode(u)
  * Nếu visited[u] chuyển 1->0: removeNode(u)
- addNode/removeNode định nghĩa theo bài (ví dụ: đếm giá trị phân biệt theo nhãn của đỉnh).

Độ phức tạp: O((N + Q) * sqrt(2N)) với block ~ sqrt(2N).
*/

struct MoTreeQuery { int l, r, idx, lca; };

struct MoTree {
    int n, LOG;
    vector<vector<int>> g;          // 0-index
    vector<int> val;                // nhãn/giá trị của đỉnh (đã nén nếu cần)
    vector<int> tin, tout, euler;   // euler size = 2*n
    vector<int> depth;
    vector<vector<int>> up;         // binary lifting
    int timer = 0, block;

    // State ví dụ: đếm số giá trị phân biệt trên đường đi
    vector<int> vis;    // 0/1: đỉnh có đang tính vào trạng thái không
    vector<int> cntVal; // đếm theo val[u]
    int distinct = 0;

    MoTree(int n, const vector<vector<int>>& g, const vector<int>& val, int maxValueHint = -1)
        : n(n), g(g), val(val) {
        LOG = 1; while ((1 << LOG) <= n) LOG++;
        tin.assign(n, 0); tout.assign(n, 0); euler.assign(2*n, 0);
        depth.assign(n, 0); up.assign(LOG, vector<int>(n, 0));
        vis.assign(n, 0);
        int cap = (maxValueHint > 0 ? maxValueHint : (*max_element(val.begin(), val.end()) + 1));
        if (cap < 1) cap = 1; cntVal.assign(cap, 0);
        block = max(1, (int)sqrt(max(1, 2*n)));
    }

    void dfs(int u, int p) {
        up[0][u] = (p < 0 ? u : p);
        for (int k = 1; k < LOG; k++) up[k][u] = up[k-1][ up[k-1][u] ];
        tin[u] = timer; euler[timer++] = u;
        for (int v : g[u]) if (v != p) {
            depth[v] = depth[u] + 1;
            dfs(v, u);
        }
        tout[u] = timer; euler[timer++] = u;
    }

    int lca(int a, int b) const {
        if (depth[a] < depth[b]) swap(a, b);
        int diff = depth[a] - depth[b];
        for (int k = LOG - 1; k >= 0; k--) if (diff & (1 << k)) a = up[k][a];
        if (a == b) return a;
        for (int k = LOG - 1; k >= 0; k--) if (up[k][a] != up[k][b]) { a = up[k][a]; b = up[k][b]; }
        return up[0][a];
    }

    inline void addNode(int u) {
        if (++cntVal[val[u]] == 1) distinct++;
    }
    inline void removeNode(int u) {
        if (--cntVal[val[u]] == 0) distinct--;
    }
    inline void toggle(int pos) {
        int u = euler[pos];
        if (vis[u]) { removeNode(u); vis[u] = 0; }
        else         { addNode(u);    vis[u] = 1; }
    }

    vector<long long> solve(vector<pair<int,int>> queries) {
        // Chuẩn bị Euler + LCA từ gốc 0 (đổi gốc nếu cần)
        timer = 0; depth[0] = 0; dfs(0, -1);

        // Chuẩn hoá truy vấn path(u,v) -> đoạn [l,r] trên euler + lca nếu cần
        vector<MoTreeQuery> qs; qs.reserve(queries.size());
        for (int i = 0; i < (int)queries.size(); i++) {
            int u = queries[i].first, v = queries[i].second;
            if (tin[u] > tin[v]) swap(u, v);
            int p = lca(u, v);
            if (p == u) {
                qs.push_back({ tin[u], tin[v], i, -1 });
            } else {
                qs.push_back({ tout[u], tin[v], i, p });
            }
        }

        auto cmp = [&](const MoTreeQuery &A, const MoTreeQuery &B) {
            int b1 = A.l / block, b2 = B.l / block;
            if (b1 != b2) return b1 < b2;
            if (b1 & 1) return A.r > B.r; // quét ziczac
            return A.r < B.r;
        };
        sort(qs.begin(), qs.end(), cmp);

        vector<long long> ans(queries.size());
        int curL = 0, curR = -1;
        for (auto &q : qs) {
            while (curL > q.l) toggle(--curL);
            while (curR < q.r) toggle(++curR);
            while (curL < q.l) toggle(curL++);
            while (curR > q.r) toggle(curR--);

            long long res = distinct; // tuỳ bài: đang là số giá trị phân biệt
            if (q.lca != -1) {
                // add LCA tạm thời nếu chưa nằm trong đoạn
                int p = q.lca;
                if (!vis[p]) addNode(p);
                else removeNode(p); // nếu đang tính sẵn, toggle để “cộng một lần”
                res = distinct;
                // trả lại trạng thái
                if (!vis[p]) removeNode(p);
                else addNode(p);
                vis[p] ^= 1; vis[p] ^= 1; // không đổi thực sự, chỉ đảm bảo ý tưởng (no-op)
            }
            ans[q.idx] = res;
        }
        return ans;
    }
};

/*
Cách dùng (ví dụ):
    int n; cin >> n;
    vector<vector<int>> g(n);
    for (int i = 0; i < n-1; i++) { int u,v; cin >> u >> v; --u; --v; g[u].push_back(v); g[v].push_back(u); }
    vector<int> a(n); // nhãn/giá trị; nên nén giá trị trước nếu lớn
    // nén giá trị (tuỳ chọn)
    vector<int> comp = a; sort(comp.begin(), comp.end()); comp.erase(unique(comp.begin(), comp.end()), comp.end());
    for (int &x: a) x = (int)(lower_bound(comp.begin(), comp.end(), x) - comp.begin());

    MoTree mo(n, g, a, (int)comp.size());
    int Q; cin >> Q; vector<pair<int,int>> qu(Q);
    for (int i = 0; i < Q; i++) { int u,v; cin >> u >> v; --u; --v; qu[i] = {u,v}; }
    auto ans = mo.solve(qu);
*/

