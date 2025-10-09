/**
 * @file dp_optimizations.cpp
 * @brief DP optimization templates: D&C (monotone), Knuth, WQS, and CDQ
 *
 * What’s inside (straightforward functions, no heavy wrappers)
 * - Divide & Conquer Optimization (Monotone opt): one-layer solver
 * - Knuth Optimization (interval DP): O(N^2) solver
 * - WQS (Aliens trick): parametric search skeleton
 * - CDQ Divide & Conquer: generic recursion
 * - See also: Convex Hull Trick in `Template/cht.cpp:1`
 *
 * Conventions
 * - Use 0-based or 1-based consistently. Knuth section uses 1-based.
 * - Prefer `long long` for costs; set INF = 4e18.
 * - "EDIT HERE" comments mark places to customize quickly during contests.
 */

#pragma once
#include <bits/stdc++.h>
using namespace std;

using ll = long long;
static constexpr ll INF64 = (ll)4e18;

/* ========================================================================
 * Divide & Conquer Optimization (Monotone opt)
 * Form: dp_cur[i] = min_{0 <= j < i} { dp_prev[j] + C(j, i) }
 * Assumption: argmin j for i is non-decreasing in i (decision monotonicity).
 * Complexity per layer: O(N log N) for many costs; O(N) if cost queried O(1).
 * Indexing: 0-based (i in [0..N-1], j < i). You may shift to 1-based if you prefer.
 * How to use: set dp_prev, implement cost(j,i), then call dc_run_one_layer.
 * Constraints: choose ll for costs; ensure cost only called with j<i.
 * EDIT HERE: implement cost(j,i) for your problem below.
 * ====================================================================== */

// EDIT HERE: implement your C(j,i). Precondition: 0 <= j < i < N.
// Suggested: precompute prefix sums so this is O(1).
static inline ll dc_cost(int j, int i){
    // Example placeholder: quadratic gap cost. Replace for your problem.
    // If you need prefix sums: make them global/static and use here.
    // return pref[i] - pref[j];
    return (ll)(i - j) * (i - j);
}

// Internal recursive solve for [l..r] with optimal j in [optL..optR]
static void dc_compute(int l, int r, int optL, int optR,
                       const vector<ll>& dp_prev, vector<ll>& dp_cur){
    if(l > r) return;
    int m = (l + r) >> 1;
    pair<ll,int> best = {INF64, max(0, min(m - 1, optL))};
    int start = optL;
    int finish = min(m - 1, optR);
    for(int j = start; j <= finish; ++j){
        ll v = dp_prev[j] + dc_cost(j, m);
        if(v < best.first){ best = {v, j}; }
    }
    dp_cur[m] = best.first;
    int opt = best.second;
    dc_compute(l, m - 1, optL, opt, dp_prev, dp_cur);
    dc_compute(m + 1, r, opt, optR, dp_prev, dp_cur);
}

// Run a single layer: fills dp_cur from dp_prev.
// N is taken from dp_prev.size(). dp_cur resized to N and filled with INF64.
static void dc_run_one_layer(const vector<ll>& dp_prev, vector<ll>& dp_cur){
    int N = (int)dp_prev.size();
    dp_cur.assign(N, INF64);
    // If your dp starts at i=1 with j<i, you can set bounds to [1..N-1]
    if(N >= 1) dc_compute(0, N - 1, 0, N - 1, dp_prev, dp_cur);
}

/* ========================================================================
 * Knuth Optimization (interval DP, 1-based)
 * Form: dp[i][j] = min_{i <= k < j} { dp[i][k] + dp[k+1][j] } + w(i,j)
 * Assumptions (both needed):
 *   - Quadrangle inequality / Monge-like structure on w
 *   - opt monotonicity: opt[i][j-1] <= opt[i][j] <= opt[i+1][j]
 * Complexity: O(N^2). Indexing: i,j in [1..N]. Base: dp[i][i]=0, opt[i][i]=i.
 * How to use: implement w(i,j); precompute any prefix sums needed.
 * EDIT HERE: implement w(i,j) for your problem below.
 * ====================================================================== */

// EDIT HERE: implement interval cost w(i,j) on [1..N]
static inline ll knuth_w(int i, int j){
    // Example placeholder using prefix sums S: cost of [i..j] = S[j]-S[i-1]
    // return S[j] - S[i-1];
    (void)i; (void)j; // silence unused warning if unedited
    return 0;
}

