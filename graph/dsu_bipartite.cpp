struct DSU {
    vector<int> p, sz, parity; 
    // parity[x] = 0 or 1; parity[x] is XOR distance to its parent

    DSU(int n) {
        p.resize(n + 1);
        sz.assign(n + 1, 1);
        parity.assign(n + 1, 0);
        iota(p.begin(), p.end(), 0);
    }

    pair<int,int> find(int x) {
        if (p[x] == x) return {x, 0};
        auto [root, par] = find(p[x]);
        parity[x] ^= par;
        p[x] = root;
        return {p[x], parity[x]};
    }

    bool unite(int a, int b) {
        auto [ra, pa] = find(a);
        auto [rb, pb] = find(b);
        if (ra == rb) {
            // Same root → check parity
            return (pa != pb); // false if conflict (odd cycle)
        }
        if (sz[ra] < sz[rb]) swap(ra, rb), swap(pa, pb);
        p[rb] = ra;
        parity[rb] = pa ^ pb ^ 1; // enforce a and b to be in opposite sets
        sz[ra] += sz[rb];
        return true;
    }
};