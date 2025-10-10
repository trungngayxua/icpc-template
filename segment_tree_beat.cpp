// Segment Tree “Bậc Thang” (Segment Tree Beats — chmin + sum)
//
// Mục tiêu
// - Xử lý cập nhật chặn đỉnh (range chmin): a[i] = min(a[i], x) trên đoạn [l, r]
// - Truy vấn tổng (range sum) nhanh.
// - Dùng cho các bài “truy vấn bậc thang” (giảm các đỉnh cao xuống mức x), q ~ 2e5.
//
// Ý tưởng cốt lõi (Beats — nhánh cắt nhanh)
// - Mỗi node lưu: sum, max1 (lớn nhất), smax2 (lớn nhì), cnt_max (số phần tử đạt max1).
// - Với chmin(x) trên đoạn:
//   + Nếu x >= max1: không đổi.
//   + Nếu smax2 < x < max1: hạ TẤT CẢ phần tử = max1 xuống x (không cần xuống con), cập nhật sum -= (max1-x)*cnt_max, max1=x.
//   + Nếu x <= smax2: buộc phải xuống con.
// - Truy vấn sum: chuẩn O(log n), nhưng cần push cap từ cha xuống con khi đi sâu.
//
// Độ phức tạp
// - Amortized O((n + q) log n). Mỗi phần tử chỉ “đổi bậc” hữu hạn lần trước khi bị kẹp vào smax2.
//
// Bẫy thường gặp & mẹo
// - Dùng 64-bit cho sum (long long).
// - Khởi tạo smax2 = -INF tại lá (đoạn đồng nhất).
// - Khi push (đi xuống), nếu con.max1 > cha.max1 thì apply_chmin(con, cha.max1).
// - Nếu bài chỉ cần chmin + sum, không cần theo dõi min-side → đơn giản, nhanh.
// - Mở rộng: có thể thêm chmax + sum hoặc add + sum (full beats) nếu đề yêu cầu.
//
// Biên dịch gợi ý:
// g++ -std=gnu++17 -O2 -pipe -Wall -Wextra segment_tree_bac_thang.cpp -o seg

#include <bits/stdc++.h>
using namespace std;

struct Node {
    long long sum;    // tổng đoạn
    long long max1;   // giá trị lớn nhất trong đoạn
    long long smax2;  // giá trị lớn nhì trong đoạn
    int cnt_max;      // số phần tử đạt max1
    Node(long long v=0){
        sum = v; max1 = v; smax2 = LLONG_MIN; cnt_max = 1;
    }
};

struct SegTreeChminSum {
    int n; vector<Node> st;
    SegTreeChminSum(int n=0){ init(n); }
    SegTreeChminSum(const vector<long long>& a){ build(a); }

    void init(int n_){ n=n_; st.assign(4*n+4, Node()); }

    static Node merge(const Node& L, const Node& R){
        Node x; x.sum = L.sum + R.sum;
        if (L.max1 > R.max1){
            x.max1 = L.max1; x.cnt_max = L.cnt_max;
            x.smax2 = max(L.smax2, R.max1);
        } else if (L.max1 < R.max1){
            x.max1 = R.max1; x.cnt_max = R.cnt_max;
            x.smax2 = max(R.smax2, L.max1);
        } else {
            x.max1 = L.max1; x.cnt_max = L.cnt_max + R.cnt_max;
            x.smax2 = max(L.smax2, R.smax2);
        }
        return x;
    }

    void build(const vector<long long>& a){ init((int)a.size()); build(1,0,n-1,a); }
    void build(int p, int l, int r, const vector<long long>& a){
        if (l==r){ st[p] = Node(a[l]); return; }
        int m=(l+r)>>1; build(p<<1,l,m,a); build(p<<1|1,m+1,r,a);
        st[p] = merge(st[p<<1], st[p<<1|1]);
    }

