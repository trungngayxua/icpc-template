/**
 * @file segment-tree-pointer.cpp
 * @brief Segment Tree dạng con trỏ (dynamic) cho miền [L, R) rất lớn
 *
 * Mục tiêu
 * - Cây đoạn dùng con trỏ, cấp phát node theo nhu cầu (sparse), phù hợp khi miền chỉ số rất lớn
 *   (ví dụ tới 1e18) nhưng số thao tác cập nhật/truy vấn tương đối ít.
 * - Dễ tuỳ chỉnh phép gộp (sum/min/max/…): thay `ID` (đơn vị) và hàm `op(a,b)`.
 * - Kèm biến thể lazy phổ biến: cộng trên đoạn và truy vấn tổng (range add + range sum).
 *
 * Khi nào dùng
 * - Dùng: miền chỉ số dài, không thể/cần thiết cấp phát mảng 4*N; chỉ có một phần nhỏ các điểm/đoạn được chạm tới.
 * - Tránh: miền vừa phải (<= vài triệu): segment tree mảng hoặc Fenwick/BIT thường nhanh và gọn hơn.
 *
 * Quy ước
 * - Chỉ số 0-based. Đoạn chuẩn dạng nửa mở [l, r).
 * - Luôn đảm bảo L < R trong khởi tạo. Mid tính an toàn: l + (r-l)/2.
 * - Node null biểu thị node rỗng với giá trị `ID`.
 *
 * Tuỳ chỉnh nhanh (monoid):
 * - Tổng:    `ID = 0`,      `op = [](T a,T b){return a+b;}`
 * - Min:     `ID = +INF`,   `op = [](T a,T b){return min(a,b);}`
 * - Max:     `ID = -INF`,   `op = [](T a,T b){return max(a,b);}`
 * - XOR:     `ID = 0`,      `op = [](T a,T b){return a^b;}`
 *
 * Biến thể kèm theo
 * - DynLazySegTreeAddSum: hỗ trợ cộng trên đoạn và truy vấn tổng.
 *   Có thể mở rộng sang các lazy khác (assign, chmax/chmin…) nhưng phức tạp hơn, cần định nghĩa rõ `apply/compose`.
 */

#pragma once
#include <bits/stdc++.h>
using namespace std;

/* ========================================================================
 * 1) DynSegTreeMonoid — point set + rectangle query (monoid op)
 *    - Tuỳ chỉnh bằng ID (đơn vị) và op(a,b)
 *    - Dùng cho: set/overwrite giá trị tại 1 điểm, truy vấn gộp trên [ql, qr)
 * ====================================================================== */
template<class T, class Op>
struct DynSegTreeMonoid {
    struct Node {
        T val; Node *l=nullptr, *r=nullptr;
        explicit Node(const T& v): val(v) {}
    };

    long long L, R;   // miền gốc [L, R)
    T ID;             // phần tử đơn vị
    Op op;            // phép gộp
    Node* root = nullptr;

    DynSegTreeMonoid(long long L, long long R, T id, Op op)
        : L(L), R(R), ID(id), op(op) { assert(L < R); }

    // Lấy giá trị node (kể cả null) — null tương đương ID
    inline T value(Node* n) const { return n ? n->val : ID; }

    // Kéo giá trị từ 2 con
    inline void pull(Node* n){ n->val = op(value(n->l), value(n->r)); }

    // Gán điểm p = v
    void set_point(long long p, const T& v){ set_point(root, L, R, p, v); }

    // Truy vấn gộp trên [ql, qr)
    T query(long long ql, long long qr) const { return query(root, L, R, ql, qr); }

    // (Tuỳ chọn) Lấy lại giá trị điểm: query(p, p+1)
    T get_point(long long p) const { return query(p, p+1); }

private:
    void ensure(Node* &n){ if(!n) n = new Node(ID); }

    void set_point(Node* &n, long long l, long long r, long long p, const T& v){
        if(p < l || p >= r) return;
        ensure(n);
        if(l + 1 == r){ n->val = v; return; }
        long long m = l + ((r - l) >> 1);
        if(p < m) set_point(n->l, l, m, p, v); else set_point(n->r, m, r, p, v);
        pull(n);
    }

    T query(Node* n, long long l, long long r, long long ql, long long qr) const {
        if(!n || qr <= l || r <= ql) return ID;
        if(ql <= l && r <= qr) return n->val;
        long long m = l + ((r - l) >> 1);
        return op(query(n->l, l, m, ql, qr), query(n->r, m, r, ql, qr));
    }
};

