/*
ICPC String Algorithms – Tập hợp gọn, kèm ghi chú sử dụng

Nội dung (độ phức tạp xây dựng):
- Z-Algorithm: O(n) – khớp mẫu nhanh bằng nối "pat#text"
- KMP (pi + search + automaton): O(n) – tìm tất cả vị trí khớp
- Manacher: O(n) – mọi bán kính palindrome (lẻ/chẵn)
- Minimal String Rotation (Booth): O(n) – chỉ số xoay từ điển nhỏ nhất
- Suffix Array (+ LCP Kasai): O(n log n) + O(n) – mảng suffix và LCP kề nhau
- Suffix Automaton (SAM): O(n) – kiểm tra chứa, đếm số substring khác nhau

Lưu ý:
- Tất cả hàm đều độc lập; không cần macro ngoài STL.
- Dùng `string` ASCII/byte. Nếu cần Unicode, tự ánh xạ về byte.
*/

#include <bits/stdc++.h>
using namespace std;

/* ============================ Z-ALGORITHM ============================ */
// Mô tả: z[i] = LCP giữa s và s[i..]. z[0] = 0 theo quy ước.
// Dùng để tìm mẫu: tính z trên P + '#' + T, mọi i có z[i] == |P| là vị trí khớp.
// ĐPT: Thời gian O(n); Bộ nhớ phụ O(1) (ngoài mảng z kích thước n)
static vector<int> z_algorithm(const string& s) {
    int n = (int)s.size();
    vector<int> z(n, 0);
    int l = 0, r = 0;
    for (int i = 1; i < n; ++i) {
        if (i <= r) z[i] = min(r - i + 1, z[i - l]);
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) ++z[i];
        if (i + z[i] - 1 > r) l = i, r = i + z[i] - 1;
    }
    return z;
}

// Tìm tất cả vị trí khớp của pattern trong text bằng Z.
// Trả về danh sách vị trí bắt đầu (0-based).
// ĐPT: Thời gian O(n + m); Bộ nhớ O(n + m) cho xâu ghép và mảng z
static vector<int> z_search(const string& text, const string& pat) {
    const char sep = '\x01'; // giả định không xuất hiện trong pat/text
    string s = pat + sep + text;
    vector<int> z = z_algorithm(s);
    vector<int> res;
    int m = (int)pat.size();
    for (int i = m + 1; i < (int)s.size(); ++i) if (z[i] >= m) res.push_back(i - (m + 1));
    return res;
}

/* ============================== KMP ================================== */
// prefix_function / pi: pi[i] = độ dài border dài nhất của s[0..i]
// ĐPT: Thời gian O(n); Bộ nhớ O(1) phụ (ngoài mảng pi kích thước n)
static vector<int> prefix_function(const string& s) {
    int n = (int)s.size();
    vector<int> pi(n, 0);
    for (int i = 1; i < n; ++i) {
        int j = pi[i - 1];
        while (j > 0 && s[i] != s[j]) j = pi[j - 1];
        if (s[i] == s[j]) ++j;
        pi[i] = j;
    }
    return pi;
}

// KMP search: tìm tất cả vị trí bắt đầu của pat trong text.
// ĐPT: Thời gian O(|text| + |pat|); Bộ nhớ O(|pat|) cho mảng pi
static vector<int> kmp_search(const string& text, const string& pat) {
    vector<int> res;
    if (pat.empty()) return res;
    vector<int> pi = prefix_function(pat);
    int j = 0;
    for (int i = 0; i < (int)text.size(); ++i) {
        while (j > 0 && text[i] != pat[j]) j = pi[j - 1];
        if (text[i] == pat[j]) ++j;
        if (j == (int)pat.size()) {
            res.push_back(i - j + 1);
            j = pi[j - 1];
        }
    }
    return res;
}

