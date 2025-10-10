/**
 * @file sweep-line.cpp
 * @brief Sweep Line — Guidelines and Pitfalls (Guide-first)
 *
 * Mục tiêu
 * - Tài liệu hướng dẫn (thiên về guideline, ít/không code) để thiết kế thuật toán sweep line chắc tay.
 * - Tập trung vào: tổ chức event, thứ tự xử lý, cấu trúc dữ liệu active, pitfall thường gặp, mẹo tối ưu.
 * - Dẫn chiếu các mẫu sẵn có trong repo: ví dụ Union-length theo trục y có thể tái dùng SegCover ở `interval-set.cpp:1`.
 *
 * Khi nào dùng sweep line
 * - Hình học phẳng: tìm giao đoạn/đoạn, diện tích/hợp hình chữ nhật, chu vi, giao điểm, nearest pair (biến thể với cửa sổ), angle sweep.
 * - Dữ liệu 2D tổng quát: xử lý offline theo một trục, dùng Fenwick/Segtree trên trục còn lại (đếm điểm trong hình chữ nhật, max/min theo miền,…).
 * - 1D timeline: số khoảng phủ tối đa, tổng chiều dài hợp, overlap, minimum rooms (interval partitioning), đoạn với trọng số.
 *
 * Mẫu hình cơ bản
 * 1) Chuẩn hoá input: đảm bảo l<r, y1<y2; bỏ phần tử suy biến nếu cần.
 * 2) Tạo danh sách event theo trục sweep (x hoặc t). Mỗi event chứa: toạ độ, loại (enter/leave), payload.
 * 3) Sắp xếp event: theo toạ độ tăng; ràng buộc tie-break rõ ràng (xem bên dưới).
 * 4) Duy trì "active set" (BST/multiset/segment tree/Fenwick) của các phần tử đang mở tại vị trí sweep hiện tại.
 * 5) Cập nhật kết quả:
 *    - Kiểu tích luỹ (area/length): cộng phần đóng góp = aggregator * (cur_coord - prev_coord) trước/hoặc sau khi áp event, thống nhất quy ước.
 *    - Kiểu tối ưu/đếm: cập nhật khi thêm/bớt vào active theo logic bài.
 * 6) Lặp: tiến prev_coord=cur_coord, áp toàn bộ event cùng toạ độ, update active, tiếp tục.
 *
 * Thiết kế event và tie-break
 * - Đại số nửa mở: ưu tiên quy ước [l, r) để loại trừ chồng biên ảo. Khi đó event thường là: (+1 tại l), (-1 tại r).
 * - Thứ tự cùng toạ độ (x == cur):
 *   - Diện tích hợp hình chữ nhật: area += cover_y * (x - prev_x) trước, rồi áp tất cả event tại x để chuẩn bị cho đoạn [x, next_x).
 *   - Chu vi: cần cẩn thận event order vì cạnh chia sẻ; thường xử lý "đóng" trước "mở" hoặc ngược lại theo quy ước mặt mở [l, r).
 *   - Đếm overlap 1D: với [l,r), khi sort theo điểm: cộng +1 ở l, trừ 1 ở r; lấy max prefix → số overlap tối đa.
 * - Comparator phải toàn phần và ổn định về mặt toán học; với số thực, tránh so sánh mơ hồ (xem Geometry pitfall).
 *
 * Lựa chọn Active Set
 * - 1D hợp chiều dài: dùng segment tree nén toạ độ (SegCover trong `interval-set.cpp`) để lưu chiều dài phủ và counter.
 * - Đếm điểm trong hình chữ nhật: quét theo x, active là Fenwick theo y (cập nhật +1/-1 cho điểm, trả lời query theo y-range).
 * - Giao điểm đoạn thẳng: BST theo thứ tự y tại x hiện tại (Bentley–Ottmann), cần comparator phụ thuộc x_sweep.
 * - Bài tối ưu (max/min) trên miền: active là multiset/heap nếu chỉ cần giá trị biên; segtree nếu cần query đoạn.
 *
 * Pitfall chung (rất hay gặp)
 * - Inclusive/Exclusive mập mờ: dùng [l, r) nhất quán. Event "mở" tại l, "đóng" tại r.
 * - Tie-break sai: cùng x mà xử lý mở/đóng lẫn lộn → double-count hoặc bỏ sót. Ghi rõ quy ước và giữ nhất quán.
 * - Toạ độ lớn: tràn 32-bit; dùng 64-bit cho toạ độ/tích (x * chiều dài). Với diện tích, dùng 128-bit nếu ngôn ngữ hỗ trợ.
 * - Số thực: so sánh bằng EPS; comparator phụ thuộc x dễ mất tính bắc cầu → sắp xếp thất bại. Ưu tiên số học nguyên/nén toạ độ khi có thể.
 * - Dữ liệu suy biến: đoạn rỗng (l==r), cạnh trùng/điểm trùng; chuẩn hoá trước khi tạo event.
 * - Nhầm cập nhật aggregator: quên pull/push (nếu dùng segtree lazy) hoặc quên reset khi đi qua mốc.
 * - Xử lý theo nhóm event: quên gom tất cả event cùng toạ độ vào một batch trước khi tiến sweep.
 * - Hiệu năng: active không phù hợp (map khi cần segtree) gây TLE; dư event (chưa nén toạ độ) tốn bộ nhớ.
 *
 * Mẹo hữu ích
 * - Nén toạ độ: giảm bộ nhớ, áp dụng phép toán nguyên, tránh sai số. Luôn lưu bản "giá trị thật" để tính chênh lệch (dx, dy).
 * - Batch event theo toạ độ: dùng chỉ số i chạy qua blocks cùng x để update 1 lần.
 * - Dùng nửa mở [l, r) để loại bỏ corner-case; khi input đóng [L, R], đổi về [L, R+1).
 * - Tách bài khó: area/length qua quét + SegCover; perimeter qua đếm thay đổi phủ dọc/ ngang ở cạnh.
 * - Logging kiểm thử: in (prev_x, cur_x, cover_len) cho vài bước đầu để so sánh với lời giải ngây thơ.
 * - Với hình học: dùng long double cho tính y tại x; always guard với EPS trong so sánh thứ tự.
 *
 * Pattern tiêu biểu
 * - Union Area Rectangles (trục song song):
 *   Event tại x: edges { [y1,y2), delta=+1/-1 }. Quét theo x tăng:
 *     area += cover_y * (x - prev_x);
 *     áp tất cả delta vào SegCover(Y);
 *     prev_x = x.
 *   Lưu ý: chuẩn hoá [y1,y2) với y1<y2; bỏ hình rỗng.
 * - Union Perimeter Rectangles:
 *   Quét dọc và ngang tách biệt: mỗi lần áp event, đóng góp vào chu vi là số đoạn mới mở/đóng (đếm chuyển trạng thái trong SegCover).
 * - Max Overlap 1D / Minimum Rooms:
 *   Sắp event (time, type) với type: end trước start hoặc dùng [l,r) + prefix; lấy peak.
 * - Offline 2D Points-In-Rectangle:
 *   Biến mỗi query [x1,x2)×[y1,y2) thành 2 event tại x2 và x1, cộng/ trừ Fenwick(y) theo điểm; sau đó lấy hiệu.
 * - Segment Intersections (Bentley–Ottmann):
 *   Event gồm endpoint và giao điểm; status là BST theo thứ tự y tại x hiện tại; kiểm tra cặp láng giềng khi chèn/loại.
 *   Cực kỳ nhạy với EPS và comparator, ưu tiên số học nguyên khi đoạn trục song song.
 *
 * Checklist nhanh trước khi submit
 * - [ ] Dùng [l, r) nhất quán; đã chuẩn hoá input?
 * - [ ] Event sort có tie-break rõ ràng? (ví dụ: end trước start hoặc ngược lại theo quy ước)
 * - [ ] Gom batch cùng toạ độ? cập nhật area += cover * dx trước khi áp event?
 * - [ ] Active set đúng loại? (Fenwick/Segtree/BST) và đã kiểm soát độ phức tạp?
 * - [ ] Tránh overflow: dùng long long / __int128 cho tích.
 * - [ ] Với hình học: dùng EPS; comparator không vi phạm tính bắc cầu.
 * - [ ] Đã test trên case nhỏ, ngẫu nhiên, biên (trùng cạnh, chạm điểm)?
 *
 * Tiny reference snippets (chỉ tham khảo — giữ ở dạng bình luận)
 *
 * // 1) 1D max overlap bằng event + prefix
 * // vector<pair<long long,int>> ev; // (pos, +1/-1)
 * // sort(ev.begin(), ev.end());
 * // long long cur=0, best=0; for(auto [x,d]:ev){ cur+=d; best=max(best,cur);} 
 *
 * // 2) Quét hợp chiều dài theo x, dùng SegCover(Y) (xem interval-set.cpp)
 * // struct Edge{ long long x,y1,y2; int d; };
 * // sort(edges by x);
 * // long long area=0, prev=edges[0].x; SegCover seg(Ys);
 * // for(i over edges by blocks with same x){
 * //   long long x = edges[i].x; area += seg.covered() * (x - prev);
 * //   while(j in block) seg.cover(edges[j].y1, edges[j].y2, edges[j].d), ++j;
 * //   prev = x;
 * // }
 */

#pragma once
#include <bits/stdc++.h>
using namespace std;

// File này cố ý không cung cấp template cài đặt đầy đủ.
// Mục tiêu là guideline. Tham khảo cài đặt SegCover tại: interval-set.cpp

