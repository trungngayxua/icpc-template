// Đồ thị phẳng — kiến thức cốt lõi và “recipes” cho CP (kèm cài đặt trợ giúp)
//
// KIẾN THỨC CỐT LÕI (chi tiết)
// - Công thức Euler:
//     * Liên thông:             v - e + f = 2
//     * Có c thành phần liên thông: v - e + f = 1 + c
// - Tổng bậc các mặt:  tổng deg(F) qua mọi mặt = 2e (mỗi cạnh kề 2 mặt).
// - Cận cạnh cho đồ thị phẳng đơn (không vòng lặp/cạnh song song), v >= 3:
//     * Tổng quát (chu vi tối thiểu g >= 3):  e <= 3v - 6
//     * Không tam giác (g >= 4):             e <= 2v - 4
//     * Girth bất kỳ g:                      e <= g/(g-2) * (v - 2)
//       Suy ra từ: 2e = sum deg(F) >= g f  và Euler ⇒ khử f.
// - Đồ thị phẳng hai phía (bipartite) không có tam giác ⇒ e <= 2v - 4 (v >= 3).
// - Outerplanar: e <= 2v - 3 (và không có K4 hay K2,3 làm minor).
// - Cận bậc trung bình:
//     * Phẳng: 2e < 6v ⇒ tồn tại đỉnh bậc <= 5 (5-degenerate).
//     * Phẳng không tam giác: 2e <= 4v - 8 ⇒ tồn tại đỉnh bậc <= 3 (3-degenerate).
//   Hệ quả: đồ thị phẳng tô được ≤ 6 màu bằng greedy; phẳng không tam giác ≤ 4 màu bằng greedy.
// - Test phi phẳng nhanh:
//     * K5: v=5, e=10 > 3v-6=9  ⇒ không phẳng.
//     * K3,3: hai phía, v=6, e=9 > 2v-4=8 ⇒ không phẳng.
// - Kuratowski/Wagner: phẳng ⇔ không chứa subdivision/minor của K5 hoặc K3,3 (lý thuyết).
// - Cận tập độc lập: phẳng ⇒ 6-colorable ⇒ alpha(G) ≥ v/6 (lấy lớp màu lớn nhất).
//
// QUICK RECIPES (cách áp dụng nhanh)
// - Số cạnh tối đa (đơn, v >= 3): e_max = 3v - 6 (đạt khi tam giác hoá hoàn toàn).
// - Đếm số mặt: liên thông ⇒  f = e - v + 2;  tổng quát ⇒  f = e - v + 1 + c.
// - Hai phía/không tam giác: e_max = 2v - 4.
// - Outerplanar: e_max = 2v - 3.
// - Chứng minh tồn tại đỉnh bậc <= 5: 2e <= 3(2v - 4) < 6v  ⇒ bậc TB < 6.
// - Tô 6 màu: tìm thứ tự suy biến (degeneracy, luôn tồn tại bậc tối đa ≤ 5), greedy ngược.
// - Kiểm tra cần thiết (chưa đủ): nếu đơn và v>=3 mà e > 3v-6 (hoặc bipartite mà e > 2v-4) ⇒ không phẳng.
// - Đồ thị tam giác hoá: mọi mặt là tam giác ⇒ e = 3v - 6, f = 2v - 4.
// - Đồ thị phẳng hai phía: mọi chu trình là chẵn (không tam giác); kết hợp Euler để chặn e.
// - Tập độc lập: từ 6-coloring, lấy lớp màu lớn nhất (≥ ceil(v/6)).
//
// CÀI ĐẶT TRỢ GIÚP
// - Tô màu greedy theo suy biến (hai phiên bản):
//     * O(n^2 + m) quét đơn giản (bên dưới, hàm A)
//     * O(n + m) dùng “bucket” (bên dưới, hàm B)
//   Với đồ thị phẳng, cả hai đều dùng ≤ 6 màu do 5-degeneracy.

#include <bits/stdc++.h>
using namespace std;