// Fills dp and opt. dp/opt must be sized (N+2)x(N+2) or will be resized.
static void knuth_solve(int N, vector<vector<ll>>& dp, vector<vector<int>>& opt){
    dp.assign(N + 2, vector<ll>(N + 2, 0));
    opt.assign(N + 2, vector<int>(N + 2, 0));
    for(int i = 1; i <= N; ++i){ dp[i][i] = 0; opt[i][i] = i; }
    for(int len = 2; len <= N; ++len){
        for(int i = 1; i + len - 1 <= N; ++i){
            int j = i + len - 1;
            dp[i][j] = INF64;
            int s = opt[i][j - 1];
            int e = opt[i + 1][j];
            if(s < i) s = i;
            if(e > j - 1) e = j - 1;
            for(int k = s; k <= e; ++k){
                ll v = dp[i][k] + dp[k + 1][j] + knuth_w(i, j);
                if(v < dp[i][j]){ dp[i][j] = v; opt[i][j] = k; }
            }
        }
    }
}

/* ========================================================================
 * WQS (Aliens Trick) — parametric search on lambda
 * Replace per-part cost with penalty lambda, solve unconstrained DP.
 * parts(lambda) must be non-increasing as lambda increases.
 * ====================================================================== */
/* ========================================================================
 * WQS (Aliens Trick) — parametric search on lambda (λ)
 * Use when maximizing/minimizing with a constraint on number of parts/segments.
 * Idea: subtract λ per part in DP; parts(λ) is monotone (non-increasing as λ increases).
 * How to use: implement solve_lambda(λ) to return {value_minus_lambda*parts, parts}.
 * Choose search bounds [lam_lo, lam_hi] wide enough for your data.
 * EDIT HERE: implement solve_lambda(λ) for your problem below.
 * ====================================================================== */

struct WQSResult { ll val; int parts; };

// EDIT HERE: implement this DP under penalty lam (λ). Return value already minus lam*parts.
static WQSResult wqs_solve_lambda(ll lam){
    // Example placeholder: replace with your unconstrained DP under penalty lam
    // Return best value (already minus lam*parts) and number of parts used
    (void)lam;
    return {0, 0};
}

// Binary search λ for exactly/at-least K parts (typical variant: maximize with parts >= K)
// Returns pair(real_answer, chosen_lambda)
static pair<ll,ll> wqs_maximize_with_K(int K, ll lam_lo, ll lam_hi){
    WQSResult best = {-(INF64/4), 0};
    ll bestLam = lam_lo;
    ll lo = lam_lo, hi = lam_hi;
    while(lo <= hi){
        ll mid = (lo + hi) >> 1;
        WQSResult r = wqs_solve_lambda(mid);
        if(r.parts >= K){
            if(r.val > best.val){ best = r; bestLam = mid; }
            lo = mid + 1; // increase penalty to reduce parts
        } else hi = mid - 1;
    }
    ll real_ans = best.val + (ll)K * bestLam; // recover original objective
    return {real_ans, bestLam};
}

/* ========================================================================
 * CDQ Divide & Conquer skeleton
 * Use when transitions from earlier indices influence later indices and
 * contributions can be accumulated in order (often with BIT/Seg/FFT).
 * ====================================================================== */
/* ========================================================================
 * CDQ Divide & Conquer — generic skeleton
 * Use when transitions from earlier indices influence later indices, and
 * contributions can be accumulated in order (often with BIT/Seg/FFT).
 * How to use: implement combine(l, m, r) to apply effects of [l..m] onto [m+1..r].
 * Typical pattern: sort events by key, sweep with BIT to update right half queries.
 * ====================================================================== */

template<class Combine>
static void cdq(int l, int r, Combine combine){
    if(l >= r) return;
    int m = (l + r) >> 1;
    cdq(l, m, combine);
    combine(l, m, r); // apply cross influence left -> right
    cdq(m + 1, r, combine);
}

/* ---------------- Example usage (commented) ------------------
int main(){
    // Example usage snippets (comment/uncomment for quick testing)

    // --- D&C layer ---
    {
        int N = 10; // EDIT HERE: set size
        vector<ll> dp_prev(N, 0), dp_cur;
        // EDIT HERE: precompute any prefix sums used by dc_cost()
        dc_run_one_layer(dp_prev, dp_cur);
        // dp_cur now holds dp for this layer
    }

    // --- Knuth ---
    {
        int N = 8; // EDIT HERE: set interval size
        // EDIT HERE: precompute any arrays used by knuth_w(i,j)
        vector<vector<ll>> dp; vector<vector<int>> opt;
        knuth_solve(N, dp, opt);
        // dp[1][N] is the answer if problem matches Knuth assumptions
    }

    // --- WQS ---
    {
        int K = 3; // EDIT HERE: target number of parts/segments
        ll lam_lo = -1000, lam_hi = 1000; // EDIT HERE: choose adequate bounds
        auto [ans, lam] = wqs_maximize_with_K(K, lam_lo, lam_hi);
        (void)ans; (void)lam;
    }

    // --- CDQ ---
    {
        int n = 16; // EDIT HERE: range size
        auto combine = [&](int l, int m, int r){
            // EDIT HERE: apply effects of [l..m] to [m+1..r]
            // e.g., sort pointers by key, use BIT to accumulate contributions
        };
        cdq(0, n - 1, combine);
    }
}
------------------------------------------------*/
