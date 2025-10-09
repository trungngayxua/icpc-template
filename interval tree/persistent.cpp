// Persistent Data Structures — Blueprint (array / segment tree / binary trie / string trie)
//
// Ý tưởng chung (path-copying, fully persistent):
// - Mỗi cập nhật tạo 1 phiên bản (root) mới, chỉ sao chép O(log N) nút trên đường đi.
// - Lưu vector roots[]; mỗi query thao tác trên root mong muốn.
// - Ưu điểm: truy vết lịch sử, xử lý offline, cấu trúc phiên bản.
// - Độ phức tạp: update/query ~ O(log N) (segment tree), O(|key|) (trie).
// - Bộ nhớ: ~ số_update × chi_phí_mỗi_update; khuyến nghị dùng pool (vector) thay vì new/delete.

#include <bits/stdc++.h>
using namespace std;

/*
================================================================================
SECTION A — Persistent Segment Tree (mảng phiên bản, range sum)
================================================================================
API (pool-based, 0-index):
- build(arr): trả về root phiên bản gốc từ mảng values[0..n-1]
- update(prev_root, pos, new_val): trả về root phiên bản mới sau khi set values[pos]=new_val
- query(root, ql, qr): trả về tổng đoạn [ql, qr] tại phiên bản root
Ghi chú:
- Có thể thay merge (sum) thành min/max/xor... và đổi neutral tương ứng.
- Nếu cần range update, dùng kỹ thuật khác (lazy persistent phức tạp hơn) hoặc Fenwick không phù hợp cho persistence chuẩn.
*/

struct PSTSum {
    struct Node { int L, R; long long sum; };
    int n;
    vector<Node> pool; // pool[0] không dùng để dùng 0 làm null
    PSTSum(): n(0) { pool.reserve(1 << 20); pool.push_back({0,0,0}); }

    int new_node(int from = 0) {
        pool.push_back(pool[from]);
        return (int)pool.size() - 1;
    }

    int build_rec(const vector<long long>& a, int l, int r) {
        int u = new_node(0);
        if (l == r) { pool[u].sum = a[l]; return u; }
        int m = (l + r) >> 1;
        pool[u].L = build_rec(a, l, m);
        pool[u].R = build_rec(a, m + 1, r);
        pool[u].sum = pool[ pool[u].L ].sum + pool[ pool[u].R ].sum;
        return u;
    }

    int build(const vector<long long>& a) {
        n = (int)a.size();
        return n ? build_rec(a, 0, n - 1) : 0;
    }

    int update_rec(int prev, int l, int r, int pos, long long val) {
        int u = new_node(prev);
        if (l == r) { pool[u].sum = val; return u; }
        int m = (l + r) >> 1;
        if (pos <= m) pool[u].L = update_rec(pool[prev].L, l, m, pos, val);
        else          pool[u].R = update_rec(pool[prev].R, m + 1, r, pos, val);
        pool[u].sum = pool[ pool[u].L ].sum + pool[ pool[u].R ].sum;
        return u;
    }

    int update(int prev_root, int pos, long long val) {
        if (n == 0) return 0; // no-op
        return update_rec(prev_root, 0, n - 1, pos, val);
    }

    long long query_rec(int u, int l, int r, int ql, int qr) const {
        if (!u || qr < l || r < ql) return 0LL; // neutral for sum
        if (ql <= l && r <= qr) return pool[u].sum;
        int m = (l + r) >> 1;
        return query_rec(pool[u].L, l, m, ql, qr) + query_rec(pool[u].R, m + 1, r, ql, qr);
    }

    long long query(int root, int ql, int qr) const {
        if (n == 0 || ql > qr) return 0LL;
        ql = max(ql, 0); qr = min(qr, n - 1);
        return query_rec(root, 0, n - 1, ql, qr);
    }
};

/*
Sử dụng nhanh:
    PSTSum pst;
    int r0 = pst.build(a);              // phiên bản 0
    int r1 = pst.update(r0, i, x);      // phiên bản 1
    long long s = pst.query(r1, L, R);  // query trên phiên bản 1
*/

/*
================================================================================
SECTION B — Persistent Binary Trie (số nguyên, 0-index, MSB-first)
================================================================================
API:
- add(prev_root, x, +1/-1): thêm/xoá một số x, trả về root mới
- countLess(root, x): đếm số phần tử < x
- maxXor(root, x): tìm giá trị y trong trie tối đa hoá (x xor y)
Ghi chú:
- Thay MAX_BITS tuỳ miền giá trị (mặc định 31 cho int không âm < 2^31).
*/

