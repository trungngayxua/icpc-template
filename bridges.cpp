// Cầu (bridges) và đỉnh khớp (articulation points) — facts + helpers (0-index)
//
// Kiến thức nhanh
// - Cầu: cạnh mà nếu bỏ đi làm tăng số thành phần liên thông (không nằm trên chu trình).
// - Đỉnh khớp: đỉnh mà nếu bỏ đi làm tăng số thành phần liên thông.
// - Tarjan thấp-nhất (lowlink): DFS với thời điểm vào tin[u], giá trị thấp nhất low[u].
//   * Với cạnh cây u -> v: nếu low[v] > tin[u] thì (u,v) là cầu.
//   * Nếu low[v] >= tin[u] và u không phải gốc DFS, u là đỉnh khớp. Gốc là đỉnh khớp nếu có >= 2 con DFS.
// - 2-edge-connected components (2ECC): nén các đỉnh bằng cách “dính” qua cạnh không phải cầu.
//   Cây cầu (bridge tree) có mỗi cạnh tương ứng với một cầu của đồ thị gốc.
// - 2-vertex-connected (BCC) và block-cut tree: tương tự nhưng dùng ngăn xếp cạnh; không triển khai ở đây.
//
// Cài đặt bên dưới:
// - BridgeFinder: thêm cạnh vô hướng (u,v), tìm cầu + đỉnh khớp.
// - build_2ecc_tree(): dựng thành phần 2ECC và cây cầu.
// - Tất cả 0-index, hỗ trợ đa cạnh (qua id cạnh).

#include <bits/stdc++.h>
using namespace std;

struct BridgeFinder {
    int n, timer = 0, edge_cnt = 0;
    vector<vector<pair<int,int>>> g; // (to, edge_id)
    vector<int> tin, low;
    vector<char> vis, is_art, is_bridge;
    vector<pair<int,int>> edges, bridges; // edges by id; bridges list

    explicit BridgeFinder(int n = 0) { init(n); }

    void init(int n_) {
        n = n_;
        g.assign(n, {});
        tin.assign(n, -1);
        low.assign(n, -1);
        vis.assign(n, 0);
        is_art.assign(n, 0);
        edges.clear();
        bridges.clear();
        is_bridge.clear();
        timer = 0; edge_cnt = 0;
    }

    int add_edge(int u, int v) {
        int id = edge_cnt++;
        if ((int)is_bridge.size() < edge_cnt) is_bridge.push_back(0);
        if ((int)edges.size() < edge_cnt) edges.push_back({u, v});
        else edges[id] = {u, v};
        g[u].push_back({v, id});
        g[v].push_back({u, id});
        return id;
    }

    void dfs(int u, int pe = -1) {
        vis[u] = 1;
        tin[u] = low[u] = timer++;
        int child = 0;
        for (auto [v, id] : g[u]) {
            if (id == pe) continue;
            if (vis[v]) {
                // back-edge
                low[u] = min(low[u], tin[v]);
            } else {
                dfs(v, id);
                low[u] = min(low[u], low[v]);
                if (low[v] > tin[u]) {
                    is_bridge[id] = 1;
                    bridges.push_back({u, v});
                }
                if (pe != -1 && low[v] >= tin[u]) is_art[u] = 1;
                child++;
            }
        }
        if (pe == -1 && child > 1) is_art[u] = 1;
    }

    void run() {
        for (int i = 0; i < n; i++) if (!vis[i]) dfs(i, -1);
    }
};

// Nén thành phần 2-edge-connected (bỏ qua các cầu), dựng cây cầu.
struct BridgeTree {
    int comp_cnt = 0;                // số node trong cây cầu
    vector<int> comp;                // comp[u] ∈ [0..comp_cnt-1]
    vector<vector<int>> tree;        // cây cầu (đơn, vô hướng)
};

BridgeTree build_2ecc_tree(const BridgeFinder& bf) {
    int n = bf.n;
    BridgeTree bt;
    bt.comp.assign(n, -1);
    vector<vector<pair<int,int>>> g = bf.g;
    const auto& is_bridge = bf.is_bridge;

    // DFS gán comp, bỏ qua cạnh là cầu
    function<void(int,int)> dfs2 = [&](int u, int cid) {
        bt.comp[u] = cid;
        for (auto [v, id] : g[u]) if (bt.comp[v] == -1 && !is_bridge[id]) dfs2(v, cid);
    };

    int cid = 0;
    for (int i = 0; i < n; i++) if (bt.comp[i] == -1) dfs2(i, cid++);
    bt.comp_cnt = cid;
    bt.tree.assign(bt.comp_cnt, {});

    // thêm cạnh cây cho mỗi cầu
    for (int id = 0; id < bf.edge_cnt; id++) if (is_bridge[id]) {
        auto [u, v] = bf.edges[id];
        int a = bt.comp[u], b = bt.comp[v];
        if (a == b) continue;
        bt.tree[a].push_back(b);
        bt.tree[b].push_back(a);
    }
    return bt;
}

/*
Sử dụng nhanh
- Tạo BridgeFinder bf(n); gọi add_edge(u,v) cho mọi cạnh; bf.run();
- Cầu: bf.bridges (danh sách (u,v)), bf.is_bridge[id] theo id cạnh.
- Đỉnh khớp: bf.is_art[u] (bool).
- Cây cầu/2ECC: BridgeTree bt = build_2ecc_tree(bf);
    + bt.comp[u]: id thành phần 2ECC của đỉnh u.
    + bt.tree: cây cầu (đồ thị nén thành cây theo các cầu).

Bài toán mẫu (recipes)
- Đếm số cầu, đỉnh khớp: duyệt bf.is_bridge / bf.is_art.
- Truy vấn trên thành phần 2ECC: nén bằng bt.comp, chạy trên bt.tree (cây).
- Đếm thành phần liên thông sau khi xóa k cạnh: mỗi cầu bị xóa tăng số thành phần.
- Kiểm tra cạnh có nằm trên chu trình không: cạnh không phải cầu ⇔ có chu trình.
*/
