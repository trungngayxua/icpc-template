#include <bits/stdc++.h>
using namespace std;

/*
===============================================================================
                            Suffix Array (O(N log N))
===============================================================================

Builds the suffix array SA for string s.
SA[i] = starting index of the i-th lexicographically smallest suffix.

Algorithm: "Prefix Doubling" with counting sort
Time:  O(N log N)
Space: O(N)
===============================================================================
*/

struct SuffixArray {
    vector<int> sa, rank, lcp;

    // Build SA in O(N log N)
    SuffixArray(const string &s) {
        int n = s.size();
        sa.resize(n); rank.resize(n);
        for (int i = 0; i < n; i++) sa[i] = i, rank[i] = s[i];

        vector<int> tmp(n);
        for (int k = 1; k < n; k <<= 1) {
            auto cmp = [&](int i, int j) {
                if (rank[i] != rank[j]) return rank[i] < rank[j];
                int ri = i + k < n ? rank[i + k] : -1;
                int rj = j + k < n ? rank[j + k] : -1;
                return ri < rj;
            };
            sort(sa.begin(), sa.end(), cmp);     // radix/counting sort version -> O(N)
            tmp[sa[0]] = 0;
            for (int i = 1; i < n; i++)
                tmp[sa[i]] = tmp[sa[i-1]] + cmp(sa[i-1], sa[i]);
            rank = tmp;
            if (rank[sa[n-1]] == n-1) break;     // all ranks unique
        }

        buildLCP(s);
    }

    // Kasai algorithm — O(N)
    void buildLCP(const string &s) {
        int n = s.size();
        lcp.assign(n-1, 0);
        int h = 0;
        for (int i = 0; i < n; i++) {
            int r = rank[i];
            if (r == n-1) { h = 0; continue; }
            int j = sa[r+1];
            while (i+h < n && j+h < n && s[i+h] == s[j+h]) h++;
            lcp[r] = h;
            if (h) h--;
        }
    }
};

/*
===============================================================================
                                DOCUMENTATION
===============================================================================

🔹 SuffixArray SA(s);
    - SA.sa[i]   → chỉ số bắt đầu của suffix nhỏ thứ i
    - SA.rank[i] → thứ tự từ điển của suffix bắt đầu tại i
    - SA.lcp[i]  → độ dài prefix chung giữa SA[i] và SA[i+1]

===============================================================================
🔹 COMBINATIONS / USE-CASES

1️⃣  Inverse SA
     rank[i] = vị trí của suffix i trong thứ tự từ điển.

2️⃣  LCP Range Minimum Query
     - Dùng Sparse Table để lấy LCP giữa 2 suffix bất kỳ.
     - Ứng dụng: so sánh nhanh 2 substring, longest repeat, pattern matching.

3️⃣  Pattern Search
     - Dùng binary search trên SA:
         lower_bound(pattern), upper_bound(pattern)
     - Complexity: O(|pattern| log N)

4️⃣  Longest Repeated Substring
     - Kết quả = max(lcp)
     - Vị trí: sa[idx] và sa[idx+1]

5️⃣  Longest Common Substring (giữa 2 chuỗi)
     - Ghép s = s1 + '#' + s2 + '$'
     - Xây SA + LCP
     - Tìm max(LCP[i]) mà SA[i], SA[i+1] thuộc 2 chuỗi khác nhau.

6️⃣  Substring Count
     - Tổng số substring khác nhau = N*(N+1)/2 − Σ LCP[i]

7️⃣  Dùng cho nén dữ liệu (Burrows–Wheeler Transform),
     bioinformatics (so sánh genome), plagiarism check, pattern index.

===============================================================================
*/