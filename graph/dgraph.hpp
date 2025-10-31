// Directed Graph Template: SCC, condensation DAG, topo, BFS, Dijkstra, 0-1 BFS, Bellman-Ford, Euler trail
// -------------------------------------------------------------------------------------------------
// Encapsulates:
// - Directed adjacency (unweighted and weighted)
// - Reverse adjacency for SCC
// - Edge list with ids for Euler trail (directed Hierholzer)
// Utilities:
// - BFS (single/multi-source)
// - Topological sort (Kahn) + cycle detection
// - Strongly Connected Components (Kosaraju, iterative) and condensation DAG
// - Dijkstra (non-negative), 0-1 BFS, Bellman-Ford (negative edges and cycle marking)
// - Euler trail/cycle in directed graphs
// -------------------------------------------------------------------------------------------------

#pragma once
#include <bits/stdc++.h>
using namespace std;

struct DGraph {
  int n = 0;
  // Unweighted adjacency (directed)
  vector<vector<int>> adj, radj; // 1..n

  // For Euler trail (edge ids)
  struct DEdge { int u, v; };
  vector<vector<pair<int,int>>> eAdj; // (to, edgeId) for directed edges
  vector<DEdge> edges;                // 0..m-1

  // Weighted adjacency for shortest paths
  vector<vector<pair<int,int>>> wadj; // (to, weight)
  struct WEdge { int u, v, w; };
  vector<WEdge> wEdges;               // list of weighted edges (for Bellman-Ford)

  // Reachability (SCC DAG transitive closure) - stored internally
  mutable bool _reach_ready = false;
  mutable vector<int> _comp;                  // comp[u] in 1.._cc
  mutable int _cc = 0;                        // number of components
  mutable vector<vector<unsigned long long>> _reach; // bitset per comp (1.._cc)

  void init(int N) {
    n = N;
    adj.assign(n+1, {});
    radj.assign(n+1, {});
    eAdj.assign(n+1, {});
    wadj.assign(n+1, {});
    edges.clear();
    wEdges.clear();
  }

  inline void add_edge(int u, int v) {
    adj[u].push_back(v);
    radj[v].push_back(u);
    int id = (int)edges.size();
    edges.push_back({u,v});
    eAdj[u].push_back({v,id});
  }

  inline void add_edge_w(int u, int v, int w) {
    wadj[u].push_back({v,w});
    wEdges.push_back({u,v,w});
  }

  // BFS single-source; returns (dist, parent)
  pair<vector<int>, vector<int>> bfs(int s) const {
    vector<int> dist(n+1, -1), par(n+1, 0);
    queue<int> q; q.push(s); dist[s]=0;
    while(!q.empty()){
      int u=q.front(); q.pop();
      for(int v: adj[u]) if(dist[v]==-1){ dist[v]=dist[u]+1; par[v]=u; q.push(v);}    
    }
    return {dist, par};
  }

  // Multi-source BFS
  vector<int> bfs_multi(const vector<int>& sources) const {
    vector<int> dist(n+1, -1);
    queue<int> q; for(int s: sources){ if(s>=1&&s<=n && dist[s]==-1){ dist[s]=0; q.push(s);} }
    while(!q.empty()){
      int u=q.front(); q.pop();
      for(int v: adj[u]) if(dist[v]==-1){ dist[v]=dist[u]+1; q.push(v);}    
    }
    return dist;
  }

  // Kahn's algorithm for topological sort. Returns {isDAG, order}.
  pair<bool, vector<int>> topo_sort() const {
    vector<int> indeg(n+1, 0);
    for (int u = 1; u <= n; ++u) for (int v : adj[u]) ++indeg[v];
    queue<int> q; for (int u = 1; u <= n; ++u) if (indeg[u] == 0) q.push(u);
    vector<int> ord; ord.reserve(n);
    while (!q.empty()) {
      int u = q.front(); q.pop(); ord.push_back(u);
      for (int v : adj[u]) if (--indeg[v] == 0) q.push(v);
    }
    bool isDAG = ((int)ord.size() == n);
    return {isDAG, ord};
  }

  // Kosaraju SCC (iterative). Returns (compId, compCount). compId in 1..compCount
  pair<vector<int>, int> scc() const {
    vector<char> vis(n+1, 0);
    vector<int> order; order.reserve(n);
    // 1st pass: order by finish time (iterative)
    for (int s = 1; s <= n; ++s) if (!vis[s]) {
      vector<pair<int,int>> st; // (u, state) state 0=enter,1=exit
      st.emplace_back(s, 0);
      while (!st.empty()) {
        auto [u, stt] = st.back(); st.pop_back();
        if (stt == 0) {
          if (vis[u]) continue;
          vis[u] = 1;
          st.emplace_back(u, 1);
          for (int v : adj[u]) if (!vis[v]) st.emplace_back(v, 0);
        } else {
          order.push_back(u);
        }
      }
    }
    // 2nd pass: reverse graph
    vector<int> comp(n+1, 0); int cid = 0;
    for (int i = (int)order.size() - 1; i >= 0; --i) {
      int s = order[i]; if (comp[s]) continue;
      ++cid; vector<int> st; st.push_back(s); comp[s] = cid;
      while (!st.empty()) {
        int u = st.back(); st.pop_back();
        for (int v : radj[u]) if (!comp[v]) { comp[v] = cid; st.push_back(v); }
      }
    }
    return {comp, cid};
  }