struct PBinaryTrie {
    struct Node { int ch[2]; int cnt; Node(){ ch[0]=ch[1]=0; cnt=0; } };
    vector<Node> t; int MAX_BITS;
    PBinaryTrie(int MAX_BITS = 31): MAX_BITS(MAX_BITS) { t.reserve(1<<20); t.push_back(Node()); }

    int clone(int from) { t.push_back(t[from]); return (int)t.size()-1; }

    int add(int prev, int x, int delta) {
        int u = clone(prev); t[u].cnt += delta; int cur = u;
        for (int b = MAX_BITS; b >= 0; --b) {
            int bit = (x >> b) & 1;
            int nxt = t[cur].ch[bit];
            int v = clone(nxt);
            t[cur].ch[bit] = v; cur = v; t[cur].cnt += delta;
        }
        return u;
    }

    int countLess(int root, int x) const {
        long long res = 0; int cur = root;
        for (int b = MAX_BITS; b >= 0 && cur; --b) {
            int bit = (x >> b) & 1;
            if (bit) res += (long long)t[ t[cur].ch[0] ].cnt, cur = t[cur].ch[1];
            else     cur = t[cur].ch[0];
        }
        return (int)res;
    }

    int maxXor(int root, int x) const {
        int cur = root, ans = 0; if (!cur) return 0;
        for (int b = MAX_BITS; b >= 0; --b) {
            int bit = (x >> b) & 1, want = bit ^ 1;
            int to = t[cur].ch[want];
            if (to && t[to].cnt > 0) { ans |= (1 << b); cur = to; }
            else cur = t[cur].ch[bit];
            if (!cur) break;
        }
        return ans; // trả về (x xor best)
    }
};

/*
Sử dụng nhanh:
    PBinaryTrie bt(31);
    int r0 = 0;                // r0 là 0 (trie rỗng)
    int r1 = bt.add(r0, 5, +1);
    int r2 = bt.add(r1, 8, +1);
    int c  = bt.countLess(r2, 7);     // đếm < 7
    int xo = bt.maxXor(r2, 10);       // giá trị (10 xor best)
*/

/*
================================================================================
SECTION C — Persistent String Trie (26 chữ thường)
================================================================================
API:
- setVal(prev_root, key, val): trả về root mới sau khi gán key -> val
- getVal(root, key): trả về val hoặc defaultVal nếu không tồn tại
Ghi chú:
- Có thể mở rộng bảng chữ cái (ALPHA) theo nhu cầu.
*/

struct PStringTrie {
    struct Node { int ch[26]; int val; Node(){ memset(ch, 0, sizeof(ch)); val = INT_MIN; } };
    vector<Node> t; int ALPHA = 26;
    PStringTrie(){ t.reserve(1<<20); t.push_back(Node()); }

    int clone(int from){ t.push_back(t[from]); return (int)t.size()-1; }

    int setVal(int prev, const string& s, int v) {
        int u = clone(prev), cur = u;
        for (char c: s) {
            int idx = c - 'a'; if (idx < 0 || idx >= ALPHA) return u; // bỏ qua ký tự ngoài phạm vi
            int nxt = t[cur].ch[idx]; int nv = clone(nxt);
            t[cur].ch[idx] = nv; cur = nv;
        }
        t[cur].val = v; return u;
    }

    int getVal(int root, const string& s, int defaultVal = INT_MIN) const {
        int cur = root;
        for (char c: s) {
            int idx = c - 'a'; if (idx < 0 || idx >= ALPHA) return defaultVal;
            cur = t[cur].ch[idx]; if (!cur) return defaultVal;
        }
        return t[cur].val == INT_MIN ? defaultVal : t[cur].val;
    }
};

/*
Sử dụng nhanh:
    PStringTrie st;
    int r0 = 0;
    int r1 = st.setVal(r0, "abc", 42);
    int v  = st.getVal(r1, "abc", -1); // 42
*/

/*
================================================================================
GỢI Ý/RECIPE PHỔ BIẾN
================================================================================
- Versioned array: dùng PSTSum; mỗi update tạo root mới, truy vấn theo phiên bản.
- Distinct count trên [L,R]: lưu tại vị trí cuối cùng của mỗi giá trị, update khi gặp lại.
- K-th nhỏ nhất trên đoạn: cần PST theo tần suất (value-compressed) + tìm nhị phân trên cây.
- Binary trie: đếm < X theo phiên bản, truy vấn max xor theo phiên bản.
- String trie: từ điển phiên bản theo thời gian.
*/