    // Hạ đỉnh max1 về x (tiền điều kiện: smax2 < x < max1)
    inline void apply_chmin(int p, long long x){
        long long old = st[p].max1;
        if (x >= old) return;
        st[p].sum -= (old - x) * 1LL * st[p].cnt_max;
        st[p].max1 = x; // smax2 giữ nguyên
    }

    // Đẩy cap từ cha xuống con để đảm bảo khi đi sâu thì con không vi phạm cap của cha
    inline void push(int p){
        int lc=p<<1, rc=p<<1|1;
        long long cap = st[p].max1;
        if (st[lc].max1 > cap) apply_chmin(lc, cap);
        if (st[rc].max1 > cap) apply_chmin(rc, cap);
    }

    void range_chmin(int L, int R, long long x){ range_chmin(1,0,n-1,L,R,x); }
    void range_chmin(int p, int l, int r, int L, int R, long long x){
        if (R<l || r<L || x >= st[p].max1) return;
        if (L<=l && r<=R && x > st[p].smax2){ apply_chmin(p, x); return; }
        int m=(l+r)>>1; push(p);
        range_chmin(p<<1, l, m, L, R, x);
        range_chmin(p<<1|1, m+1, r, L, R, x);
        st[p] = merge(st[p<<1], st[p<<1|1]);
    }

    long long range_sum(int L, int R){ return range_sum(1,0,n-1,L,R); }
    long long range_sum(int p, int l, int r, int L, int R){
        if (R<l || r<L) return 0;
        if (L<=l && r<=R) return st[p].sum;
        int m=(l+r)>>1; push(p);
        return range_sum(p<<1,l,m,L,R) + range_sum(p<<1|1,m+1,r,L,R);
    }
};

#if 1
// Biến thể đối xứng: chmax + sum (nâng đáy)
// Theo dõi min1/smin2/cnt_min (tương tự max-side ở trên).
struct NodeMin {
    long long sum;
    long long min1;
    long long smin2;
    int cnt_min;
    NodeMin(long long v=0){ sum=v; min1=v; smin2=LLONG_MAX; cnt_min=1; }
};

struct SegTreeChmaxSum {
    int n; vector<NodeMin> st;
    SegTreeChmaxSum(int n=0){ init(n); }
    SegTreeChmaxSum(const vector<long long>& a){ build(a); }

    void init(int n_){ n=n_; st.assign(4*n+4, NodeMin()); }

    static NodeMin merge(const NodeMin& L, const NodeMin& R){
        NodeMin x; x.sum = L.sum + R.sum;
        if (L.min1 < R.min1){
            x.min1 = L.min1; x.cnt_min = L.cnt_min;
            x.smin2 = min(L.smin2, R.min1);
        } else if (L.min1 > R.min1){
            x.min1 = R.min1; x.cnt_min = R.cnt_min;
            x.smin2 = min(R.smin2, L.min1);
        } else {
            x.min1 = L.min1; x.cnt_min = L.cnt_min + R.cnt_min;
            x.smin2 = min(L.smin2, R.smin2);
        }
        return x;
    }

    void build(const vector<long long>& a){ init((int)a.size()); build(1,0,n-1,a); }
    void build(int p, int l, int r, const vector<long long>& a){
        if (l==r){ st[p] = NodeMin(a[l]); return; }
        int m=(l+r)>>1; build(p<<1,l,m,a); build(p<<1|1,m+1,r,a);
        st[p] = merge(st[p<<1], st[p<<1|1]);
    }

    inline void apply_chmax(int p, long long x){ // pre: smin2 > x > min1
        long long old = st[p].min1;
        if (x <= old) return;
        st[p].sum += (x - old) * 1LL * st[p].cnt_min;
        st[p].min1 = x; // smin2 giữ nguyên
    }

    inline void push(int p){
        int lc=p<<1, rc=p<<1|1;
        long long floorv = st[p].min1;
        if (st[lc].min1 < floorv) apply_chmax(lc, floorv);
        if (st[rc].min1 < floorv) apply_chmax(rc, floorv);
    }

