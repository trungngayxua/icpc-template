#include <bits/stdc++.h>
using namespace std;

class SegmentTree {
private:
    int n;
    vector<long long> tree;
    vector<long long> data;

    inline long long merge(long long a, long long b) const {
        return a + b;
    }

    void build(int node, int l, int r) {
        if (l == r) {
            tree[node] = data[l];
            return;
        }
        int mid = (l + r) >> 1;
        build(node << 1, l, mid);
        build(node << 1 | 1, mid + 1, r);
        tree[node] = merge(tree[node << 1], tree[node << 1 | 1]);
    }

    void update(int node, int l, int r, int idx, long long val) {
        if (l == r) {
            tree[node] = val;
            return;
        }
        int mid = (l + r) >> 1;
        if (idx <= mid) update(node << 1, l, mid, idx, val);
        else update(node << 1 | 1, mid + 1, r, idx, val);
        tree[node] = merge(tree[node << 1], tree[node << 1 | 1]);
    }

    long long query(int node, int l, int r, int ql, int qr) const {
        if (qr < l || r < ql) return 0; // neutral cho sum
        if (ql <= l && r <= qr) return tree[node];
        int mid = (l + r) >> 1;
        return merge(query(node << 1, l, mid, ql, qr),
                     query(node << 1 | 1, mid + 1, r, ql, qr));
    }

public:
    explicit SegmentTree(const vector<long long> &arr) {
        n = (int)arr.size() - 1; // arr[1..n]
        data = arr;
        tree.assign(4 * (n + 2), 0);
        build(1, 1, n);
    }

    void update(int idx, long long val) {
        update(1, 1, n, idx, val);
    }

    long long query(int L, int R) const {
        return query(1, 1, n, L, R);
    }

    void printTree() const {
        cout << "Tree: ";
        for (int i = 1; i <= 2 * n; i++) cout << tree[i] << ' ';
        cout << "\n";
    }
};

/*
===============================================================================
                                DOCUMENTATION
===============================================================================
🔹 DỮ LIỆU:
    tree[] : lưu giá trị segment (size ~ 4N)
    data[] : mảng gốc (đánh chỉ số 1..N)
===============================================================================
🔹 PHƯƠNG THỨC:
  SegmentTree(arr)
      Xây cây từ mảng arr (phải là 1-indexed, arr[0] bỏ trống)
===============================================================================
🔹 ĐỔI PHÉP TOÁN:
  Để tạo min/max/xor segment tree, chỉ cần sửa merge():
      inline long long merge(long long a, long long b) const {
          return min(a, b); // hoặc max(a,b), a^b,...
      }
  Và sửa giá trị neutral trong query() (VD: INF, -INF, 0,...)
===============================================================================
🔹 VÍ DỤ:
    vector<long long> a = {0, 1, 2, 3, 4, 5}; // bỏ a[0]
    SegmentTree seg(a);
    cout << seg.query(2, 4) << "\n"; // 2+3+4 = 9
    seg.update(3, 10);
    cout << seg.query(2, 4) << "\n"; // 2+10+4 = 16
===============================================================================
*/