// Ordered Set (PBDS) — mô tả ngắn
// Mục đích: set có thứ tự, hỗ trợ truy cập theo chỉ số k.
// Độ phức tạp: mọi thao tác chính O(log n).
// API chính:
//   - insert(x), erase(x)
//   - order_of_key(x): đếm bao nhiêu phần tử < x
//   - find_by_order(k): phần tử thứ k (0-based)
// Đếm trong đoạn [l, r]: order_of_key(r+1) - order_of_key(l)
// Lưu ý: không có phần tử trùng; cần -std=gnu++17; MSVC không hỗ trợ PBDS.
// Biên dịch: g++ -std=gnu++17 -O2 -pipe -Wall -Wextra docs/cheatsheet_cp_vi.cpp -o main

#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
using namespace std;
using namespace __gnu_pbds;

template <class T>
using ordered_set = tree<T, null_type, less<T>, rb_tree_tag, tree_order_statistics_node_update>;

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);

    ordered_set<int> os; // không lưu trùng
    for (int x: {3,1,4,1,5,9}) os.insert(x); // -> {1,3,4,5,9}

    cout << os.order_of_key(5) << "\n";   // 3 (1,3,4)
    cout << *os.find_by_order(2) << "\n"; // 4

    return 0;
}
