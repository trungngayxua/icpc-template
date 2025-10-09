#include <bits/stdc++.h>
using namespace std;

/*
===============================================================================
                              Li Chao Tree
===============================================================================

PURPOSE:
    Dynamic structure for storing linear functions f(x) = a*x + b.
    Supports queries for min or max f(x) at any point x.

TIME COMPLEXITY:
    - addLine(a,b):  O(log Range)
    - query(x):      O(log Range)
SPACE:
    - O(#lines * log Range)

USAGE:
    - Can handle both min and max queries (set Compare = std::less or std::greater)
    - Works with integer or floating-point coordinates
===============================================================================
*/

template <typename T, typename Compare = std::less<T>>
class LiChaoTree {
private:
    struct Line {
        T a, b; // y = a*x + b
        Line(T _a = 0, T _b = 0) : a(_a), b(_b) {}
        T get(T x) const { return a * x + b; }
    };

    struct Node {
        Line line;
        Node *left, *right;
        Node(Line _line) : line(_line), left(nullptr), right(nullptr) {}
    };

    Node* root;
    T L, R;
    Compare cmp;     // comparator: less -> min, greater -> max
    T neutral;       // neutral element (+∞ for min, −∞ for max)

    // helper: return true if new value is better than old
    inline bool better(T x, T y) const { return cmp(x, y); }

    // choose the better value according to comparator
    inline T select(T a, T b) const { return better(a, b) ? a : b; }

    void addLine(Node *&node, T l, T r, Line newLine) {
        if (!node) { node = new Node(newLine); return; }
        T mid = (l + r) >> 1;

        bool leftBetter = better(newLine.get(l), node->line.get(l));
        bool midBetter  = better(newLine.get(mid), node->line.get(mid));

        if (midBetter) swap(newLine, node->line);
        if (l == r) return;

        if (leftBetter != midBetter)
            addLine(node->left, l, mid, newLine);
        else
            addLine(node->right, mid + 1, r, newLine);
    }

    // void addSegment(Node *&node, long long l, long long r, long long Lq, long long Rq, Line newLine) {
    //     if (r < Lq || Rq < l) return;          // đoạn hiện tại không giao với đoạn line
    //     if (Lq <= l && r <= Rq) {              // đoạn hiện tại nằm trọn trong [Lq, Rq]
    //         addLine(node, l, r, newLine);      // gọi logic Li Chao Tree chuẩn
    //         return;
    //     }
    //     long long mid = (l + r) >> 1;
    //     addSegment(node->left, l, mid, Lq, Rq, newLine);
    //     addSegment(node->right, mid + 1, r, Lq, Rq, newLine);
    // }

    T query(Node *node, T l, T r, T x) const {
        if (!node) return neutral;
        T res = node->line.get(x);
        if (l == r) return res;
        T mid = (l + r) >> 1;
        if (x <= mid) return select(res, query(node->left, l, mid, x));
        else          return select(res, query(node->right, mid + 1, r, x));
    }

public:
    // Constructor: choose mode (min or max) by Compare
    LiChaoTree(T _L, T _R, Compare _cmp = Compare())
        : L(_L), R(_R), root(nullptr), cmp(_cmp) {
        if constexpr (std::is_same_v<Compare, std::less<T>>) 
            neutral = numeric_limits<T>::max();     // min mode
        else 
            neutral = numeric_limits<T>::lowest();  // max mode
    }

    // Add a line y = a*x + b
    void addLine(T a, T b) { addLine(root, L, R, Line(a, b)); }

    // Query min/max at a point x
    T query(T x) const { return query(root, L, R, x); }
};

/*
===============================================================================
                               DOCUMENTATION
===============================================================================

🔹 STRUCTURE OVERVIEW
    LiChaoTree<T, Compare> tree(L, R);
    - Supports both min and max mode:
        LiChaoTree<long long, std::less<long long>>   → find min
        LiChaoTree<long long, std::greater<long long>>→ find max
    - Works for integral or floating-point x.

===============================================================================
🔹 CORE FUNCTIONS

  addLine(a, b)
      Inserts a line f(x) = a*x + b into the structure.
      O(log Range).

  query(x)
      Returns min/max f(x) over all lines at given x.
      O(log Range).

===============================================================================
🔹 COMBINATIONS / USE-CASES

1️⃣ Dynamic Convex Hull Trick
     - Maintain lower or upper envelope of lines.
     - Use Compare=less for min, Compare=greater for max.

2️⃣ DP Convex Optimization
     dp[i] = min_j(dp[j] + a[j]*b[i])
     ⇒ Add line y = a[j]*x + dp[j], query at x = b[i].

3️⃣ Segment Li Chao Tree (range insertion)
     - If each line only valid on [xL, xR], addLine over segment nodes.

4️⃣ Convex Cost / Profit Problems
     - Find minimal cost / maximal profit among multiple linear models.

5️⃣ Continuous or Discrete
     - Works for discrete x if integer.
     - For floating-point domain: use double and stop when r−l < eps.

===============================================================================
🔹 COMPLEXITY SUMMARY
     addLine: O(log (R − L))
     query:   O(log (R − L))
     Space:   O(#lines * log Range)

===============================================================================
🔹 EXAMPLE
     LiChaoTree<long long, std::less<long long>> minTree(-1e6, 1e6);
     minTree.addLine(2, 3);       // y = 2x + 3
     minTree.addLine(-1, 10);     // y = -x + 10
     cout << minTree.query(4);    // => min(11, 6) = 6

     LiChaoTree<long long, std::greater<long long>> maxTree(-1e6, 1e6);
     maxTree.addLine(2, 3);
     maxTree.addLine(-1, 10);
     cout << maxTree.query(4);    // => max(11, 6) = 11
===============================================================================
*/

int main() {
    // Example: Min Li Chao Tree
    LiChaoTree<long long, std::less<long long>> minTree(-10, 10);
    minTree.addLine(2, 3);   // y = 2x + 3
    minTree.addLine(-1, 10); // y = -x + 10
    cout << "Min query(4): " << minTree.query(4) << "\n";

    // Example: Max Li Chao Tree
    LiChaoTree<long long, std::greater<long long>> maxTree(-10, 10);
    maxTree.addLine(2, 3);
    maxTree.addLine(-1, 10);
    cout << "Max query(4): " << maxTree.query(4) << "\n";
}