  // Build condensation DAG of SCCs (1..cc). Multiple edges preserved (dedup optional by caller).
  vector<vector<int>> condensation_dag(const vector<int>& comp, int cc) const {
    vector<vector<int>> dag(cc + 1);
    for (int u = 1; u <= n; ++u) {
      int cu = comp[u];
      for (int v : adj[u]) {
        int cv = comp[v];
        if (cu != cv) dag[cu].push_back(cv);
      }
    }
    return dag;
  }

  // Dijkstra on directed graph (non-negative weights)
  vector<long long> dijkstra(int s) const {
    const long long INF = (1LL<<62);
    vector<long long> dist(n+1, INF);
    priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<pair<long long,int>>> pq;
    dist[s]=0; pq.push({0,s});
    while(!pq.empty()){
      auto [d,u]=pq.top(); pq.pop(); if(d!=dist[u]) continue;
      for(auto [v,w]: wadj[u]) if(dist[v] > d + w){ dist[v]=d+w; pq.push({dist[v], v}); }
    }
    return dist;
  }

  // 0-1 BFS (edge weights 0 or 1) on wadj
  vector<int> zero_one_bfs(int s) const {
    const int INF = 1e9;
    vector<int> dist(n+1, INF);
    deque<int> dq; dist[s]=0; dq.push_back(s);
    while(!dq.empty()){
      int u = dq.front(); dq.pop_front();
      for (auto [v, w] : wadj[u]) {
        int nd = dist[u] + w;
        if (nd < dist[v]) { dist[v] = nd; if (w == 0) dq.push_front(v); else dq.push_back(v); }
      }
    }
    return dist;
  }

  // Bellman-Ford: returns (dist, negCycle) where negCycle[v]=1 if v is reachable from some negative cycle reachable from s
  pair<vector<long long>, vector<char>> bellman_ford(int s) const {
    const long long INF = (1LL<<62);
    vector<long long> dist(n+1, INF);
    dist[s] = 0;
    for (int it = 1; it <= n - 1; ++it) {
      bool any = false;
      for (auto &e : wEdges) if (dist[e.u] != INF && dist[e.u] + e.w < dist[e.v]) {
        dist[e.v] = dist[e.u] + e.w; any = true;
      }
      if (!any) break;
    }
    vector<char> neg(n+1, 0);
    queue<int> q;
    for (auto &e : wEdges) if (dist[e.u] != INF && dist[e.u] + e.w < dist[e.v]) { if (!neg[e.v]) { neg[e.v] = 1; q.push(e.v); } }
    // propagate negativity along outgoing edges
    while (!q.empty()) {
      int u = q.front(); q.pop();
      for (auto [v, w] : wadj[u]) if (!neg[v]) { neg[v] = 1; q.push(v); }
    }
    return {dist, neg};
  }

  // Euler trail/cycle for directed graphs. Returns sequence of vertices, or empty if not possible.
  vector<int> euler_trail(int start = 1) const {
    int m = (int)edges.size();
    if (m == 0) return {start};
    vector<int> indeg(n+1, 0), outdeg(n+1, 0);
    for (auto &e : edges) { ++outdeg[e.u]; ++indeg[e.v]; }
    int startCand = -1, endCand = -1;
    for (int i = 1; i <= n; ++i) {
      if (outdeg[i] == indeg[i] + 1) { if (startCand == -1) startCand = i; else return {}; }
      else if (indeg[i] == outdeg[i] + 1) { if (endCand == -1) endCand = i; else return {}; }
      else if (indeg[i] != outdeg[i]) return {};
    }
    int s = (startCand != -1) ? startCand : start;
    if (outdeg[s] == 0) {
      // pick any node with outgoing edge
      for (int i = 1; i <= n; ++i) if (outdeg[i]) { s = i; break; }
    }
    // Hierholzer
    vector<char> used(m, 0);
    vector<int> it(n+1, 0), st; st.push_back(s);
    vector<int> path; path.reserve(m + 1);
    while (!st.empty()) {
      int u = st.back();
      while (it[u] < (int)eAdj[u].size() && used[eAdj[u][it[u]].second]) ++it[u];
      if (it[u] == (int)eAdj[u].size()) { path.push_back(u); st.pop_back(); }
      else { auto [v, id] = eAdj[u][it[u]++]; if (used[id]) continue; used[id] = 1; st.push_back(v); }
    }
    if ((int)path.size() != m + 1) return {}; // disconnected or invalid
    reverse(path.begin(), path.end());
    return path;
  }
};
