/**
 * @file line_container.hpp
 * @brief Dynamic Convex Hull (Line Container) and Monotonic CHT templates
 * @note For dynamic max queries or monotone DP optimization
 *
 * Usage:
 *   LineContainer lc;
 *   lc.add(a, b);               // add line y = a*x + b
 *   long long ans = lc.query(x); // get max y at x
 *
 * To switch between MAX/MIN:
 *   - For min: add(-a, -b), then query(x) * -1
 *   - Or flip inequality signs in bad()/isect()
 */

#pragma once
#include <bits/stdc++.h>
using namespace std;

/* ---------------- Dynamic Line Container (KACTL style) ------------------ */
/**
 * Supports adding lines of the form y = a*x + b in arbitrary order,
 * and querying maximum y at arbitrary x in O(log N).
 * Uses long double intersection points; safe for 64-bit coefficients.
 */
struct Line {
    long long a, b;
    mutable long double x;
    bool operator<(const Line& o) const { return a < o.a; }
    bool operator<(long double X) const { return x < X; }
};

struct LineContainer : multiset<Line, less<>> {
    static constexpr long double INF = 1e400;

    // Compute intersection of line x and y, store in x->x
    long double isect(iterator x, iterator y) {
        if (y == end()) return x->x = INF;
        if (x->a == y->a) x->x = (x->b > y->b ? -INF : INF);
        else x->x = (long double)(y->b - x->b) / (x->a - y->a);
        return x->x;
    }

    // Add new line (for maximum)
    void add(long long a, long long b) {
        auto z = insert({a, b, 0}), y = z++, x = y;
        while (z != end() && y->a == z->a) z = erase(z);
        if (x != begin() && prev(x)->a == x->a) { erase(x); return; }

        while (y != begin() && prev(y)->x >= isect(prev(y), y)) {
            auto p = prev(y);
            isect(p, erase(y));
            y = p;
        }
        auto nxt = y;
        while (++nxt != end() && y->x >= isect(y, nxt)) nxt = erase(nxt);
        if (nxt != end()) isect(y, nxt);
    }

    // Query max at given x
    long long query(long long x) {
        assert(!empty());
        auto it = lower_bound((long double)x);
        if (it == end()) it = prev(end());
        return it->a * x + it->b;
    }
};

/* ---------------- Monotonic Convex Hull Trick (Deque) ------------------ */
/**
 * For monotone queries and lines (O(1) amortized).
 * Add lines with monotonic slopes and query with monotonic x.
 * Default: MIN queries, slopes increasing, x non-decreasing.
 */
struct LineM {
    long long a, b;
    long double interX;
    LineM(long long A=0, long long B=0, long double X=-1e400) : a(A), b(B), interX(X) {}
    long long value(long long x) const { return a * x + b; }
};

struct CHTMonotone {
    deque<LineM> dq;

    // Check if middle line l2 is unnecessary (for min)
    bool bad(const LineM& l1, const LineM& l2, const LineM& l3) {
        return (__int128)(l3.b - l1.b) * (l1.a - l2.a)
             <= (__int128)(l2.b - l1.b) * (l1.a - l3.a);
    }

    // Add line with slope a (increasing)
    void add(long long a, long long b) {
        LineM nl(a, b);
        while (dq.size() >= 2 && bad(dq[dq.size()-2], dq.back(), nl)) dq.pop_back();
        if (!dq.empty()) {
            long double x = (long double)(dq.back().b - nl.b) / (nl.a - dq.back().a);
            nl.interX = x;
        }
        dq.push_back(nl);
    }

    // Query min at x, assuming x non-decreasing
    long long query(long long x) {
        while (dq.size() >= 2 && dq[1].interX <= x) dq.pop_front();
        return dq.front().value(x);
    }
};

/* ---------------- Example ------------------
int main() {
    LineContainer lc; // dynamic hull
    lc.add(2, 3); lc.add(-1, 5);
    cout << lc.query(4) << "\n";

    CHTMonotone cht; // monotone DP hull (min)
    cht.add(-5, 10);
    cht.add(-2, 4);
    cout << cht.query(3) << "\n";
}
------------------------------------------------*/  