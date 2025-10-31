// Centroid Decomposition — code gọn, rõ ràng (VN)
//
// Key Features
// - Mỗi lần tách ở centroid c, mọi thành phần còn lại có kích thước ≤ n/2 → chiều cao cây centroid O(log N).
// - Mỗi đỉnh/cạnh tham gia O(log N) mức xử lý → tổng độ phức tạp thường O(N log N) cho các bài đếm theo khoảng cách.
// - Lưu sẵn distToCent[u][lvl]: khoảng cách u → centroid ở tầng lvl (tái sử dụng cho nhiều bài, đặc biệt dynamic).
// - Cây centroid (parent[], level[]) độc lập với gốc ban đầu; có thể làm “nền” cho nhiều cấu trúc phụ.
//
// Hệ Quả Đặc Biệt (thực dụng)
// - Đếm cặp theo khoảng cách (<=K, =K) trong O(N log N) bằng kỹ thuật “gộp theo nhánh quanh centroid”.
// - Bài nearest marked (add-only) chạy O(log N) mỗi truy vấn (đi theo chuỗi tổ tiên centroid).
// - Các truy vấn là hàm cộng dồn theo dist (sum/min/max) thường quy được về gộp theo nhánh + tránh double count.
// - Độ cao nhỏ → có thể gắn cấu trúc cho từng centroid (Fenwick, vector tần suất, map…) mà vẫn đảm bảo logN mức cập nhật.
//
// Key Information (lưu ý triển khai)
// - 0-index toàn bộ; yêu cầu g là cây (n-1 cạnh, vô hướng).
// - Khi build CD, “removed” giữ nguyên (không unmark) — chỉ dùng cho quá trình phân rã.
// - Với đếm cặp: luôn khởi đầu “all = {0}” tại centroid; gộp nhánh theo thứ tự, đếm giao giữa nhánh mới và all rồi mới hợp nhất.
// - Tránh double-count: chỉ đếm cặp bắt chéo nhánh (không đếm trong-nhánh tại bước centroid, vì sẽ được xử lý ở đệ quy).
// - Đệ quy sâu: cân nhắc tăng stack hoặc viết lại BFS stack nếu môi trường hạn chế.
// - Bộ nhớ distToCent ~ O(N log N) (mỗi u có ≤ chiều cao centroid entries).

#include <bits/stdc++.h>
using namespace std;

struct CentroidDecomposition {
    int n;
    vector<vector<int>> g;
    vector<int> parent, level, sz;
    vector<vector<int>> distToCent; // distToCent[u][level]
    vector<char> removed;

    CentroidDecomposition() {}
    explicit CentroidDecomposition(const vector<vector<int>>& G) { build(G); }

    void build(const vector<vector<int>>& G) {
        g = G; n = (int)g.size();
        parent.assign(n, -1);
        level.assign(n, -1);
        sz.assign(n, 0);
        removed.assign(n, 0);
        distToCent.assign(n, {});
        decompose(0, -1, 0);
    }

    int dfs_sz(int u, int p) {
        sz[u] = 1;
        for (int v : g[u]) if (v != p && !removed[v]) sz[u] += dfs_sz(v, u);
        return sz[u];
    }

    int find_centroid(int u, int p, int tot) {
        for (int v : g[u]) if (v != p && !removed[v]) if (sz[v] * 2 > tot) return find_centroid(v, u, tot);
        return u;
    }

    void fill_dist(int u, int p, int d, int L) {
        if ((int)distToCent[u].size() <= L) distToCent[u].resize(L + 1, 0);
        distToCent[u][L] = d;
        for (int v : g[u]) if (v != p && !removed[v]) fill_dist(v, u, d + 1, L);
    }

    void decompose(int root, int p, int L) {
        int tot = dfs_sz(root, -1);
        int c = find_centroid(root, -1, tot);
        parent[c] = p; level[c] = L; removed[c] = 1;
        fill_dist(c, -1, 0, L);
        for (int v : g[c]) if (!removed[v]) decompose(v, c, L + 1);
        // giữ removed[c] = 1; chỉ dùng khi build
    }
};

// Đếm số cặp dist<=K và dist==K bằng CD
struct CDPairCounter {
    int n, K;
    const vector<vector<int>>& g;
    vector<int> sz; vector<char> removed;
    long long atMost = 0, equalK = 0;

    CDPairCounter(const vector<vector<int>>& G, int K) : n((int)G.size()), K(K), g(G) {
        sz.assign(n, 0); removed.assign(n, 0);
    }

    int dfs_sz(int u, int p) {
        sz[u] = 1;
        for (int v : g[u]) if (v != p && !removed[v]) sz[u] += dfs_sz(v, u);
        return sz[u];
    }
    int find_centroid(int u, int p, int tot) {
        for (int v : g[u]) if (v != p && !removed[v]) if (sz[v] * 2 > tot) return find_centroid(v, u, tot);
        return u;
    }
    void collect(int u, int p, int d, vector<int>& out) {
        if (d > K) return;
        out.push_back(d);
        for (int v : g[u]) if (v != p && !removed[v]) collect(v, u, d + 1, out);
    }
    static long long count_leq(vector<int>& A, vector<int>& B, int K) {
        sort(A.begin(), A.end()); sort(B.begin(), B.end());
        long long r = 0; int j = (int)B.size() - 1;
        for (int i = 0; i < (int)A.size(); i++) { while (j >= 0 && A[i] + B[j] > K) j--; r += j + 1; }
        return r;
    }
    static long long count_eq(vector<int>& A, vector<int>& B, int K) {
        sort(A.begin(), A.end()); sort(B.begin(), B.end());
        long long r = 0; int i = 0, j = (int)B.size() - 1;
        while (i < (int)A.size() && j >= 0) {
            int s = A[i] + B[j];
            if (s < K) i++;
            else if (s > K) j--;
            else { int a = A[i], b = B[j]; long long ca = 0, cb = 0; while (i < (int)A.size() && A[i] == a) { ca++; i++; } while (j >= 0 && B[j] == b) { cb++; j--; } r += ca * cb; }
        }
        return r;
    }
    void solve_centroid(int c) {
        vector<int> all = {0};
        for (int v : g[c]) if (!removed[v]) {
            vector<int> tmp; collect(v, c, 1, tmp);
            if (!tmp.empty()) {
                atMost += count_leq(all, tmp, K);
                equalK += count_eq(all, tmp, K);
                vector<int> merged; merged.reserve(all.size() + tmp.size());
                merge(all.begin(), all.end(), tmp.begin(), tmp.end(), back_inserter(merged));
                all.swap(merged);
            }
        }
    }
    void decompose(int u) {
        int tot = dfs_sz(u, -1);
        int c = find_centroid(u, -1, tot);
        solve_centroid(c);
        removed[c] = 1;
        for (int v : g[c]) if (!removed[v]) decompose(v);
    }
    pair<long long,long long> run() { decompose(0); return {atMost, equalK}; }
};