// KMP automaton: xây dựng tự động hữu hạn trên bảng chữ cái kích thước ALPHA,
// base là ký tự đầu (vd 'a'). aut[i][c] là trạng thái tiếp theo khi ở i và đọc c.
// ĐPT: Thời gian O(n * ALPHA); Bộ nhớ O(n * ALPHA)
static vector<vector<int>> kmp_automaton(const string& s, int ALPHA = 26, char base = 'a') {
    int n = (int)s.size();
    vector<int> pi = prefix_function(s);
    vector<vector<int>> aut(n, vector<int>(ALPHA, 0));
    for (int i = 0; i < n; ++i) {
        for (int c = 0; c < ALPHA; ++c) {
            char ch = char(base + c);
            if (i > 0 && ch != s[i]) aut[i][c] = aut[pi[i - 1]][c];
            else aut[i][c] = i + (ch == s[i]);
        }
    }
    return aut;
}

/* ============================ MANACHER =============================== */
// Mô tả: Trả về hai mảng bán kính palindrome trung tâm mỗi vị trí.
// - d1[i]: bán kính (số ký tự) palindrome lẻ (tâm tại i) – s[i-d1+1..i+d1-1]
// - d2[i]: bán kính palindrome chẵn (tâm giữa i-1 và i) – s[i-d2..i+d2-1]
// ĐPT: Thời gian O(n); Bộ nhớ O(1) phụ (ngoài 2 mảng đầu ra kích thước n)
static pair<vector<int>, vector<int>> manacher(const string& s) {
    int n = (int)s.size();
    vector<int> d1(n), d2(n);
    // odd length
    for (int i = 0, l = 0, r = -1; i < n; ++i) {
        int k = 1;
        if (i <= r) k = min(d1[l + r - i], r - i + 1);
        while (i - k >= 0 && i + k < n && s[i - k] == s[i + k]) ++k;
        d1[i] = k;
        if (i + k - 1 > r) l = i - k + 1, r = i + k - 1;
    }
    // even length
    for (int i = 0, l = 0, r = -1; i < n; ++i) {
        int k = 0;
        if (i <= r) k = min(d2[l + r - i + 1], r - i + 1);
        while (i - k - 1 >= 0 && i + k < n && s[i - k - 1] == s[i + k]) ++k;
        d2[i] = k;
        if (i + k - 1 > r) l = i - k, r = i + k - 1;
    }
    return {d1, d2};
}

/* ===================== MINIMAL STRING ROTATION ======================= */
// Booth algorithm: Trả về chỉ số bắt đầu của xoay từ điển nhỏ nhất của s.
// ĐPT: Thời gian O(n); Bộ nhớ O(n) cho mảng thất bại (failure)
static int minimal_rotation_index(const string& s) {
    int n = (int)s.size();
    if (n == 0) return 0;
    string ss = s + s;
    vector<int> f(2 * n, -1);
    int k = 0; // chỉ số bắt đầu tốt nhất hiện tại
    for (int j = 1; j < 2 * n; ++j) {
        int i = f[j - k - 1];
        while (i != -1 && ss[j] != ss[k + i + 1]) {
            if (ss[j] < ss[k + i + 1]) k = j - i - 1;
            i = f[i];
        }
        if (i == -1 && ss[j] != ss[k + i + 1]) {
            if (ss[j] < ss[k + i + 1]) k = j;
            f[j - k] = -1;
        } else {
            f[j - k] = i + 1;
        }
    }
    return k % n;
}

// Trả về chuỗi đã xoay nhỏ nhất theo từ điển.
// ĐPT: Thời gian O(n); Bộ nhớ O(n) cho kết quả trả về
static string minimal_rotation(const string& s) {
    if (s.empty()) return s;
    int k = minimal_rotation_index(s);
    return s.substr(k) + s.substr(0, k);
}

