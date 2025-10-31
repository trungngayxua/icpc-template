// Undirected Graph Template: connectivity, bridges, BCC (edge), bipartite, Euler, MST, Dijkstra
// -----------------------------------------------------------------------------
// Encapsulates:
// - Adjacency (unweighted)
// - Edge list with ids for Euler trail (Hierholzer)
// - Weighted edges & adjacency for Dijkstra/MST
// Utilities:
// - BFS (single/multi-source), connected components
// - Bipartite check (2-coloring)
// - Bridges and articulation points (iterative low-link)
// - 2-edge-connected components and bridge tree construction
// - Euler trail/cycle (if exists)
// - Dijkstra (non-negative weights), Kruskal MST
// -----------------------------------------------------------------------------

#pragma once
#include <bits/stdc++.h>
using namespace std;

struct DSU {
  int n, comps;
  vector<int> p, r;
  DSU(): n(0), comps(0) {}
  void init(int N) { n = comps = N; p.resize(N+1); r.assign(N+1,0); iota(p.begin(), p.end(), 0); }
  int find(int x){ return p[x]==x?x:p[x]=find(p[x]); }
  bool unite(int a, int b){ a=find(a); b=find(b); if(a==b) return false; if(r[a]<r[b]) swap(a,b); p[b]=a; if(r[a]==r[b]) r[a]++; comps--; return true; }
};

struct UGraph {
  int n = 0;
  // Unweighted adjacency
  vector<vector<int>> adj;              // 1..n

  // For Euler trail (edge ids)
  struct EEdge { int u, v; };
  vector<vector<pair<int,int>>> eAdj;   // (to, edgeId)
  vector<EEdge> edges;                  // 0..m-1

  // Weighted adjacency / edges for Dijkstra & MST
  vector<vector<pair<int,int>>> wadj;   // (to, weight)
  struct WEdge { int u, v, w; };
  vector<WEdge> wEdges;

  void init(int N) {
    n = N;
    adj.assign(n+1, {});
    eAdj.assign(n+1, {});
    wadj.assign(n+1, {});
    edges.clear();
    wEdges.clear();
  }

  // Unweighted edge (also recorded for Euler)
  inline void add_edge(int u, int v) {
    adj[u].push_back(v); adj[v].push_back(u);
    int id = (int)edges.size();
    edges.push_back({u,v});
    eAdj[u].push_back({v,id}); eAdj[v].push_back({u,id});
  }

  // Weighted undirected edge
  inline void add_edge_w(int u, int v, int w) {
    wadj[u].push_back({v,w}); wadj[v].push_back({u,w});
    wEdges.push_back({u,v,w});
  }

  // ---------------------------------------------------------------------------
  // BFS single-source; returns dist (=-1 unreachable) and parent
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

  // Connected components: returns comp id (1..cc) and count
  pair<vector<int>, int> connected_components() const {
    vector<int> comp(n+1, 0); int cid=0; 
    for(int i=1;i<=n;++i) if(!comp[i]){ ++cid; queue<int> q; q.push(i); comp[i]=cid; while(!q.empty()){ int u=q.front(); q.pop(); for(int v: adj[u]) if(!comp[v]){ comp[v]=cid; q.push(v);} } }
    return {comp, cid};
  }

  // Bipartite check (2-coloring). Returns {ok, color}, color in {0,1} or -1.
  pair<bool, vector<int>> bipartite() const {
    vector<int> col(n+1, -1); queue<int> q;
    for(int s=1;s<=n;++s) if(col[s]==-1){ col[s]=0; q.push(s); while(!q.empty()){ int u=q.front(); q.pop(); for(int v: adj[u]){ if(col[v]==-1){ col[v]=col[u]^1; q.push(v);} else if(col[v]==col[u]) return {false, col}; } } }
    return {true, col};
  }

