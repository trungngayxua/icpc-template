/**
 * @file treap.cpp
 * @brief Treap templates: keyed set/map with order-statistics and implicit treap for sequences
 *
 * Key Facts
 * - Treap = BST (by key/position) + heap (by random priority)
 * - Expected O(log N) for insert/erase/find/split/merge
 * - Two common variants provided here:
 *   1) KeyedTreap: like set/map (supports duplicates via cnt), order_of_key, find_by_order, lower_bound
 *   2) ImplicitTreap: treats an array as a BST by index; supports split/merge, insert/delete by position,
 *      reverse on range (lazy), and range sum query
 *
 * Typical Uses
 * - Replacement for PBDS ordered_set when portability matters
 * - Maintain a dynamic sequence with cut/paste, reverse, k-th, sum/min, etc.
 * - Online problems where split/merge on ordered data is convenient
 */

#pragma once
#include <bits/stdc++.h>
using namespace std;

/* ---------------- RNG ---------------- */
static mt19937 rng((uint32_t)chrono::steady_clock::now().time_since_epoch().count());

/* ========================================================================
 * 1) KeyedTreap: multiset-style treap with order statistics
 *    - Stores integer keys (change to long long if needed)
 *    - Supports duplicates via cnt
 * ====================================================================== */
struct KeyedTreap {
    struct Node {
        long long key;
        uint32_t pr;
        Node *l = nullptr, *r = nullptr;
        int sz = 1;     // size including duplicates
        int cnt = 1;    // multiplicity of this key
        explicit Node(long long k, uint32_t p): key(k), pr(p) {}
    };

    Node* root = nullptr;

    static int getsz(Node* t) { return t ? t->sz : 0; }
    static void pull(Node* t) {
        if (!t) return;
        t->sz = t->cnt + getsz(t->l) + getsz(t->r);
    }

    // Merge assumes all keys in L < all keys in R
    static Node* merge(Node* L, Node* R) {
        if (!L || !R) return L ? L : R;
        if (L->pr < R->pr) {
            L->r = merge(L->r, R);
            pull(L);
            return L;
        } else {
            R->l = merge(L, R->l);
            pull(R);
            return R;
        }
    }

    // Split by key k: left has keys < k, right has keys >= k
    static pair<Node*, Node*> split_key(Node* t, long long k) {
        if (!t) return {nullptr, nullptr};
        if (k <= t->key) {
            auto res = split_key(t->l, k);
            t->l = res.second; pull(t);
            return {res.first, t};
        } else {
            auto res = split_key(t->r, k);
            t->r = res.first; pull(t);
            return {t, res.second};
        }
    }

    // Insert key (duplicates aggregated in cnt)
    static void insert(Node* &t, long long k, uint32_t pr) {
        if (!t) { t = new Node(k, pr); return; }
        if (k == t->key) { t->cnt++; pull(t); return; }
        if (pr < t->pr) {
            auto [L, R] = split_key(t, k);
            t = new Node(k, pr);
            t->l = L; t->r = R; pull(t);
            return;
        }
        if (k < t->key) insert(t->l, k, pr);
        else insert(t->r, k, pr);
        pull(t);
    }

    // Erase one occurrence of k (if exists)
    static void erase(Node* &t, long long k) {
        if (!t) return;
        if (k == t->key) {
            if (t->cnt > 1) { t->cnt--; pull(t); return; }
            Node* u = merge(t->l, t->r);
            delete t; t = u; return;
        }
        if (k < t->key) erase(t->l, k); else erase(t->r, k);
        pull(t);
    }

    // Lower bound: smallest node with key >= k (nullptr if none)
    static Node* lower_bound(Node* t, long long k) {
        Node* ans = nullptr;
        while (t) {
            if (t->key >= k) { ans = t; t = t->l; }
            else t = t->r;
        }
        return ans;
    }

    // Count elements < k
    static int order_of_key(Node* t, long long k) {
        if (!t) return 0;
        if (k <= t->key) return order_of_key(t->l, k);
        return t->cnt + getsz(t->l) + order_of_key(t->r, k);
    }

    // Find k-th element (0-based). Returns pointer or nullptr if out-of-range
    static Node* find_by_order(Node* t, int k) {
        if (!t || k < 0 || k >= getsz(t)) return nullptr;
        int L = getsz(t->l);
        if (k < L) return find_by_order(t->l, k);
        if (k < L + t->cnt) return t;
        return find_by_order(t->r, k - L - t->cnt);
    }