/* =========================== SUFFIX AUTOMATON ======================== */
// SAM (alphabet liên tiếp từ base, mặc định 'a'..'z').
// ĐPT & tính năng:
// - extend: O(1) trung bình/biến cố (amortized)
// - build(s): O(|s|)
// - contains(p): O(|p|)
// - countDistinct(): O(|states|) ~ O(|s|)
// - Bộ nhớ: O(|states| * ALPHA) với cài đặt mảng next kích thước ALPHA mỗi state
struct SuffixAutomaton {
    struct State {
        int link = -1, len = 0;
        // Dùng mảng cố định cho ALPHA nhỏ (vd 26). Nếu cần tổng quát, đổi sang unordered_map.
        vector<int> next;
        State() {}
        State(int alpha): next(alpha, -1) {}
    };

    int ALPHA;
    char base;
    vector<State> st;
    int last;

    SuffixAutomaton(int alpha = 26, char baseChar = 'a') : ALPHA(alpha), base(baseChar) {
        st.reserve(1);
        st.clear();
        st.push_back(State(ALPHA));
        st[0].link = -1; st[0].len = 0;
        last = 0;
    }

    inline int idx(char c) const { return (int)(c - base); }

    // ĐPT: O(1) amortized
    void extend(char ch) {
        int c = idx(ch);
        if (c < 0 || c >= ALPHA) {
            // Nếu ký tự ngoài alphabet, có thể mở rộng alphabet hoặc bỏ qua theo yêu cầu.
            // Ở đây ta coi như ký tự ngoài phạm vi không được hỗ trợ.
            return;
        }
        int cur = (int)st.size();
        st.push_back(State(ALPHA));
        st[cur].len = st[last].len + 1;
        int p = last;
        while (p != -1 && st[p].next[c] == -1) {
            st[p].next[c] = cur;
            p = st[p].link;
        }
        if (p == -1) {
            st[cur].link = 0;
        } else {
            int q = st[p].next[c];
            if (st[p].len + 1 == st[q].len) {
                st[cur].link = q;
            } else {
                int clone = (int)st.size();
                st.push_back(st[q]);
                st[clone].len = st[p].len + 1;
                // st[clone].next giữ nguyên, link giữ nguyên
                while (p != -1 && st[p].next[c] == q) {
                    st[p].next[c] = clone;
                    p = st[p].link;
                }
                st[q].link = st[cur].link = clone;
            }
        }
        last = cur;
    }

    // ĐPT: O(|s|)
    void build(const string& s) {
        for (char ch : s) extend(ch);
    }

    // ĐPT: O(|p|)
    bool contains(const string& p) const {
        int v = 0;
        for (char ch : p) {
            int c = (int)(ch - base);
            if (c < 0 || c >= ALPHA) return false;
            v = st[v].next[c];
            if (v == -1) return false;
        }
        return true;
    }

    // Số substring khác nhau = sum_{i>0} (len[i] - len[link[i]])
    // ĐPT: O(|states|)
    long long countDistinct() const {
        long long ans = 0;
        for (int v = 1; v < (int)st.size(); ++v) ans += st[v].len - st[st[v].link].len;
        return ans;
    }
};

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

/* ============================ GHI CHÚ SỬ DỤNG =========================
- Z:
  auto z = z_algorithm(s);
  auto occ = z_search(text, pat);

- KMP:
  auto pi = prefix_function(s);
  auto occ = kmp_search(text, pat);
  auto aut = kmp_automaton(pat); // tra cứu trạng thái khi đọc từng ký tự

- Manacher:
  auto [d1, d2] = manacher(s);
  // Palindrome lẻ tại i dài = 2*d1[i]-1, chẵn tại i dài = 2*d2[i]

- Minimal Rotation:
  int k = minimal_rotation_index(s);
  string t = minimal_rotation(s);

- Suffix Array + LCP:
  auto sa = suffix_array(s);
  auto lcp = lcp_array(s, sa);

- Suffix Automaton:
  SuffixAutomaton sam(26, 'a'); sam.build(s);
  bool ok = sam.contains(p);
  long long distinct = sam.countDistinct();
======================================================================== */