    void range_chmax(int L, int R, long long x){ range_chmax(1,0,n-1,L,R,x); }
    void range_chmax(int p, int l, int r, int L, int R, long long x){
        if (R<l || r<L || x <= st[p].min1) return;
        if (L<=l && r<=R && x < st[p].smin2){ apply_chmax(p, x); return; }
        int m=(l+r)>>1; push(p);
        range_chmax(p<<1, l, m, L, R, x);
        range_chmax(p<<1|1, m+1, r, L, R, x);
        st[p] = merge(st[p<<1], st[p<<1|1]);
    }

    long long range_sum(int L, int R){ return range_sum(1,0,n-1,L,R); }
    long long range_sum(int p, int l, int r, int L, int R){
        if (R<l || r<L) return 0;
        if (L<=l && r<=R) return st[p].sum;
        int m=(l+r)>>1; push(p);
        return range_sum(p<<1,l,m,L,R) + range_sum(p<<1|1,m+1,r,L,R);
    }
};
#endif
#if 1
// ================================================================
// Segment Tree — Range cộng cấp số cộng (v, v+1, v+2, …)
// Mục tiêu: cập nhật đoạn [l, r] với giá trị tăng dần theo chỉ số, rồi truy vấn tổng.
//   - Update: với mọi i∈[l,r], a[i] += v + (i - l)
//     (Tổng quát hơn: a[i] += v + d*(i - l) với step d)
//   - Query: range sum
// Độ phức tạp: O(log n) mỗi thao tác.
//
// Ý tưởng cốt lõi
// - Nhận xét: v + (i - l) = 1*i + (v - l). Tức là “hàm tuyến tính theo i”.
// - Ta dùng segment tree lazy có 2 tag: add_i (hệ số theo i) và add_c (hằng số).
// - Khi đoạn [L,R] được cộng “alpha*i + beta” cho mọi i∈[L,R], thì tổng đoạn tăng:
//     alpha * sum_{i=L..R} i + beta * len, với len = (R-L+1).
// - Khi lazy đẩy xuống con, ta giữ nguyên alpha/beta (vì i là chỉ số tuyệt đối), chỉ khác nhau là sum_i của mỗi con.
//
// Cách áp dụng cho bài v, v+1, v+2, …
// - Update [l, r] thêm (v + (i-l)) => alpha = 1, beta = v - l.
// - Nếu step d: alpha = d, beta = v - d*l.
//
// Mẹo cho contest
// - Gán cấp số cộng (set thay vì add): thêm flag “cover_linear” (alpha, beta) để ghi đè, và khi push cần clear các add trước đó.
// - Nếu chỉ cần prefix-based (i tính từ đầu mảng), dùng đúng công thức trên là đủ; không cần “i-L”.
// - Có thể thay segment tree bằng 2 Fenwick/BIT khi chỉ cần cộng tuyến tính (alpha*i + beta) và hỏi tổng: rất gọn, nhưng tree dễ tổng quát hóa.
//
// Dưới đây là cài đặt tham khảo (add AP + range sum):

struct SegTreeAP {
    int n; 
    struct Node { long long sum, add_i, add_c; };
    vector<Node> st;
    SegTreeAP(int n=0){ init(n); }
    SegTreeAP(const vector<long long>& a){ build(a); }

    void init(int n_){ n = n_; st.assign(4*n+4, {0,0,0}); }

    static inline long long sum_i(int L, int R){ // sum of i for i=L..R
        long long len = R - L + 1LL;
        return (long long)(L + R) * len / 2;
    }

    void build(const vector<long long>& a){ init((int)a.size()); build(1,0,n-1,a); }
    void build(int p, int L, int R, const vector<long long>& a){
        if (L==R){ st[p].sum = a[L]; return; }
        int M=(L+R)>>1; build(p<<1,L,M,a); build(p<<1|1,M+1,R,a);
        st[p].sum = st[p<<1].sum + st[p<<1|1].sum;
    }

