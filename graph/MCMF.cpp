/*
================================================================================
                        Min-Cost Max-Flow (Single Engine)
================================================================================
 
Mục tiêu
- Dùng một thuật toán duy nhất (Min-Cost Max-Flow – MCMF) để giải đa số bài
  liên quan đến luồng: Max Flow/Min Cut, Matching, Assignment, Transportation,
  k đường ngắn nhất (tổng chi phí nhỏ nhất), Circulation với ràng buộc,
  Path Cover có trọng số trên DAG, Project Selection (min-cut modeling), v.v.
 
Tổng quan
- Cấu trúc hỗ trợ addEdge(u,v, cap, cost) và tính (totalFlow, totalCost).
- Khi không có chi phí (hoặc mọi cost=0), bài toán suy biến về Max Flow/Min Cut.
- Áp dụng Dijkstra với Potential (Johnson) ⇒ nhanh cho cost không âm.
  Nếu có cạnh chi phí âm, nên chạy SPFA/Bellman-Ford một lần để khởi tạo `pot`.
 
Yêu cầu ngoài lề
- Định nghĩa hằng `oo` đủ lớn (vd: 4e18) nếu bạn biên dịch riêng.
  Ví dụ: `const long long oo = (long long)4e18;`
- Chỉ số đỉnh 1..n theo template hiện tại.
 
Mô hình hóa nhanh (recipes)
- Max Flow / Min Cut:
  • Nguồn S, đích T; cạnh (u→v) với capacity; cost = 0 mọi cạnh.
  • Kết quả: `totalFlow` là max flow; min-cut = tổng capacity cạnh cắt.
 
- Edge-Disjoint Paths (k đường phân biệt cạnh):
  • Mỗi cạnh capacity=1; cost=0. Max flow từ S→T chính là số đường.
 
- Node-Disjoint Paths: tách đỉnh u thành u_in→u_out với capacity=1, cost=0;
  chuyển mọi cạnh đến u vào u_in và từ u là từ u_out.
 
- Bipartite Matching (không chi phí):
  • S→L_i (cap=1, cost=0), L_i→R_j (cap=1, cost=0 nếu có cạnh), R_j→T (cap=1).
  • Max flow = kích thước ghép cực đại. Min vertex cover = qua König.
 
- Assignment / Transportation (min tổng chi phí):
  • Giống matching nhưng đặt `cost = c(i,j)` cho cạnh L_i→R_j, cap=1 hoặc supply.
  • `totalCost` là chi phí nhỏ nhất khi đạt max flow mong muốn.
 
- Minimum Path Cover in DAG (có/không trọng số cạnh):
  • Tách mỗi đỉnh v thành v_in (trái) và v_out (phải). S→v_in (cap=1, cost=0),
    v_out→T (cap=1, cost=0). Với mỗi cạnh u→v trên DAG, nối u_in→v_out
    (cap=1, cost = w(u,v) nếu có). Gửi tối đa N đơn vị luồng.
  • Không trọng số: số đường cover tối thiểu = N − max matching.
  • Có trọng số: dùng MCMF để tối ưu tổng chi phí cover.
 
- k Shortest Edge-Disjoint Paths (tổng chi phí nhỏ nhất):
  • Mỗi cạnh có cost = trọng số; cap=1. Gửi k đơn vị luồng từ S→T.
 
- Project Selection / Min s-t Cut modeling:
  • Tạo mạng theo profit/penalty, cạnh phụ thuộc dùng capacity=INF, cost=0.
    Tổng lợi ích = Sum(profit) − maxflow.
 
- Circulation with Demands (lower/upper bounds):
  • Với mỗi cạnh e: [l_e, u_e], tách capacity về (u_e − l_e); tích lũy `balance[v]
    += l_e` tại đích và `balance[u] -= l_e` tại nguồn. Thêm siêu nguồn SS và
    siêu đích TT; nối SS→v với cap = max(balance[v],0); nối u→TT với cap =
    max(−balance[u],0). Chạy Max Flow từ SS đến TT; nếu bão hòa hết thì khả thi.
  • Phiên bản min-cost circulation: giữ chi phí trên cạnh gốc (u_e − l_e),
    thêm cạnh 0-chi phí T→S (cho phép tuần hoàn), rồi MCMF để tối thiểu hóa chi phí.
  
- Konig's Theorem :
    In any bipartite graph, the number of edges in a maximum matching equals
    the number of vertices in a minimum vertex cover
 
 
Độ phức tạp
- Mỗi augment tìm bằng Dijkstra (với potential) ~ O((m log n)) trên đồ thị còn dư;
  tổng phụ thuộc lượng luồng đẩy và cấu trúc input (thực tế CP đủ nhanh cho n≤5e3, m≤1e5).
 
Sử dụng
- Khởi tạo: `MinCostMaxFlow mf(n, S, T);`
- Thêm cạnh: `mf.addEdge(u, v, cap, cost);`
- Chạy: `auto [flow, cost] = mf.minCostMaxFlow();`
- Nếu chỉ cần max flow: đặt cost=0 cho mọi cạnh, đọc `flow`.
 
================================================================================
*/
 
