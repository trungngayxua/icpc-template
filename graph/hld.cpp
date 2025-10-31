/*
===============================================================================
                          Heavy-Light Decomposition (HLD)
===============================================================================

PURPOSE:
    - Phân rã cây thành các "heavy path" và ánh xạ node → index tuyến tính.
    - Chỉ làm đúng chức năng decomposition, không chứa Segment Tree.

FEATURES:
    - Cấu trúc OOP sạch: private data, public interface.
    - Hỗ trợ iterate theo path(u,v) và subtree(u).
    - Dễ tích hợp với backend (Segment Tree, Li Chao Tree, BIT...).

TIME COMPLEXITY:
    - build(): O(N)
    - processPath(u,v): O(log N) đoạn
===============================================================================
*/

class HLD {
private:
    int n;
    vector<vector<int>> adj;
    vector<int> parent, depth, heavy, head, pos, sz;
    int curPos = 0;

    // DFS 1: tính kích thước và chọn heavy child
    int dfs(int u, int p) {
        parent[u] = p;
        sz[u] = 1;
        int maxSub = 0;
        for (int v : adj[u]) {
            if (v == p) continue;
            depth[v] = depth[u] + 1;
            int sub = dfs(v, u);
            sz[u] += sub;
            if (sub > maxSub) {
                maxSub = sub;
                heavy[u] = v;
            }
        }
        return sz[u];
    }

    // DFS 2: gán head và pos
    void decompose(int u, int h) {
        head[u] = h;
        pos[u] = curPos++;
        if (heavy[u] != -1)
            decompose(heavy[u], h);
        for (int v : adj[u]) {
            if (v == parent[u] || v == heavy[u]) continue;
            decompose(v, v);
        }
    }

public:
    // Constructor
    explicit HLD(int n)
        : n(n),
          adj(n + 1),
          parent(n + 1),
          depth(n + 1),
          heavy(n + 1, -1),
          head(n + 1),
          pos(n + 1),
          sz(n + 1) {}

    // Add edge (1-indexed)
    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    // Build decomposition from root
    void build(int root = 1) {
        curPos = 0;
        dfs(root, -1);
        decompose(root, root);
    }

    // Process a path (u,v), call callback(L,R) for each contiguous segment
    template <typename F>
    void processPath(int u, int v, F callback) const {
        while (head[u] != head[v]) {
            if (depth[head[u]] < depth[head[v]]) swap(u, v);
            callback(pos[head[u]], pos[u]); // đoạn [L,R]
            u = parent[head[u]];
        }
        if (depth[u] > depth[v]) swap(u, v);
        callback(pos[u], pos[v]); // đoạn cuối [L,R]
    }

    // Process a subtree of u, call callback(L,R)
    template <typename F>
    void processSubtree(int u, F callback) const {
        callback(pos[u], pos[u] + sz[u] - 1);
    }
};

/*
===============================================================================
                                DOCUMENTATION
===============================================================================

🔹 THUỘC TÍNH:
    head[u]  : đỉnh đầu của heavy path chứa u
    pos[u]   : vị trí của u trong mảng flattened
    parent[u]: cha của u
    depth[u] : độ sâu trong cây
    heavy[u] : con nặng nhất
    sz[u]    : kích thước subtree(u)

===============================================================================
🔹 PHƯƠNG THỨC CÔNG KHAI:

  addEdge(u, v)
      Thêm cạnh vô hướng giữa u và v.

  build(root = 1)
      Chạy decomposition, flatten cây bắt đầu từ root.

===============================================================================
🔹 ĐỘ PHỨC TẠP:
    Build       : O(N)
    Path query  : O(log N) đoạn
    Subtree map : O(1)
===============================================================================
*/

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n = 9;
    HLD hld(n);
    vector<pair<int,int>> edges = {
        {1,2},{1,3},{2,4},{2,5},{3,6},{3,7},{6,8},{6,9}
    };
    for (auto [u,v]: edges) hld.addEdge(u,v);

    hld.build(1);

    cout << "pos: ";
    for (int i = 1; i <= n; i++) cout << hld.getPos(i) << ' ';
    cout << "\nhead: ";
    for (int i = 1; i <= n; i++) cout << hld.getHead(i) << ' ';
    cout << "\n";

    cout << "Path(4,9): ";
    hld.processPath(4,9, [&](int l,int r){
        cout << "["<<l<<","<<r<<"] ";
    });
    cout << "\n";
}