    inline void apply(int p, int L, int R, long long alpha, long long beta){
        long long len = R-L+1LL;
        st[p].sum += alpha * sum_i(L,R) + beta * len;
        st[p].add_i += alpha; st[p].add_c += beta;
    }

    inline void push(int p, int L, int R){
        long long ai = st[p].add_i, ac = st[p].add_c;
        if (ai==0 && ac==0) return;
        int M=(L+R)>>1; int lc=p<<1, rc=p<<1|1;
        apply(lc, L, M, ai, ac);
        apply(rc, M+1, R, ai, ac);
        st[p].add_i = st[p].add_c = 0;
    }

    // Cộng “alpha*i + beta” lên [l, r]
    void range_add_linear(int l, int r, long long alpha, long long beta){ range_add_linear(1,0,n-1,l,r,alpha,beta); }
    void range_add_linear(int p, int L, int R, int l, int r, long long alpha, long long beta){
        if (r<L || R<l) return;
        if (l<=L && R<=r){ apply(p,L,R,alpha,beta); return; }
        push(p,L,R);
        int M=(L+R)>>1; 
        range_add_linear(p<<1, L, M, l, r, alpha, beta);
        range_add_linear(p<<1|1, M+1, R, l, r, alpha, beta);
        st[p].sum = st[p<<1].sum + st[p<<1|1].sum;
    }

    // Cộng cấp số cộng (step=1): a[i] += v + (i - l) cho i∈[l,r]
    void range_add_ap(int l, int r, long long v){
        long long alpha = 1; 
        long long beta = v - l; // v + (i-l) = 1*i + (v-l)
        range_add_linear(l,r,alpha,beta);
    }

    // Tổng quát: step d (a[i] += v + d*(i - l))
    void range_add_ap_step(int l, int r, long long v, long long d){
        long long alpha = d; 
        long long beta = v - d*1LL*l;
        range_add_linear(l,r,alpha,beta);
    }

    long long range_sum(int l, int r){ return range_sum(1,0,n-1,l,r); }
    long long range_sum(int p, int L, int R, int l, int r){
        if (r<L || R<l) return 0;
        if (l<=L && R<=r) return st[p].sum;
        push(p,L,R);
        int M=(L+R)>>1;
        return range_sum(p<<1,L,M,l,r) + range_sum(p<<1|1,M+1,R,l,r);
    }
};

#ifdef LOCAL_TEST_AP
int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int n = 8; vector<long long> a(n, 0);
    SegTreeAP st(a);
    // Cộng [2,6] với v=5, step=1 => + (5,6,7,8,9)
    st.range_add_ap(2,6,5);
    cout << st.range_sum(0,7) << "\n"; // 35
    // Cộng [0,3] với v=1, step=2 => + (1,3,5,7)
    st.range_add_ap_step(0,3,1,2);
    // Mảng lúc này: [1,3,5,7,0,0,0,0] + [0,0,5,6,7,8,9,0] => [1,3,10,13,7,8,9,0]
    cout << st.range_sum(0,7) << "\n"; // 51
    cout << st.range_sum(2,4) << "\n"; // 30 (10+13+7)
    return 0;
}
#endif
#endif
#ifdef LOCAL_TEST
int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int n = 8; vector<long long> a = {5, 4, 3, 2, 1, 6, 7, 8};
    SegTreeChminSum st(a);
    // chmin [0,4] = 3 => {3,3,3,2,1,6,7,8}
    st.range_chmin(0,4,3);
    cout << st.range_sum(0,7) << "\n"; // 33
    // chmin [2,6] = 5 => {3,3,3,2,1,5,5,5}
    st.range_chmin(2,6,5);
    cout << st.range_sum(0,7) << "\n"; // 27
    return 0;
}
#endif
