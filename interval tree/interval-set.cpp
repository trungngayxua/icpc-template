/**
 * @file interval-set.cpp
 * @brief Disjoint Interval Set (union of [l, r)) and Segment-Cover tree
 *
 * Overview
 * - IntervalSet: maintain a set of disjoint half-open intervals [l, r)
 *   with fast add/remove (merge/split), membership and stats:
 *     - total covered length
 *     - maximum segment length
 * - SegCover: coordinate-compressed segment tree that maintains
 *   dynamic union length under add/remove of segments.
 *
 * Conventions
 * - Prefer half-open intervals [l, r) on integer domain.
 * - For closed input [L, R], convert to [L, R+1).
 * - Adjacent intervals [a,b) and [b,c) are merged to [a,c).
 */

#pragma once
#include <bits/stdc++.h>
using namespace std;

/* ---------------- Disjoint Interval Set on [l, r) ------------------ */
struct IntervalSet {
    using ll = long long;
    static constexpr ll NEG_INF = (ll)-4e18;
    static constexpr ll POS_INF = (ll) 4e18;

    // Store disjoint intervals, each as [l, r) with l < r
    set<pair<ll,ll>> st;
    ll total = 0;        // total covered length (sum of r-l)
    multiset<ll> lens;   // multiset of current segment lengths

    bool empty() const { return st.empty(); }
    int size() const { return (int)st.size(); }
    ll total_coverage() const { return total; }
    ll max_segment() const { return lens.empty() ? 0 : *lens.rbegin(); }

    // Check if point x is covered
    bool contains(ll x) const {
        auto it = st.upper_bound({x, POS_INF});
        if (it == st.begin()) return false;
        --it;
        return it->first <= x && x < it->second;
    }

    // Check if there exists an interval intersecting [l, r)
    bool intersects(ll l, ll r) const {
        if (l >= r) return false;
        auto it = st.lower_bound({l, NEG_INF});
        if (it != st.begin()) {
            auto p = prev(it);
            if (p->second > l) return true; // prev overlaps [l, r)
        }
        if (it != st.end() && it->first < r) return true;
        return false;
    }

    // Add [l, r) and merge with overlapping/adjacent intervals
    void add(ll l, ll r) {
        if (l >= r) return;
        auto it = st.lower_bound({l, NEG_INF});
        if (it != st.begin()) {
            auto p = prev(it);
            if (p->second >= l) it = p; // p overlaps/adjacent to [l, r)
        }
        ll L = l, R = r;
        while (it != st.end() && it->first <= R) {
            L = min(L, it->first);
            R = max(R, it->second);
            ll seglen = it->second - it->first;
            total -= seglen;
            auto hit = lens.find(seglen);
            if (hit != lens.end()) lens.erase(hit);
            it = st.erase(it);
        }
        st.insert({L, R});
        total += (R - L);
        lens.insert(R - L);
    }

    // Remove coverage of [l, r) from the set (may split intervals)
    void remove(ll l, ll r) {
        if (l >= r) return;
        auto it = st.lower_bound({l, NEG_INF});
        if (it != st.begin()) {
            auto p = prev(it);
            if (p->second > l) {
                // p contains l (or overlaps)
                ll a = p->first, b = p->second;
                ll seglen = b - a;
                total -= seglen;
                auto hit = lens.find(seglen);
                if (hit != lens.end()) lens.erase(hit);
                st.erase(p);
                if (a < l) { st.insert({a, l}); total += (l - a); lens.insert(l - a); }
                if (b > r) { st.insert({r, b}); total += (b - r); lens.insert(b - r); }
            }
        }
        it = st.lower_bound({l, NEG_INF});
        while (it != st.end() && it->first < r) {
            ll a = it->first, b = it->second;
            it = st.erase(it);
            ll seglen = b - a;
            total -= seglen;
            auto hit = lens.find(seglen);
            if (hit != lens.end()) lens.erase(hit);
            if (b > r) {
                st.insert({r, b});
                total += (b - r);
                lens.insert(b - r);
                break;
            }
            // else: removed entirely, continue
        }
    }

    // Return intervals as a vector (optional helper)
    vector<pair<ll,ll>> intervals() const {
        return vector<pair<ll,ll>>(st.begin(), st.end());
    }
};

/* ---------------- Segment Tree for Covered Length (compressed) ------------------ */
struct SegCover {
    using ll = long long;
    vector<ll> X; // sorted unique coordinates
    struct Node { int cnt = 0; ll len = 0; };
    vector<Node> st;

    SegCover() {}
    explicit SegCover(const vector<ll>& xs) { init(xs); }

    void init(const vector<ll>& xs) {
        X = xs;
        if (!is_sorted(X.begin(), X.end())) sort(X.begin(), X.end());
        X.erase(unique(X.begin(), X.end()), X.end());
        st.assign(4 * max<int>(1, (int)X.size()), {});
        if ((int)X.size() >= 2) build(1, 0, (int)X.size() - 1);
    }

    void build(int p, int l, int r) {
        st[p] = {0, 0};
        if (l + 1 == r) return;
        int m = (l + r) >> 1;
        build(p<<1, l, m);
        build(p<<1|1, m, r);
    }

    void pull(int p, int l, int r) {
        if (st[p].cnt > 0) {
            st[p].len = X[r] - X[l];
        } else if (l + 1 == r) {
            st[p].len = 0;
        } else {
            st[p].len = st[p<<1].len + st[p<<1|1].len;
        }
    }

    void add(int p, int l, int r, int ql, int qr, int v) {
        if (qr <= l || r <= ql) return;
        if (ql <= l && r <= qr) { st[p].cnt += v; pull(p, l, r); return; }
        int m = (l + r) >> 1;
        add(p<<1, l, m, ql, qr, v);
        add(p<<1|1, m, r, ql, qr, v);
        pull(p, l, r);
    }

    // Cover/uncover [L, R) in real coordinates (must exist in X-range)
    void cover(ll L, ll R, int v) {
        if (L >= R || X.size() < 2) return;
        int l = (int)(lower_bound(X.begin(), X.end(), L) - X.begin());
        int r = (int)(lower_bound(X.begin(), X.end(), R) - X.begin());
        if (l < r) add(1, 0, (int)X.size() - 1, l, r, v);
    }

    // Current total covered length
    ll covered() const { return X.size() < 2 ? 0 : st[1].len; }
};

/* ---------------- Example ------------------
int main() {
    // IntervalSet usage
    IntervalSet S;
    S.add(1, 5);
    S.add(5, 7); // merges to [1,7)
    S.remove(3, 6); // leaves [1,3) and [6,7)
    cout << S.contains(6) << "\n";       // 1
    cout << S.total_coverage() << "\n";   // 3
    cout << S.max_segment() << "\n";      // 2

    // SegCover usage (coordinate-compressed)
    vector<long long> xs = {0, 1, 3, 7, 10};
    SegCover seg(xs);
    seg.cover(1, 7, +1);
    cout << seg.covered() << "\n";        // 6
}
------------------------------------------------*/

