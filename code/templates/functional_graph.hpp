// Functional (Successor) Graph Template
// -----------------------------------------------------------------------------
// Each node has exactly one outgoing edge: succ[u] in [1..n].
// Features:
// - Build cycle decomposition (cycles + in-forests feeding cycles)
// - Binary lifting for k-th successor
// - For every node: dist_to_cycle[u], entry[u] (cycle node reached), cycle_id[u], pos_in_cycle[u], cycle_len[id]
// - Reachability(u,v) and distance(u,v) along successor steps
// - tin/tout over reverse-forest (for subtree/ancestor checks in predecessor trees)
// -----------------------------------------------------------------------------

#pragma once
#include <bits/stdc++.h>
using namespace std;

struct FunctionalGraph {
  static const int LOG = 60; // supports k up to 2^60
  int n = 0;
  vector<int> succ;               // 1..n, succ[u] in [1..n]
  vector<vector<int>> pred;       // predecessors (reverse edges)

  // Lifting table: up[j][u] = 2^j-th successor of u
  vector<array<int, LOG>> up;     // up[u][j]

  // Cycle + forest info
  vector<int> dist_to_cycle;      // 0 if in cycle, else positive
  vector<int> entry;              // cycle node reached from u (u after dist_to_cycle[u] steps)
  vector<int> cycle_id;           // id of cycle containing entry[u]
  vector<int> pos_in_cycle;       // position in cycle if in cycle, else position of entry[u] if needed
  vector<int> cycle_len;          // length per cycle id (1..cc)
  vector<char> in_cycle;          // 1 if node belongs to some cycle

  // Reverse-forest Euler timing (rooted at each cycle node)
  vector<int> tin, tout; int timer = 0;

  void init(int N) {
    n = N;
    succ.assign(n + 1, 0);
  }

  inline void set_succ(int u, int v) { succ[u] = v; }

  void build() {
    // Build reverse edges and indegree
    pred.assign(n + 1, {});
    vector<int> indeg(n + 1, 0);
    for (int u = 1; u <= n; ++u) { int v = succ[u]; pred[v].push_back(u); ++indeg[v]; }

    // Peel nodes not in cycles using indegree queue
    vector<char> removed(n + 1, 0);
    queue<int> q;
    for (int u = 1; u <= n; ++u) if (indeg[u] == 0) q.push(u);
    while (!q.empty()) {
      int u = q.front(); q.pop(); if (removed[u]) continue; removed[u] = 1;
      int v = succ[u]; if (--indeg[v] == 0) q.push(v);
    }

    // Identify cycles and assign ids/positions
    in_cycle.assign(n + 1, 0); cycle_id.assign(n + 1, 0); pos_in_cycle.assign(n + 1, -1);
    int cid = 0; cycle_len.clear(); cycle_len.push_back(0); // 1-based
    vector<int> seen(n + 1, 0); // timestamp per cycle-discovery
    for (int u = 1; u <= n; ++u) if (!removed[u] && !in_cycle[u]) {
      // Walk to find cycle starting from u
      unordered_map<int,int> idx; idx.reserve(64);
      int cur = u;
      while (!idx.count(cur)) { idx[cur] = (int)idx.size(); cur = succ[cur]; }
      int start = idx[cur];
      // Nodes from start..end-1 are cycle
      vector<int> cyc; cyc.reserve(idx.size() - start);
      for (auto &kv : idx) (void)kv; // no-op to silence warnings
      // Reconstruct order along the walk
      vector<int> path(idx.size());
      for (auto &kv : idx) path[kv.second] = kv.first;
      for (int i = start; i < (int)path.size(); ++i) cyc.push_back(path[i]);
      ++cid; int L = (int)cyc.size();
      if ((int)cycle_len.size() <= cid) cycle_len.resize(cid + 1);
      cycle_len[cid] = L;
      for (int i = 0; i < L; ++i) {
        int x = cyc[i]; in_cycle[x] = 1; cycle_id[x] = cid; pos_in_cycle[x] = i;
      }
    }

    // BFS from cycles over reverse edges to fill dist_to_cycle and entry/cycle_id
    dist_to_cycle.assign(n + 1, -1); entry.assign(n + 1, 0);
    queue<int> qb;
    for (int u = 1; u <= n; ++u) if (in_cycle[u]) { dist_to_cycle[u] = 0; entry[u] = u; qb.push(u); }
    while (!qb.empty()) {
      int u = qb.front(); qb.pop();
      for (int p : pred[u]) if (dist_to_cycle[p] == -1) {
        dist_to_cycle[p] = dist_to_cycle[u] + 1;
        entry[p] = entry[u];
        cycle_id[p] = cycle_id[u];
        qb.push(p);
      }
    }

    // Binary lifting table up[u][j]
    up.assign(n + 1, {});
    for (int u = 1; u <= n; ++u) up[u][0] = succ[u];
    for (int j = 1; j < LOG; ++j) for (int u = 1; u <= n; ++u) up[u][j] = up[ up[u][j - 1] ][j - 1];

    // tin/tout on reverse forest (ignore cycle-to-cycle reversed edges)
    tin.assign(n + 1, 0); tout.assign(n + 1, 0); timer = 0;
    vector<int> it(n + 1, 0);
    for (int root = 1; root <= n; ++root) if (in_cycle[root] && tin[root] == 0) {
      // iterative DFS limited to non-cycle nodes (except root itself)
      vector<pair<int,int>> st; st.emplace_back(root, 0);
      while (!st.empty()) {
        int u = st.back().first; int &i = st.back().second;
        if (i == 0) tin[u] = ++timer;
        auto &vec = pred[u];
        if (i < (int)vec.size()) {
          int v = vec[i++];
          if (in_cycle[v]) continue; // don't traverse into other cycle nodes
          st.emplace_back(v, 0);
        } else {
          tout[u] = timer; st.pop_back();
        }
      }
    }
  }

  inline int kth_successor(int u, unsigned long long k) const {
    for (int j = 0; j < LOG; ++j) if (k & (1ULL << j)) u = up[u][j];
    return u;
  }

  inline bool is_in_cycle(int u) const { return dist_to_cycle[u] == 0; }

  inline bool same_cycle(int u, int v) const { return cycle_id[u] == cycle_id[v] && cycle_id[u] != 0; }

  inline int cycle_length_of(int u) const { int cid = cycle_id[u]; return cid ? cycle_len[cid] : 0; }

  inline bool rev_ancestor(int anc, int v) const { return tin[anc] && tin[v] && tin[anc] <= tin[v] && tout[v] <= tout[anc]; }

  // Distance along successor edges from u to v, or -1 if unreachable
  long long distance(int u, int v) const {
    if (cycle_id[u] != cycle_id[v]) return -1;
    if (!is_in_cycle(v)) {
      // v is in tree; reachable only if u is also in tree and v lies on u->cycle path
      if (is_in_cycle(u)) return -1;
      if (!rev_ancestor(v, u)) return -1; // v is ancestor of u in reverse forest
      return (long long)dist_to_cycle[u] - dist_to_cycle[v];
    } else {
      // v is in cycle; from any node in same cycle component it's reachable
      int cid = cycle_id[v]; int L = cycle_len[cid];
      if (is_in_cycle(u)) {
        int du = pos_in_cycle[u]; int dv = pos_in_cycle[v];
        return (dv - du + L) % L;
      } else {
        int start = entry[u]; int du = dist_to_cycle[u];
        int su = pos_in_cycle[start]; int dv = pos_in_cycle[v];
        return (long long)du + (dv - su + L) % L;
      }
    }
  }

  inline bool reachable(int u, int v) const { return distance(u, v) != -1; }
};