/* ========================================================================
 * 2) DynLazySegTreeAddSum — range add + range sum (long long)
 *    - Tổng hợp theo SUM; cập nhật là cộng (add) trên đoạn.
 *    - Hữu ích cho bài hình chữ nhật 1D trên miền dài (nhiều lần cộng và hỏi tổng).
 * ====================================================================== */
struct DynLazySegTreeAddSum {
    struct Node { long long sum=0, add=0; Node *l=nullptr, *r=nullptr; };
    long long L, R; // [L, R)
    Node* root = nullptr;

    explicit DynLazySegTreeAddSum(long long L, long long R): L(L), R(R) { assert(L < R); }

    // Cộng delta trên [ql, qr)
    void range_add(long long ql, long long qr, long long delta){ range_add(root, L, R, ql, qr, delta); }
    // Truy vấn tổng trên [ql, qr)
    long long range_sum(long long ql, long long qr) const { return range_sum(root, L, R, ql, qr); }

private:
    static inline long long seg_len(long long l, long long r){ return r - l; }
    static inline void ensure(Node* &n){ if(!n) n = new Node(); }

    // Áp dụng lazy lên node (không đi xuống)
    static inline void apply(Node* &n, long long l, long long r, long long delta){
        ensure(n);
        n->sum += delta * seg_len(l, r);
        n->add += delta;
    }

    static inline void push(Node* n, long long l, long long r){
        if(!n || n->add == 0 || l + 1 == r) return;
        long long m = l + ((r - l) >> 1);
        // tạo con khi cần và đẩy lazy
        apply(n->l, l, m, n->add);
        apply(n->r, m, r, n->add);
        n->add = 0;
    }

    static inline void pull(Node* n){
        long long left = n->l ? n->l->sum : 0;
        long long right = n->r ? n->r->sum : 0;
        n->sum = left + right;
    }

    void range_add(Node* &n, long long l, long long r, long long ql, long long qr, long long delta){
        if(qr <= l || r <= ql) return;
        if(ql <= l && r <= qr){ apply(n, l, r, delta); return; }
        ensure(n); push(n, l, r);
        long long m = l + ((r - l) >> 1);
        range_add(n->l, l, m, ql, qr, delta);
        range_add(n->r, m, r, ql, qr, delta);
        pull(n);
    }

    long long range_sum(Node* n, long long l, long long r, long long ql, long long qr) const {
        if(!n || qr <= l || r <= ql) return 0LL;
        if(ql <= l && r <= qr) return n->sum;
        long long m = l + ((r - l) >> 1);
        // Không gọi push const; tính đúng vì sum của n đã bao hàm lazy tại n.
        return range_sum(n->l, l, m, ql, qr) + range_sum(n->r, m, r, ql, qr);
    }
};

/* ---------------- Ví dụ sử dụng ------------------
// Tổng trên miền rất lớn với cập nhật cộng theo đoạn:
// long long L = 0, R = (long long)1e18;
// DynLazySegTreeAddSum st(L, R);
// st.range_add(10, 20, +5);   // cộng +5 cho mọi i in [10,20)
// st.range_add(15, 18, +2);
// cout << st.range_sum(0, 100) << "\n"; // tổng trên [0,100)

// Monoid tuỳ biến (ví dụ Max) với point set + range query:
// auto op = [](long long a, long long b){ return max(a,b); };
// const long long NEG = -(1LL<<60);
// DynSegTreeMonoid<long long, decltype(op)> t(0, (long long)1e18, NEG, op);
// t.set_point(1234567890123LL, 42);
// cout << t.query(0, 2e12) << "\n"; // max trên [0, 2e12)

/* ---------------- Gợi ý tuỳ chỉnh ------------------
1) Đổi phép gộp (sum/min/max/xor...):
   - Chọn `ID` đúng với đơn vị của phép gộp và đặt `op` tương ứng.
   - Không cần đổi logic còn lại.

2) Point add thay vì set (với monoid cộng):
   - Lấy `old = t.get_point(p)` rồi `t.set_point(p, old + delta)`.
   - Hoặc tự thêm hàm `add_point` tương tự `set_point` nhưng cộng tại lá.

3) Lazy khác ngoài cộng:
   - Cần thiết kế `apply(node, len, tag)` và `push/compose` tương ứng.
   - Ví dụ range assign + range sum: tag ghi (hasAssign, assignVal, addDelta) và ưu tiên assign ghi đè add; cẩn thận khi `push`.

4) Tối ưu bộ nhớ/hiệu năng:
   - Tránh tạo node khi không cần (chỉ tạo khi cập nhật đi qua hoặc cần lưu lazy).
   - Truy vấn không tạo node mới, chỉ đọc.
   - Với nhiều thao tác, cân nhắc pool/arena để giảm chi phí `new`.
*/