#include <bits/stdc++.h>
using namespace std;
 
const long long oo = 4e18;
 
struct edge {
  long long x, y, cap, flow, cost;
};
 
struct MinCostMaxFlow {
  long long n, S, T;
  vector<vector<long long>> a;
  vector<long long> dist, prev, done, pot;
  vector<edge> e;
 
  MinCostMaxFlow() {}
  MinCostMaxFlow(long long _n, long long _S, long long _T) {
    n = _n;
    S = _S;
    T = _T;
    a = vector<vector<long long>>(n + 1);
    dist = vector<long long>(n + 1);
    prev = vector<long long>(n + 1);
    done = vector<long long>(n + 1);
    pot = vector<long long>(n + 1, 0);
  }
 
  void addEdge(long long x, long long y, long long _cap, long long _cost) {
    edge e1 = {x, y, _cap, 0, _cost};
    edge e2 = {y, x, 0, 0, -_cost};
    a[x].push_back(e.size());
    e.push_back(e1);
    a[y].push_back(e.size());
    e.push_back(e2);
  }
 
  pair<long long, long long> dijkstra() {
    long long flow = 0, cost = 0;
    for (long long i = 1; i <= n; i++)
      done[i] = 0, dist[i] = oo;
    priority_queue<pair<long long, long long>> q;
    dist[S] = 0;
    prev[S] = -1;
    q.push(make_pair(0, S));
    while (!q.empty()) {
      long long x = q.top().second;
      q.pop();
      if (done[x])
        continue;
      done[x] = 1;
      for (int i = 0; i < int(a[x].size()); i++) {
        long long id = a[x][i], y = e[id].y;
        if (e[id].flow < e[id].cap) {
          long long D = dist[x] + e[id].cost + pot[x] - pot[y];
          if (!done[y] && D < dist[y]) {
            dist[y] = D;
            prev[y] = id;
            q.push(make_pair(-dist[y], y));
          }
        }
      }
    }
 
    for (long long i = 1; i <= n; i++)
      pot[i] += dist[i];
 
    if (done[T]) {
      flow = oo;
      for (long long id = prev[T]; id >= 0; id = prev[e[id].x])
        flow = min(flow, e[id].cap - e[id].flow);
      for (long long id = prev[T]; id >= 0; id = prev[e[id].x]) {
        cost += e[id].cost * flow;
        e[id].flow += flow;
        e[id ^ 1].flow -= flow;
      }
    }
 
    return make_pair(flow, cost);
  }
 
  pair<long long, long long> minCostMaxFlow() {
    long long totalFlow = 0, totalCost = 0;
    while (1) {
      pair<long long, long long> u = dijkstra();
      if (!done[T])
        break;
      totalFlow += u.first;
      totalCost += u.second;
    }
    return make_pair(totalFlow, totalCost);
  }
} MCMF;