// Tiện ích: các cận trên nhanh cho đồ thị phẳng đơn (điều kiện cần, không đủ)
inline long long planar_edge_ub_simple(long long v) {
    if (v <= 2) return v * (v - 1) / 2; // đồ thị đầy đủ K_v
    return 3 * v - 6;
}
inline long long planar_edge_ub_bipartite(long long v) {
    if (v <= 2) return v * (v - 1) / 2;
    return 2 * v - 4;
}
inline long long faces_connected(long long v, long long e) { return e - v + 2; }
inline long long faces_general(long long v, long long e, long long c) { return e - v + 1 + c; }

// A) O(n^2 + m) — tô màu greedy dựa trên thứ tự suy biến
// Input: n (0..n-1), danh sách kề g
// Output: colors[0..n-1] dùng nhiều nhất (degeneracy+1) màu (≤6 nếu phẳng)
vector<int> planar_greedy_coloring(int n, const vector<vector<int>>& g) {
    vector<int> deg(n), order;
    for (int i = 0; i < n; i++) deg[i] = (int)g[i].size();
    int K = 0; // suy biến (độ bão hoà lớn nhất trong quá trình loại)
    vector<char> removed(n, false);
    order.reserve(n);
    for (int it = 0; it < n; it++) {
        int u = -1, dmin = INT_MAX;
        for (int i = 0; i < n; i++) if (!removed[i] && deg[i] < dmin) {
            dmin = deg[i]; u = i;
        }
        if (u == -1) break; // hết đỉnh khả dụng
        K = max(K, dmin);
        removed[u] = true;
        order.push_back(u);
        for (int v : g[u]) if (!removed[v]) deg[v]--;
    }
    vector<int> color(n, -1);
    vector<char> used(K + 2, false); // tối đa K+1 màu
    for (int i = (int)order.size() - 1; i >= 0; i--) {
        int u = order[i];
        fill(used.begin(), used.end(), false);
        for (int v : g[u]) if (color[v] != -1 && color[v] <= K + 1) used[color[v]] = true;
        int c = 0; while (c <= K + 1 && used[c]) c++;
        color[u] = c;
    }
    return color; // nếu phẳng: K <= 5 ⇒ dùng ≤ 6 màu
}

// B) O(n + m) — tô màu greedy theo suy biến dùng “bucket”
vector<int> planar_greedy_coloring_linear(int n, const vector<vector<int>>& g) {
    vector<int> deg(n), order;
    for (int i = 0; i < n; i++) deg[i] = (int)g[i].size();
    int maxd = 0; for (int d : deg) maxd = max(maxd, d);
    vector<vector<int>> bucket(max(maxd + 1, n + 1));
    for (int i = 0; i < n; i++) bucket[deg[i]].push_back(i);
    vector<char> removed(n, false);
    int K = 0, cur = 0, picked = 0;
    order.reserve(n);
    while (picked < n) {
        while (cur < (int)bucket.size() && bucket[cur].empty()) cur++;
        // chọn u có bậc hiện tại == cur
        int u = -1;
        while (cur < (int)bucket.size() && !bucket[cur].empty()) {
            int v = bucket[cur].back(); bucket[cur].pop_back();
            if (!removed[v] && deg[v] == cur) { u = v; break; }
        }
        if (u == -1) { cur++; continue; }
        K = max(K, deg[u]);
        removed[u] = true; picked++; order.push_back(u);
        for (int v : g[u]) if (!removed[v]) {
            int dv = deg[v];
            if (dv > 0) { deg[v] = dv - 1; bucket[dv - 1].push_back(v); }
        }
    }
    vector<int> color(n, -1); vector<char> used(K + 2, false);
    for (int i = n - 1; i >= 0; i--) {
        int u = order[i]; fill(used.begin(), used.end(), false);
        for (int v : g[u]) if (color[v] != -1 && color[v] <= K + 1) used[color[v]] = true;
        int c = 0; while (c <= K + 1 && used[c]) c++;
        color[u] = c;
    }
    return color; // đồ thị phẳng ⇒ ≤ 6 màu
}
