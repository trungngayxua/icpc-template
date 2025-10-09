// Matrix Multiplication (0-indexed, dynamic size, modulo)
// CP-friendly: identity, multiply, fast exponent, vector multiply

#include <bits/stdc++.h>
using namespace std;

constexpr long long MOD = 1000000007LL; // đổi tuỳ bài (vd: 998244353)

struct Mat {
    int n;
    vector<vector<long long>> a; // 0-indexed: a[i][j]

    Mat(int n = 0, bool ident = false) : n(n), a(n, vector<long long>(n, 0)) {
        if (ident) {
            for (int i = 0; i < n; i++) a[i][i] = 1;
        }
    }

    static Mat identity(int n) { return Mat(n, true); }

    Mat& operator*=(const Mat& o) {
        assert(n == o.n);
        Mat r(n);
        for (int i = 0; i < n; i++) {
            const auto& Ai = a[i];
            auto& Ri = r.a[i];
            for (int k = 0; k < n; k++) {
                long long Aik = Ai[k];
                if (!Aik) continue;
                const auto& Bk = o.a[k];
                unsigned long long mul = 0;
                for (int j = 0; j < n; j++) {
                    if (!Bk[j]) continue;
                    Ri[j] += (Aik * Bk[j]) % MOD;
                    if (Ri[j] >= MOD) Ri[j] -= MOD;
                }
            }
        }
        return (*this = r);
    }

    friend Mat operator*(Mat l, const Mat& r) { l *= r; return l; }

    Mat pow(long long e) const {
        Mat base = *this;
        Mat res = identity(n);
        while (e > 0) {
            if (e & 1) res = res * base;
            base = base * base;
            e >>= 1;
        }
        return res;
    }

    vector<long long> mulVec(const vector<long long>& v) const {
        assert((int)v.size() == n);
        vector<long long> res(n, 0);
        for (int i = 0; i < n; i++) {
            unsigned long long sum = 0;
            for (int j = 0; j < n; j++) {
                if (!a[i][j]) continue;
                sum += (unsigned long long)a[i][j] * (unsigned long long)v[j] % MOD;
                if (sum >= (1ull << 62)) sum %= MOD; // tránh tràn tạm thời
            }
            res[i] = (long long)(sum % MOD);
        }
        return res;
    }
};

/*
================================================================================
                                  DOCUMENTATION
================================================================================

MỤC ĐÍCH
- Phiên bản dynamic size, 0-indexed, có modulo: dễ tái sử dụng cho CP.
- Dùng cho truy hồi tuyến tính, đếm đường đi độ dài k, chuyển trạng thái k bước…

API
- Mat(n, ident=false): tạo ma trận n x n (0-index), tuỳ chọn đơn vị.
- Mat::identity(n): ma trận đơn vị.
- operator* / operator*= : nhân ma trận (modulo), O(n^3).
- pow(e): lũy thừa ma trận A^e, O(n^3 log e).
- mulVec(v): nhân ma trận với vector kích thước n, trả về vector mới.

CÁCH DÙNG NHANH
- Fibonacci n:
    Mat T(2);
    T.a[0][0] = 1; T.a[0][1] = 1;
    T.a[1][0] = 1; T.a[1][1] = 0;
    Mat P = T.pow(n); // P = T^n
    // Nếu vector cột ban đầu x0 = [F1, F0]^T, thì x_n = P * x0, F_n = x_n[1].

- Số đường đi độ dài k trên đồ thị n đỉnh:
    Mat A(n);
    // A[u][v] = số cung u->v (thường 0/1)
    Mat Ak = A.pow(k); // Ak[u][v] = số đường đi độ dài k từ u đến v

GỢI Ý TỐI ƯU
- Đổi MOD tuỳ bài (1e9+7, 998244353, ...).
- Nếu MOD lớn và tích có thể vượt 1e18, cân nhắc dùng __int128 khi nhân.
- Với kích thước nhỏ cố định, có thể dùng std::array<array<long long,N>,N> để nhanh hơn.

ĐỘ PHỨC TẠP
- Nhân: O(n^3)
- Lũy thừa: O(n^3 log e)
================================================================================
*/

