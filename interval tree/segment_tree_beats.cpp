// Segment Tree Beats — Range chmin/chmax/add and range sum
// Gợi ý biên dịch:
// g++ -std=gnu++17 -O2 -pipe -Wall -Wextra "interval tree/segment_tree_beats.cpp" -o beats

#include <bits/stdc++.h>
using namespace std;

struct Node {
    long long sum = 0;
    long long add = 0; // lazy add cho toàn đoạn

    long long max1 = LLONG_MIN; // giá trị lớn nhất
    long long smax2 = LLONG_MIN; // giá trị lớn thứ 2
    int cnt_max = 0; // số phần tử đạt max1

    long long min1 = LLONG_MAX; // giá trị nhỏ nhất
    long long smin2 = LLONG_MAX; // giá trị nhỏ thứ 2
    int cnt_min = 0; // số phần tử đạt min1
};

struct SegTreeBeats {
    int n; vector<Node> st;
    SegTreeBeats(int n=0){ init(n); }
    SegTreeBeats(const vector<long long>& a){ build(a); }

    void init(int n_){ n = n_; st.assign(4*n+4, Node()); }

    static Node make_node(long long v){
        Node x; x.sum = v; x.add = 0;
        x.max1 = x.min1 = v;
        x.smax2 = LLONG_MIN; x.smin2 = LLONG_MAX;
        x.cnt_max = x.cnt_min = 1;
        return x;
    }

    static Node merge(const Node& L, const Node& R){
        Node x; x.add = 0;
        // sum
        x.sum = L.sum + R.sum;
        // max side
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
        // min side
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
        if (l==r){ st[p] = make_node(a[l]); return; }
        int m=(l+r)>>1; build(p<<1,l,m,a); build(p<<1|1,m+1,r,a);
        st[p]=merge(st[p<<1], st[p<<1|1]);
    }

    // apply helpers (không đẩy xuống con)
    void apply_add(int p, long long v, int len){
        Node &x = st[p];
        x.sum += v * len;
        x.max1 += v; if (x.smax2!=LLONG_MIN) x.smax2 += v;
        x.min1 += v; if (x.smin2!=LLONG_MAX) x.smin2 += v;
        x.add += v;
    }
    void apply_chmin(int p, long long x){ // pre: x < max1 && x > smax2
        Node &a = st[p];
        long long dec = a.max1 - x;
        a.sum -= dec * a.cnt_max;
        a.max1 = x;
        if (a.min1 == a.max1 + dec) { // tất cả phần tử bằng nhau trước đó
            a.min1 = x; // vẫn đồng nhất
        }
    }
    void apply_chmax(int p, long long x){ // pre: x > min1 && x < smin2
        Node &a = st[p];
        long long inc = x - a.min1;
        a.sum += inc * a.cnt_min;
        a.min1 = x;
        if (a.max1 == a.min1 - inc) { // đồng nhất trước đó
            a.max1 = x;
        }
    }

    void push(int p, int l, int r){
        if (l==r) return;
        int m = (l+r)>>1;
        int lc = p<<1, rc = p<<1|1;
        // Đẩy add trước
        if (st[p].add != 0){
            long long v = st[p].add; st[p].add = 0;
            apply_add(lc, v, m-l+1);
            apply_add(rc, v, r-m);
        }
        // Sau đó đẩy chmin/chmax (cắt đỉnh/đáy) xuống con
        long long capMax = st[p].max1;
        if (st[lc].max1 > capMax) apply_chmin(lc, capMax);
        if (st[rc].max1 > capMax) apply_chmin(rc, capMax);

        long long capMin = st[p].min1;
        if (st[lc].min1 < capMin) apply_chmax(lc, capMin);
        if (st[rc].min1 < capMin) apply_chmax(rc, capMin);
    }

    // Update: range add
    void range_add(int L, int R, long long v){ range_add(1,0,n-1,L,R,v); }
    void range_add(int p, int l, int r, int L, int R, long long v){
        if (R<l || r<L) return;
        if (L<=l && r<=R){ apply_add(p, v, r-l+1); return; }
        int m=(l+r)>>1;
        push(p,l,r);
        range_add(p<<1,l,m,L,R,v);
        range_add(p<<1|1,m+1,r,L,R,v);
        st[p]=merge(st[p<<1], st[p<<1|1]);
    }

    // Update: range chmin
    void range_chmin(int L, int R, long long x){ range_chmin(1,0,n-1,L,R,x); }
    void range_chmin(int p, int l, int r, int L, int R, long long x){
        if (R<l || r<L || x >= st[p].max1) return;
        if (L<=l && r<=R && x > st[p].smax2){
            apply_chmin(p, x); return;
        }
        int m=(l+r)>>1;
        push(p,l,r);
        range_chmin(p<<1,l,m,L,R,x);
        range_chmin(p<<1|1,m+1,r,L,R,x);
        st[p]=merge(st[p<<1], st[p<<1|1]);
    }

    // Update: range chmax
    void range_chmax(int L, int R, long long x){ range_chmax(1,0,n-1,L,R,x); }
    void range_chmax(int p, int l, int r, int L, int R, long long x){
        if (R<l || r<L || x <= st[p].min1) return;
        if (L<=l && r<=R && x < st[p].smin2){
            apply_chmax(p, x); return;
        }
        int m=(l+r)>>1;
        push(p,l,r);
        range_chmax(p<<1,l,m,L,R,x);
        range_chmax(p<<1|1,m+1,r,L,R,x);
        st[p]=merge(st[p<<1], st[p<<1|1]);
    }

    // Query: range sum
    long long range_sum(int L, int R){ return range_sum(1,0,n-1,L,R); }
    long long range_sum(int p, int l, int r, int L, int R){
        if (R<l || r<L) return 0;
        if (L<=l && r<=R) return st[p].sum;
        int m=(l+r)>>1;
        push(p,l,r);
        return range_sum(p<<1,l,m,L,R) + range_sum(p<<1|1,m+1,r,L,R);
    }
};

#ifdef LOCAL_TEST
int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int n = 8; vector<long long> a = {5, 4, 3, 2, 1, 6, 7, 8};
    SegTreeBeats st(a);
    // chmin [0,4] by 3 => {3,3,3,2,1,6,7,8}
    st.range_chmin(0,4,3);
    cout << st.range_sum(0,7) << "\n"; // 33
    // chmax [2,6] by 5 => {3,3,5,5,5,6,7,8}
    st.range_chmax(2,6,5);
    cout << st.range_sum(0,7) << "\n"; // 42
    // add [1,3] += 2 => {3,5,7,7,5,6,7,8}
    st.range_add(1,3,2);
    cout << st.range_sum(0,7) << "\n"; // 48
    return 0;
}
#endif
