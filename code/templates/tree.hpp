// Giant Tree Template: HLD + LCA + Utilities
// ------------------------------------------------------------
// Features
// - Encapsulates all common arrays: parent, depth, size, heavy, head, pos, inv,
//   tin/tout, up (binary lifting), adjacency list
// - Non-recursive preprocessing (BFS + iterative heavy-light decomposition)
// - Binary-lifting LCA, kth ancestor, distance, kth on path
// - Path decomposition helper (node-weight or edge-weight)
// - Subtree range [pos[u], pos[u]+sz[u]-1] for segment/fenwick
// - Built-in generic SegTree for node-weight data via Monoid
// - Hooks/skeletons for DSU-on-tree and centroid decomposition
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
  // Generic segment tree (Monoid-based) over HLD base-array.
  // Monoid must provide:
  //   using T = ...;
  //   static T id();
  //   static T merge(const T&, const T&);
  // ----------------------------------------------------------
  template <class Monoid>
  struct HLD_SegTree {
    using T = typename Monoid::T;
    const Tree &g;
    int n;
    vector<T> st; // 1-indexed segtree

    HLD_SegTree(const Tree &g_) : g(g_), n(g_.n) { st.assign(4 * max(1, n), Monoid::id()); }

    void build_from_node_values(const vector<T> &nodeVal) {
      vector<T> base(n + 1, Monoid::id());
      for (int u = 1; u <= n; ++u) base[g.pos[u]] = nodeVal[u];
      build(1, 1, n, base);
    }
    void build(int id, int l, int r, const vector<T> &base) {
      if (l == r) { st[id] = base[l]; return; }
      int m = (l + r) >> 1;
      build(id << 1, l, m, base);
      build(id << 1 | 1, m + 1, r, base);
      st[id] = Monoid::merge(st[id << 1], st[id << 1 | 1]);
    }
    void point_set_node(int u, const T &val) { point_set(1, 1, n, g.pos[u], val); }
    void point_set(int id, int l, int r, int p, const T &val) {
      if (l == r) { st[id] = val; return; }
      int m = (l + r) >> 1;
      if (p <= m) point_set(id << 1, l, m, p, val);
      else point_set(id << 1 | 1, m + 1, r, p, val);
      st[id] = Monoid::merge(st[id << 1], st[id << 1 | 1]);
    }
    T range_query(int L, int R) const { return range_query(1, 1, n, L, R); }
    T range_query(int id, int l, int r, int L, int R) const {
      if (R < l || r < L) return Monoid::id();
      if (L <= l && r <= R) return st[id];
      int m = (l + r) >> 1;
      return Monoid::merge(range_query(id << 1, l, m, L, R), range_query(id << 1 | 1, m + 1, r, L, R));
    }

    // node-weight path query
    T query_path(int u, int v, bool edgeWeighted = false) const {
      T resL = Monoid::id(), resR = Monoid::id();
      int a = u, b = v;
      while (g.head[a] != g.head[b]) {
        if (g.depth[g.head[a]] >= g.depth[g.head[b]]) {
          resL = Monoid::merge(range_query(g.pos[g.head[a]], g.pos[a]), resL);
          a = g.parent[g.head[a]];
        } else {
          resR = Monoid::merge(range_query(g.pos[g.head[b]], g.pos[b]), resR);
          b = g.parent[g.head[b]];
        }
      }
      if (g.depth[a] > g.depth[b]) swap(a, b);
      int L = g.pos[a] + (edgeWeighted ? 1 : 0);
      if (L <= g.pos[b]) resL = Monoid::merge(range_query(L, g.pos[b]), resL);
      // merge left to right order (assumes Monoid merge is associative/commutative or path order agnostic)
      return Monoid::merge(resL, resR);
    }

    // subtree query (node-weight)
    T query_subtree(int u) const {
      auto seg = g.subtree_range(u);
      return range_query(seg.first, seg.second);
    }
  };

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

  // ----------------------------------------------------------
  // Centroid Decomposition - skeleton
  // ----------------------------------------------------------
  vector<int> cen_par, blocked;
  void centroid_init() { cen_par.assign(n + 1, 0); blocked.assign(n + 1, 0); }
  int centroid_calc_sz(int u, int p) {
    int s = 1; for (int v : adj[u]) if (v != p && !blocked[v]) s += centroid_calc_sz(v, u); return sz[u] = s;
  }
  int centroid_find(int u, int p, int total) {
    for (int v : adj[u]) if (v != p && !blocked[v]) if (sz[v] * 2 > total) return centroid_find(v, u, total);
    return u;
  }
  template <class Work>
  void centroid_build(int u, int p, Work work) {
    int total = centroid_calc_sz(u, 0);
    int c = centroid_find(u, 0, total);
    cen_par[c] = p; blocked[c] = 1; work(c);
    for (int v : adj[c]) if (!blocked[v]) centroid_build(v, c, work);
  }

  // ----------------------------------------------------------
  // Rerooting DP (generic, non-recursive)
  // Computes answer for every node as if it is the root.
  // Let T be the DP value type. Provide:
  //  - merge(T a, T b) -> T           (associative monoid op)
  //  - add_root(int u, T acc) -> T    (finalize node u given merged neighbor contributions)
  //  - apply_edge(int from, int to, T val) -> T (transform passing val across edge direction)
  // 'id' is the identity for merge.
  // Returns vector<T> ans with size n+1 (1..n).
  // Precondition: call build() first to fill parent/depth.
  // ----------------------------------------------------------
  template <class T, class Merge, class AddRoot, class ApplyEdge>
  vector<T> reroot_dp(const T& id, Merge merge, AddRoot add_root, ApplyEdge apply_edge, int Root = -1) const {
    int s = (Root == -1 ? root : Root);
    // order by BFS from s
    vector<int> order; order.reserve(n);
    queue<int> q; q.push(s);
    vector<int> par = parent; // copy (parent was set in build())
    while (!q.empty()) {
      int u = q.front(); q.pop(); order.push_back(u);
      for (int v : adj[u]) if (v != par[u]) { par[v] = u; q.push(v); }
    }

    // bottom-up: dp_down[u] = add_root(u, merge(apply_edge(child->u, dp_down[child]) for children))
    vector<T> dp_down(n + 1, id);
    for (int i = (int)order.size() - 1; i >= 0; --i) {
      int u = order[i];
      T acc = id;
      for (int v : adj[u]) if (v != par[u]) {
        T child_contrib = apply_edge(v, u, dp_down[v]);
        acc = merge(acc, child_contrib);
      }
      dp_down[u] = add_root(u, acc);
    }

    // top-down: compute ans[u] and pass contributions to children via prefix/suffix
    vector<T> ans(n + 1, id), up_contrib(n + 1, id); // up_contrib[u] is contribution from parent side to u (edge-level)
    up_contrib[s] = id;
    for (int u : order) {
      // gather children contributions
      vector<int> kids; kids.reserve((int)adj[u].size());
      for (int v : adj[u]) if (v != par[u]) kids.push_back(v);
      int k = (int)kids.size();
      vector<T> contrib(k);
      for (int i = 0; i < k; ++i) contrib[i] = apply_edge(kids[i], u, dp_down[kids[i]]);

      // prefix/suffix merges over children
      vector<T> pref(k + 1, id), suf(k + 1, id);
      for (int i = 0; i < k; ++i) pref[i + 1] = merge(pref[i], contrib[i]);
      for (int i = k - 1; i >= 0; --i) suf[i] = merge(contrib[i], suf[i + 1]);

      // answer at u merges parent contribution and all children
      T all_children = pref[k];
      ans[u] = add_root(u, merge(up_contrib[u], all_children));

      // pass to children
      for (int i = 0; i < k; ++i) {
        int v = kids[i];
        T without_i = merge(pref[i], suf[i + 1]);
        T acc_u = merge(up_contrib[u], without_i);
        T node_u_excl_i = add_root(u, acc_u);
        up_contrib[v] = apply_edge(u, v, node_u_excl_i);
      }
    }
    return ans;
  }
};
