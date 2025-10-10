# Segment Tree “Bậc Thang” (Segment Tree Beats) — Ghi chú nhanh (VI)

Tài liệu này tóm tắt cách xử lý các truy vấn “bậc thang” trên mảng bằng Segment Tree Beats — kỹ thuật chuẩn cho các phép cập nhật dạng chặn đỉnh/đáy khiến dãy trở thành bậc thang sau nhiều lần áp dụng.

## Khi nào dùng “bậc thang”

- Bài có một hoặc nhiều phép cập nhật dạng:
  - Range chmin: với mọi i ∈ [l, r], a[i] = min(a[i], x)
  - Range chmax: với mọi i ∈ [l, r], a[i] = max(a[i], x)
  - Có thể kèm range add (+= v), và truy vấn sum/min/max.
- Số truy vấn lớn (≈ 2e5), giá trị lớn (≈ 1e9), không thể “dumb push” từng phần tử.
- Sau nhiều lần chmin/chmax, đồ thị giá trị theo chỉ số xuất hiện dạng “bậc thang” — vì ta chỉ hạ/nhấc đỉnh.

## Ý tưởng cốt lõi (Beats)

- Với mỗi nút (đoạn), lưu các đặc trưng:
  - max1: giá trị lớn nhất; smax2: giá trị lớn thứ 2; cnt_max: số phần tử đạt max1
  - min1: giá trị nhỏ nhất; smin2: giá trị nhỏ thứ 2; cnt_min: số phần tử đạt min1
  - sum: tổng đoạn; add: lazy cộng đều toàn đoạn
- Quy tắc “chặt nhánh” nhanh:
  - Chmin(x) cho đoạn: nếu x ≥ max1 → không đổi.
  - Nếu smax2 < x < max1 → giảm tất cả phần tử đang = max1 xuống x (không cần xuống con). Cập nhật sum theo cnt_max.
  - Nếu x ≤ smax2 → phải đẩy xuống con (push) vì đoạn có nhiều “đỉnh” khác nhau.
  - Chmax tương tự nhưng cho min1/smin2/cnt_min.
- Range add: cộng đều, cập nhật sum, max1/smax2/min1/smin2.
- Truy vấn sum/min/max: lấy từ nút.

## Độ phức tạp

- Mỗi phần tử chỉ “đổi bậc” hữu hạn lần trước khi bị kẹp vào smax2/smin2; tổng thời gian amortized O((n + q) log n).
- Thực tế chạy nhanh cho n, q ≈ 2e5.

## Khi nào cần đủ cả min & max

- Nếu chỉ có chmin + sum: chỉ cần theo dõi mặt “max” (max1/smax2/cnt_max) là đủ.
- Nếu chỉ có chmax + sum: chỉ cần theo dõi mặt “min”.
- Nếu có cả chmin và chmax (và/hoặc add): cần theo dõi cả hai mặt để đảm bảo cắt nhánh đúng.

## Bố cục node và bất biến

- sum = tổng các phần tử của đoạn.
- max1 ≥ smax2 và min1 ≤ smin2. Nếu tất cả phần tử bằng nhau: smax2 = -INF, smin2 = +INF.
- cnt_max (số phần tử đạt max1) và cnt_min tương ứng.
- add là lazy cộng đều toàn đoạn (kết hợp được với chmin/chmax bằng cách đẩy add xuống trước).

## Mẹo triển khai an toàn

- Xài 64-bit cho sum (long long). Giá trị phần tử nếu có add lớn cũng nên 64-bit.
- INF: dùng giá trị an toàn, ví dụ `const long long INF = (1LL<<60);`
- Khi `apply_add`, nhớ cộng cho cả max1, smax2 (nếu khác -INF), min1, smin2 (nếu khác +INF).
- `push` thứ tự: đẩy add trước, sau đó đẩy chmin/chmax (so sánh với max1/min1 của con để quyết định).
- Luôn đảm bảo bất biến sau mỗi thao tác: smax2 < max1 và smin2 > min1, cnt_* chuẩn.
- Nếu chỉ cần 1 kiểu truy vấn (chmin hoặc chmax), lược bỏ nửa còn lại để code gọn và nhanh hơn.

## Các dạng bài hay gặp

- Range chmin + Range sum (kinh điển: “Gorgeous Sequence”): cần theo dõi mặt max.
- Range chmax + Range sum: đối xứng với chmin.
- Range chmin + Range chmax + Range add + Range sum: full beats, bài AtCoder Library Practice.
- Đếm/so sánh đối với ngưỡng: có thể suy ra từ min/max sau khi kẹp.

## Bẫy thường gặp

- Quên cập nhật sum khi “giảm” max1 (hoặc “tăng” min1).
- Quên xét smax2/smin2 là ±INF khi đoạn đồng nhất.
- Sai thứ tự push: không đẩy add trước → sai dữ liệu khi kẹp.
- Dùng int cho sum → tràn số.

## Mã tham khảo nhanh

- Xem file `interval tree/segment_tree_beats.cpp:1` để có cài đặt đầy đủ cho các phép: chmin, chmax, add, sum (kèm ví dụ dùng trong `main`).

## Khi không cần Beats

- Nếu q nhỏ (≤ 5e4) và chỉ có 1 phép cập nhật (chmin hoặc chmax) trên mảng nhỏ, có thể dùng sqrt-decomposition hoặc treap.
- Nếu chỉ cần truy vấn offline với điều kiện đơn giản, cân nhắc phân rã offline + sort + Fenwick.

---

Gợi ý biên dịch (ví dụ file tham khảo):

`g++ -std=gnu++17 -O2 -pipe -Wall -Wextra "interval tree/segment_tree_beats.cpp" -o beats`

