// ------------------------------------------------------------
// Features
// - Encapsulates all common arrays: parent, depth, size, heavy, head, pos, inv,
//   tin/tout, up (binary lifting), adjacency list
// - Non-recursive preprocessing (BFS + iterative heavy-light decomposition)
// - Binary-lifting LCA, kth ancestor, distance, kth on path
// - Path decomposition helper (node-weight or edge-weight)
// - Subtree range [pos[u], pos[u]+sz[u]-1] for segment/fenwick
// - Hooks/skeletons for DSU-on-tree
// ------------------------------------------------------------

#pragma once
#include <bits/stdc++.h>
using namespace std;

struct Tree {
  // core
  int n = 0, root = 1, LOG = 0;
  vector<vector<int>> adj;         // 1..n
  vector<int> parent, depth, sz;   // parent[1]=0
  vector<int> heavy, head;         // heavy child, head of chain
  vector<int> pos, inv;            // position in base array, inverse mapping
  vector<int> tin, tout;           // Euler entry/exit time
  vector<vector<int>> up;          // binary lifting up to LOG
  int curPos = 0, timer = 0;

  // init and edges
  void init(int N, int Root = 1) {
    n = N; root = Root;
    adj.assign(n + 1, {});
    parent.assign(n + 1, 0);
    depth.assign(n + 1, 0);
    sz.assign(n + 1, 0);
    heavy.assign(n + 1, 0);
    head.assign(n + 1, 0);
    pos.assign(n + 1, 0);
    inv.assign(n + 1, 0);
    tin.assign(n + 1, 0);
    tout.assign(n + 1, 0);
    LOG = 32 - __builtin_clz(n);
    up.assign(n + 1, vector<int>(LOG, 0));
    curPos = 0; timer = 0;
  }
  inline void add_edge(int u, int v) { adj[u].push_back(v); adj[v].push_back(u); }

  // Non-recursive preprocess: parent, depth, sizes, heavy child, head/pos, tin/tout, up[][]
  void build() {
    // 1) BFS for parent, depth, and order
    vector<int> order; order.reserve(n);
    queue<int> q; q.push(root);
    parent[root] = 0; depth[root] = 0;
    while (!q.empty()) {
      int u = q.front(); q.pop(); order.push_back(u);
      for (int v : adj[u]) if (v != parent[u]) {
        parent[v] = u; depth[v] = depth[u] + 1; q.push(v);
      }
    }

    // 2) sizes and heavy child (reverse order)
    for (int u : order) { sz[u] = 1; heavy[u] = 0; }
    for (int i = (int)order.size() - 1; i >= 0; --i) {
      int u = order[i]; int best = 0;
      for (int v : adj[u]) if (v != parent[u]) {
        sz[u] += sz[v]; if (sz[v] > best) { best = sz[v]; heavy[u] = v; }
      }
    }

    // 3) decompose to head/pos
    curPos = 0;
    for (int u = 1; u <= n; ++u) {
      if (parent[u] == 0 || heavy[parent[u]] != u) {
        for (int v = u; v; v = heavy[v]) {
          head[v] = u; pos[v] = ++curPos; inv[curPos] = v;
        }
      }
    }

    // 4) tin/tout via iterative DFS
    vector<int> it(n + 1, 0); vector<pair<int,int>> st; st.emplace_back(root, 0);
    while (!st.empty()) {
      int u = st.back().first; int p = st.back().second;
      if (it[u] == 0) tin[u] = ++timer; // entry
      if (it[u] < (int)adj[u].size()) {
        int v = adj[u][it[u]++]; if (v == p) continue; st.emplace_back(v, u);
      } else {
        tout[u] = timer; st.pop_back();
      }
    }

    // 5) binary lifting table
    for (int u = 1; u <= n; ++u) up[u][0] = parent[u];
    for (int j = 1; j < LOG; ++j)
      for (int u = 1; u <= n; ++u) up[u][j] = up[u][j-1] ? up[ up[u][j-1] ][j-1] : 0;
  }

  // Basic helpers
  inline bool is_ancestor(int u, int v) const { return tin[u] <= tin[v] && tout[v] <= tout[u]; }
  int kth_ancestor(int u, int k) const {
    for (int j = 0; j < LOG && u; ++j) if (k & (1 << j)) u = up[u][j];
    return u;
  }
  int lca(int u, int v) const {
    if (depth[u] < depth[v]) swap(u, v);
    int d = depth[u] - depth[v];
    for (int j = 0; j < LOG; ++j) if (d & (1 << j)) u = up[u][j];
    if (u == v) return u;
    for (int j = LOG - 1; j >= 0; --j)
      if (up[u][j] != up[v][j]) u = up[u][j], v = up[v][j];
    return parent[u];
  }
  inline int dist(int u, int v) const { int w = lca(u, v); return depth[u] + depth[v] - 2 * depth[w]; }
  // k-th node on path u->v (0-based: k=0 -> u, k=dist -> v)
  int kth_on_path(int u, int v, int k) const {
    int w = lca(u, v); int du = depth[u] - depth[w];
    if (k <= du) return kth_ancestor(u, k);
    int dv = depth[v] - depth[w];
    return kth_ancestor(v, du + dv - k);
  }

  // Decompose path into O(log n) base-array segments; edgeWeighted excludes LCA node on one side
  template <class F>
  void path_decompose(int u, int v, bool edgeWeighted, F &&applySeg) const {
    while (head[u] != head[v]) {
      if (depth[head[u]] < depth[head[v]]) swap(u, v);
      applySeg(pos[head[u]], pos[u]);
      u = parent[head[u]];
    }
    if (depth[u] > depth[v]) swap(u, v);
    int L = pos[u] + (edgeWeighted ? 1 : 0);
    if (L <= pos[v]) applySeg(L, pos[v]);
  }

  // subtree range in base-array
  inline pair<int,int> subtree_range(int u) const { return {pos[u], pos[u] + sz[u] - 1}; }

  // ----------------------------------------------------------
  // DSU on Tree (small-to-large) - skeleton
  // Provide your own add(u), remove(u), answer(u) lambdas.
  // ----------------------------------------------------------
  template <class Add, class Remove, class Answer>
  void dsu_on_tree(int rootNode, Add add, Remove remove, Answer answer) const {
    // This is a recursive technique normally. Skeleton below with recursion.
    // For very deep trees, convert to iterative if needed.
    function<void(int,int,bool)> dfs = [&](int u, int p, bool keep) {
      int big = heavy[u];
      for (int v : adj[u]) if (v != p && v != big) dfs(v, u, false);
      if (big) dfs(big, u, true);
      for (int v : adj[u]) if (v != p && v != big) {
        // add entire subtree of v
        function<void(int,int)> add_sub = [&](int x, int par){ add(x); for (int y: adj[x]) if (y != par && y != big) add_sub(y, x); };
        add_sub(v, u);
      }
      add(u);
      answer(u);
      if (!keep) {
        function<void(int,int)> rem_sub = [&](int x, int par){ remove(x); for (int y: adj[x]) if (y != par && y != big) rem_sub(y, x); };
        rem_sub(u, p);
      }
    };
    dfs(rootNode, 0, false);
  }
};