  // Bridges and articulation points (iterative low-link)
  pair<vector<pair<int,int>>, vector<char>> bridges_articulations() const {
    vector<int> tin(n+1,-1), low(n+1,-1), par(n+1,0), it(n+1,0), childCnt(n+1,0);
    vector<char> isArt(n+1, 0);
    vector<pair<int,int>> bridges;
    int timer=0;
    for(int s=1;s<=n;++s) if(tin[s]==-1){
      // DFS stack of (u, parent)
      vector<pair<int,int>> st; st.emplace_back(s, 0);
      while(!st.empty()){
        int u=st.back().first, p=st.back().second;
        if(tin[u]==-1){ tin[u]=low[u]=timer++; par[u]=p; }
        if(it[u] < (int)adj[u].size()){
          int v=adj[u][it[u]++]; if(v==p) continue;
          if(tin[v]!=-1){ low[u]=min(low[u], tin[v]); }
          else{ childCnt[u]++; st.emplace_back(v,u); }
        } else {
          st.pop_back();
          if(p){
            // finishing u: propagate to parent p
            low[p]=min(low[p], low[u]);
            if(low[u] >= tin[p]) isArt[p]=1;
            if(low[u] > tin[p]) bridges.push_back(minmax(p,u));
          }
        }
      }
      if(childCnt[s] >= 2) isArt[s]=1; else isArt[s]=isArt[s] & (par[s]!=0);
    }
    return {bridges, isArt};
  }

  // 2-edge-connected components via bridges; returns (compId, compCount) and bridge tree
  tuple<vector<int>, int, vector<vector<int>>> bridge_tree() const {
    auto [bridges, isArt] = bridges_articulations();
    // mark bridges in a set for O(1) check
    vector<vector<int>> mark(n+1);
    // Use unordered_set of pairs? Instead, store adjacency with edge mark by building a map
    // Simpler: build a set keyed by (min(u,v), max(u,v))
    struct PairHash { size_t operator()(const long long &x) const { return std::hash<long long>()(x); } };
    unordered_set<long long, PairHash> br;
    br.reserve(bridges.size()*2+1);
    auto key = [](int a,int b){ if(a>b) swap(a,b); return ( (long long)a<<32 ) | (unsigned int)b; };
    for(auto &e: bridges) br.insert(key(e.first, e.second));

    vector<int> cid(n+1, 0); int cc=0;
    for(int s=1;s<=n;++s) if(cid[s]==0){
      ++cc; queue<int> q; q.push(s); cid[s]=cc; while(!q.empty()){
        int u=q.front(); q.pop();
        for(int v: adj[u]){
          if(cid[v]) continue;
          if(br.count(key(u,v))) continue; // don't cross bridges
          cid[v]=cc; q.push(v);
        }
      }
    }
    // build bridge tree: vertices are comps 1..cc, edges for each bridge between components
    vector<vector<int>> T(cc+1);
    for(auto &e: bridges){ int a=cid[e.first], b=cid[e.second]; if(a!=b){ T[a].push_back(b); T[b].push_back(a);} }
    return {cid, cc, T};
  }

  // Euler trail (path or cycle). Returns sequence of vertices. Empty if not possible.
  vector<int> euler_trail(int start = 1) const {
    int m = (int)edges.size();
    if(m==0) return {start};
    vector<int> deg(n+1,0); for(auto &e: edges){ deg[e.u]++; deg[e.v]++; }
    int odd=0, s=start;
    for(int i=1;i<=n;++i) if(deg[i]&1){ odd++; s=i; }
    if(!(odd==0 || odd==2)) return {};
    vector<char> used(m, 0);
    vector<int> it(n+1,0), st; st.push_back(s);
    vector<int> path; path.reserve(m+1);
    auto &g = eAdj;
    while(!st.empty()){
      int u = st.back();
      while(it[u] < (int)g[u].size() && used[g[u][it[u]].second]) ++it[u];
      if(it[u] == (int)g[u].size()){
        path.push_back(u); st.pop_back();
      } else {
        auto [v, id] = g[u][it[u]++];
        if(used[id]) continue;
        used[id]=1; // undirected: mark once
        st.push_back(v);
      }
    }
    if((int)path.size() != m+1) return {}; // disconnected edges
    reverse(path.begin(), path.end());
    return path;
  }

  // Dijkstra (non-negative weights) on wadj
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

  // Kruskal MST on wEdges
  pair<long long, vector<pair<int,int>>> kruskal_mst() const {
    vector<int> idx(wEdges.size()); iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int i, int j){ return wEdges[i].w < wEdges[j].w; });
    DSU dsu; dsu.init(n);
    long long total=0; vector<pair<int,int>> used;
    used.reserve(n-1);
    for(int id: idx){ auto e=wEdges[id]; if(dsu.unite(e.u, e.v)){ total += e.w; used.push_back({e.u, e.v}); if((int)used.size()==n-1) break; } }
    if((int)used.size()!=n-1) return { -1, {} }; // not connected
    return { total, used };
  }
};