    // Public wrappers
    void insert(long long k) { insert(root, k, rng()); }
    void erase(long long k) { erase(root, k); }
    int size() const { return getsz(root); }
    bool empty() const { return root == nullptr; }
    int order_of_key(long long k) const { return order_of_key(root, k); }
    const Node* find_by_order(int k) const { return find_by_order(root, k); }
    const Node* lower_bound(long long k) const { return lower_bound(root, k); }
    bool contains(long long k) const {
        auto* p = lower_bound(root, k);
        return p && p->key == k;
    }
};


/* ========================================================================
 * 2) ImplicitTreap: sequence treap with reverse and range sum
 *    - Treats an array as a BST by position (k-th)
 *    - Supports split/merge by position, insert/delete, reverse range (lazy), range sum
 * ====================================================================== */
struct ImplicitTreap {
    struct Node {
        long long val = 0;  // payload
        long long sum = 0;  // aggregator: sum over subtree
        uint32_t pr = 0;    // heap priority
        Node *l = nullptr, *r = nullptr;
        int sz = 1;
        bool rev = false;   // lazy reverse flag
        Node() {}
        Node(long long v, uint32_t p): val(v), sum(v), pr(p) {}
    };

    Node* root = nullptr;

    static int getsz(Node* t) { return t ? t->sz : 0; }
    static long long getsum(Node* t) { return t ? t->sum : 0; }

    static void apply_rev(Node* t) {
        if (!t) return;
        t->rev ^= 1;
        swap(t->l, t->r);
    }

    static void push(Node* t) {
        if (!t || !t->rev) return;
        apply_rev(t->l);
        apply_rev(t->r);
        t->rev = false;
    }

    static void pull(Node* t) {
        if (!t) return;
        // Ensure children are in correct state for aggregates
        // (call push on children before using their sums if using more complex lazies)
        t->sz = 1 + getsz(t->l) + getsz(t->r);
        t->sum = t->val + getsum(t->l) + getsum(t->r);
    }

    // Split by position: left has first k elements (0-based), right has the rest
    static pair<Node*, Node*> split_pos(Node* t, int k) {
        if (!t) return {nullptr, nullptr};
        push(t);
        if (getsz(t->l) >= k) {
            auto res = split_pos(t->l, k);
            t->l = res.second; pull(t);
            return {res.first, t};
        } else {
            auto res = split_pos(t->r, k - getsz(t->l) - 1);
            t->r = res.first; pull(t);
            return {t, res.second};
        }
    }

    static Node* merge(Node* L, Node* R) {
        if (!L || !R) return L ? L : R;
        if (L->pr < R->pr) {
            push(L);
            L->r = merge(L->r, R);
            pull(L);
            return L;
        } else {
            push(R);
            R->l = merge(L, R->l);
            pull(R);
            return R;
        }
    }

    // Insert value v at position pos (0-based)
    void insert_at(int pos, long long v) {
        auto [A, B] = split_pos(root, pos);
        root = merge(merge(A, new Node(v, rng())), B);
    }

    // Erase range [l, r) by position
    void erase_range(int l, int r) {
        auto [A, B] = split_pos(root, l);
        auto [M, C] = split_pos(B, r - l);
        // Optionally delete M nodes to free memory
        root = merge(A, C);
    }

    // Reverse range [l, r)
    void reverse_range(int l, int r) {
        auto [A, B] = split_pos(root, l);
        auto [M, C] = split_pos(B, r - l);
        if (M) apply_rev(M);
        root = merge(A, merge(M, C));
    }

    // Range sum on [l, r)
    long long range_sum(int l, int r) {
        auto [A, B] = split_pos(root, l);
        auto [M, C] = split_pos(B, r - l);
        long long ans = getsum(M);
        root = merge(A, merge(M, C));
        return ans;
    }

    // Build from vector in O(n log n) expected
    void build(const vector<long long>& a) {
        root = nullptr;
        for (int i = 0; i < (int)a.size(); ++i) insert_at(i, a[i]);
    }

    int size() const { return getsz(root); }
    bool empty() const { return root == nullptr; }
};

/* ---------------- Example ------------------
int main(){
    // KeyedTreap example
    KeyedTreap T;
    T.insert(5); T.insert(3); T.insert(7); T.insert(5);
    cout << T.size() << "\n";                       // 4 (with duplicates)
    cout << T.order_of_key(5) << "\n";               // 1 (only key 3 is < 5)
    cout << (T.find_by_order(2)->key) << "\n";       // k-th element (0-based)
    T.erase(5);

    // ImplicitTreap example
    ImplicitTreap IT;
    IT.build({1,2,3,4,5});
    IT.reverse_range(1,4);  // [1,4,3,2,5]
    cout << IT.range_sum(1,4) << "\n"; // 4+3+2 = 9
}
------------------------------------------------*/

