#include "pos.h"

/* ============================================================
   Tiện ích nhập liệu
   ============================================================ */

static void clrscr(void) {
#if defined(_WIN32)
    system("cls");
#else
    system("clear");
#endif
}

static int getInt(const char *prompt, int min, int max) {
    char buf[64];
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(buf, sizeof buf, stdin)) return min;
        trimStr(buf);
        char *end = NULL;
        long v = strtol(buf, &end, 10);
        if (end != buf && *end == '\0' && v >= min && v <= max)
            return (int)v;
        printf("  (!) Vui lòng nhập số nguyên từ %d đến %d.\n", min, max);
    }
}

static long getLong(const char *prompt, long min, long max) {
    char buf[64];
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(buf, sizeof buf, stdin)) return min;
        trimStr(buf);
        char *end = NULL;
        long v = strtol(buf, &end, 10);
        if (end != buf && *end == '\0' && v >= min && v <= max)
            return v;
        printf("  (!) Vui lòng nhập số nguyên hợp lệ.\n");
    }
}

static void getString(const char *prompt, char *out, int size) {
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(out, size, stdin)) {
        out[0] = '\0';
        return;
    }
    trimStr(out);
}

static void getStringOrKeep(const char *prompt, char *val, int size) {
    char buf[MAX_LINE];
    printf("%s [%s]: ", prompt, val);
    fflush(stdout);
    if (!fgets(buf, sizeof buf, stdin)) return;
    trimStr(buf);
    if (*buf) copyStr(val, buf, size);
}

static long getLongOrKeep(const char *prompt, long cur) {
    char buf[64];
    printf("%s [%ld]: ", prompt, cur);
    fflush(stdout);
    if (!fgets(buf, sizeof buf, stdin)) return cur;
    trimStr(buf);
    if (!*buf) return cur;
    char *end = NULL;
    long v = strtol(buf, &end, 10);
    if (end != buf && *end == '\0') return v;
    printf("  (!) Giữ nguyên giá trị cũ.\n");
    return cur;
}

static int confirm(const char *prompt) {
    char buf[8];
    for (;;) {
        printf("%s (y/n): ", prompt);
        fflush(stdout);
        if (!fgets(buf, sizeof buf, stdin)) return 0;
        trimStr(buf);
        if (buf[0] == 'y' || buf[0] == 'Y') return 1;
        if (buf[0] == 'n' || buf[0] == 'N') return 0;
    }
}

static void pauseEnter(void) {
    printf("\nNhấn Enter để tiếp tục...");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* bỏ qua */ }
}

/* ============================================================
   Hiển thị sản phẩm
   ============================================================ */

static int findProductById(const SanPham *list, int n, const char *id) {
    for (int i = 0; i < n; i++)
        if (strcmp(list[i].id, id) == 0) return i;
    return -1;
}

static void nextProductId(const SanPham *list, int n, char *out, int size) {
    int max = 0;
    for (int i = 0; i < n; i++) {
        int v = atoi(list[i].id + 2); /* bỏ "SP" */
        if (v > max) max = v;
    }
    snprintf(out, size, "SP%03d", max + 1);
}

static void printProductHeader(void) {
    printf("%-8s %-28s %-14s %-7s %11s %11s %7s\n",
           "Mã SP", "Tên sản phẩm", "Nhóm", "Đơn vị", "Giá nhập", "Giá bán", "Tồn kho");
}

static void printProductRow(const SanPham *p) {
    char cost[16], price[16];
    formatMoney(p->cost, cost, sizeof cost);
    formatMoney(p->price, price, sizeof price);
    printf("%-8s %-28s %-14s %-7s %11s %11s %7d\n",
           p->id, p->name, p->category, p->unit, cost, price, p->stock);
}

/* ============================================================
   Quản lý sản phẩm
   ============================================================ */

static void addProduct(SanPham *list, int *n) {
    if (*n >= MAX_PRODUCTS) {
        printf("Danh sách sản phẩm đã đầy.\n");
        return;
    }
    SanPham p;
    memset(&p, 0, sizeof p);
    nextProductId(list, *n, p.id, sizeof p.id);
    printf("Mã sản phẩm (tự động): %s\n", p.id);
    getString("Tên sản phẩm: ", p.name, sizeof p.name);
    getString("Nhóm hàng: ", p.category, sizeof p.category);
    getString("Đơn vị: ", p.unit, sizeof p.unit);
    p.cost = getLong("Giá nhập (VND): ", 0, 100000000000L);
    p.price = getLong("Giá bán (VND): ", 0, 100000000000L);
    p.stock = getInt("Tồn kho: ", 0, 1000000);

    list[*n] = p;
    (*n)++;
    saveProducts(list, *n);
    printf("Đã thêm sản phẩm %s.\n", p.id);
}

static void editProduct(SanPham *list, int n) {
    if (n == 0) { printf("Chưa có sản phẩm nào.\n"); return; }
    char id[12];
    getString("Nhập mã sản phẩm cần sửa: ", id, sizeof id);
    int idx = findProductById(list, n, id);
    if (idx < 0) { printf("Không tìm thấy sản phẩm '%s'.\n", id); return; }

    SanPham *p = &list[idx];
    printf("\nĐang sửa: %s - %s (nhập trống để giữ nguyên)\n", p->id, p->name);
    getStringOrKeep("Tên sản phẩm", p->name, sizeof p->name);
    getStringOrKeep("Nhóm hàng", p->category, sizeof p->category);
    getStringOrKeep("Đơn vị", p->unit, sizeof p->unit);
    p->cost = getLongOrKeep("Giá nhập (VND)", p->cost);
    p->price = getLongOrKeep("Giá bán (VND)", p->price);
    p->stock = (int)getLongOrKeep("Tồn kho", p->stock);

    saveProducts(list, n);
    printf("Đã cập nhật sản phẩm %s.\n", p->id);
}

static void deleteProduct(SanPham *list, int *n) {
    if (*n == 0) { printf("Chưa có sản phẩm nào.\n"); return; }
    char id[12];
    getString("Nhập mã sản phẩm cần xóa: ", id, sizeof id);
    int idx = findProductById(list, *n, id);
    if (idx < 0) { printf("Không tìm thấy sản phẩm '%s'.\n", id); return; }
    if (!confirm("Bạn chắc chắn muốn xóa sản phẩm này?")) return;

    for (int i = idx; i < *n - 1; i++)
        list[i] = list[i + 1];
    (*n)--;
    saveProducts(list, *n);
    printf("Đã xóa sản phẩm %s.\n", id);
}

static void searchProducts(const SanPham *list, int n) {
    char kw[MAX_LINE];
    printf("Tìm theo tên hoặc mã (Enter = tất cả): ");
    fflush(stdout);
    if (!fgets(kw, sizeof kw, stdin)) return;
    trimStr(kw);

    printProductHeader();
    int found = 0;
    for (int i = 0; i < n; i++) {
        const SanPham *p = &list[i];
        if (*kw && !containsIgnoreCase(p->name, kw) && !containsIgnoreCase(p->id, kw))
            continue;
        printProductRow(p);
        found++;
    }
    if (!found) printf("(Không tìm thấy sản phẩm nào)\n");
}

static void menuSanPham(SanPham *list, int *n) {
    for (;;) {
        clrscr();
        printf("================ QUẢN LÝ SẢN PHẨM ================\n");
        printf("  [1] Xem danh sách sản phẩm\n");
        printf("  [2] Thêm sản phẩm\n");
        printf("  [3] Sửa sản phẩm\n");
        printf("  [4] Xóa sản phẩm\n");
        printf("  [5] Tìm kiếm\n");
        printf("  [0] Quay lại\n");
        int c = getInt("Chọn: ", 0, 5);
        if (c == 0) break;
        else if (c == 1) {
            printProductHeader();
            for (int i = 0; i < *n; i++) printProductRow(&list[i]);
            if (*n == 0) printf("(Chưa có sản phẩm nào)\n");
        }
        else if (c == 2) addProduct(list, n);
        else if (c == 3) editProduct(list, *n);
        else if (c == 4) deleteProduct(list, n);
        else if (c == 5) searchProducts(list, *n);
        pauseEnter();
    }
}

/* ============================================================
   Bán hàng
   ============================================================ */

static void printCart(const HoaDon *cart) {
    if (cart->itemCount == 0) {
        printf("Giỏ hàng trống.\n");
        return;
    }
    printf("%-3s %-8s %-28s %11s %5s %13s\n",
           "STT", "Mã SP", "Tên sản phẩm", "Đơn giá", "SL", "Thành tiền");
    long total = 0;
    for (int i = 0; i < cart->itemCount; i++) {
        const MatHang *m = &cart->items[i];
        char dg[16], tt[16];
        formatMoney(m->price, dg, sizeof dg);
        formatMoney(m->price * m->qty, tt, sizeof tt);
        printf("%-3d %-8s %-28s %11s %5d %13s\n",
               i + 1, m->id, m->name, dg, m->qty, tt);
        total += m->price * m->qty;
    }
    char t[32];
    formatMoney(total, t, sizeof t);
    printf("Tổng cộng: %s VND\n", t);
}

static void addToCart(const SanPham *products, int prodCount, HoaDon *cart) {
    char kw[MAX_LINE];
    printf("Tìm theo tên hoặc mã (Enter = xem tất cả còn hàng): ");
    fflush(stdout);
    if (!fgets(kw, sizeof kw, stdin)) return;
    trimStr(kw);

    printf("%-8s %-28s %11s %7s\n", "Mã SP", "Tên", "Giá bán", "Tồn kho");
    int shown = 0;
    for (int i = 0; i < prodCount; i++) {
        const SanPham *p = &products[i];
        if (p->stock <= 0) continue;
        if (*kw && !containsIgnoreCase(p->name, kw) && !containsIgnoreCase(p->id, kw))
            continue;
        char price[16];
        formatMoney(p->price, price, sizeof price);
        printf("%-8s %-28s %11s %7d\n", p->id, p->name, price, p->stock);
        shown++;
    }
    if (shown == 0) {
        printf("Không tìm thấy sản phẩm nào còn hàng.\n");
        return;
    }

    char id[12];
    getString("Nhập mã sản phẩm muốn bán: ", id, sizeof id);
    int idx = findProductById(products, prodCount, id);
    if (idx < 0) { printf("Không tìm thấy sản phẩm '%s'.\n", id); return; }
    if (products[idx].stock <= 0) { printf("Sản phẩm '%s' đã hết hàng.\n", id); return; }

    int maxQty = products[idx].stock;
    int qty = getInt("Số lượng: ", 1, maxQty);

    int existing = -1;
    for (int i = 0; i < cart->itemCount; i++)
        if (strcmp(cart->items[i].id, id) == 0) { existing = i; break; }

    if (existing >= 0) {
        if (cart->items[existing].qty + qty > maxQty) {
            printf("Vượt quá tồn kho (%d).\n", maxQty);
            return;
        }
        cart->items[existing].qty += qty;
    } else {
        if (cart->itemCount >= MAX_ITEMS) {
            printf("Giỏ hàng đã đầy.\n");
            return;
        }
        MatHang *m = &cart->items[cart->itemCount++];
        memset(m, 0, sizeof *m);
        copyStr(m->id, products[idx].id, sizeof m->id);
        copyStr(m->name, products[idx].name, sizeof m->name);
        m->price = products[idx].price;
        m->qty = qty;
    }
    printf("Đã thêm '%s' x%d vào giỏ hàng.\n", products[idx].name, qty);
}

static void editCartItem(const SanPham *products, int prodCount, HoaDon *cart) {
    if (cart->itemCount == 0) { printf("Giỏ hàng trống.\n"); return; }
    printCart(cart);
    int idx = getInt("Chọn số thứ tự (STT) muốn sửa: ", 1, cart->itemCount) - 1;
    MatHang *m = &cart->items[idx];
    int pIdx = findProductById(products, prodCount, m->id);
    int maxQty = (pIdx >= 0) ? products[pIdx].stock : m->qty;
    int qty = getInt("Số lượng mới: ", 1, maxQty);
    m->qty = qty;
    printf("Đã cập nhật số lượng '%s' thành %d.\n", m->name, qty);
}

static void removeCartItem(HoaDon *cart) {
    if (cart->itemCount == 0) { printf("Giỏ hàng trống.\n"); return; }
    printCart(cart);
    int idx = getInt("Chọn số thứ tự (STT) muốn xóa: ", 1, cart->itemCount) - 1;
    for (int i = idx; i < cart->itemCount - 1; i++)
        cart->items[i] = cart->items[i + 1];
    cart->itemCount--;
    printf("Đã xóa mặt hàng khỏi giỏ.\n");
}

static void nextInvoiceCode(const HoaDon *list, int n, char *out, int size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char stamp[16];
    strftime(stamp, sizeof stamp, "%Y%m%d", t);

    int seq = 1;
    for (int i = 0; i < n; i++) {
        if (strncmp(list[i].code, "HD", 2) == 0 &&
            strncmp(list[i].code + 2, stamp, 8) == 0)
            seq++;
    }
    snprintf(out, size, "HD%s-%03d", stamp, seq);
}

static void printReceipt(const HoaDon *h) {
    char m1[32], m2[32], m3[32], m4[32];
    printf("+-------------------------------------------+\n");
    printf("|    SIÊU THỊ MINI - HÓA ĐƠN BÁN LẺ        |\n");
    printf("| Mã HĐ  : %-33s|\n", h->code);
    printf("| Thời gian: %-31s|\n", h->time);
    printf("+-------------------------------------------+\n");
    printf("| %-20s %4s %9s %11s |\n", "Tên hàng", "SL", "Đơn giá", "Thành tiền");
    for (int i = 0; i < h->itemCount; i++) {
        const MatHang *m = &h->items[i];
        char dg[16], tt[16];
        formatMoney(m->price, dg, sizeof dg);
        formatMoney(m->price * m->qty, tt, sizeof tt);
        printf("| %-20s %4d %9s %11s |\n", m->name, m->qty, dg, tt);
    }
    printf("+-------------------------------------------+\n");
    formatMoney(h->subtotal, m1, sizeof m1);
    formatMoney(h->discount, m2, sizeof m2);
    formatMoney(h->subtotal - h->discount, m3, sizeof m3);
    formatMoney(h->received, m4, sizeof m4);
    printf("| Tổng cộng        : %20s |\n", m1);
    printf("| Giảm giá         : %20s |\n", m2);
    printf("| Khách phải trả   : %20s |\n", m3);
    if (strcmp(h->method, "Tiền mặt") == 0) {
        char m5[32];
        formatMoney(h->change, m5, sizeof m5);
        printf("| Khách đưa        : %20s |\n", m4);
        printf("| Tiền thối        : %20s |\n", m5);
    }
    printf("| Hình thức        : %20s |\n", h->method);
    printf("+-------------------------------------------+\n");
    printf("|   Cảm ơn quý khách! Hẹn gặp lại!          |\n");
    printf("+-------------------------------------------+\n");
}

static int checkout(SanPham *products, int prodCount, HoaDon *cart,
                    HoaDon *invoices, int *invCount) {
    long subtotal = 0;
    for (int i = 0; i < cart->itemCount; i++)
        subtotal += cart->items[i].price * cart->items[i].qty;

    printf("\n================ THANH TOÁN ================\n");
    printCart(cart);

    printf("Hình thức thanh toán:\n");
    printf("  [1] Tiền mặt\n  [2] Chuyển khoản\n  [3] Thẻ\n");
    int m = getInt("Chọn: ", 1, 3);
    if (m == 1) copyStr(cart->method, "Tiền mặt", sizeof cart->method);
    else if (m == 2) copyStr(cart->method, "Chuyển khoản", sizeof cart->method);
    else copyStr(cart->method, "Thẻ", sizeof cart->method);

    long discount = getLong("Giảm giá (VND, 0 nếu không có): ", 0, subtotal);
    long total = subtotal - discount;
    long received = total, change = 0;
    if (m == 1) {
        received = getLong("Khách đưa (VND): ", total, 100000000000L);
        change = received - total;
    }

    char t1[32], t2[32], t3[32];
    formatMoney(subtotal, t1, sizeof t1);
    formatMoney(discount, t2, sizeof t2);
    formatMoney(total, t3, sizeof t3);
    printf("\nTổng cộng: %s VND | Giảm giá: %s VND | Khách trả: %s VND\n", t1, t2, t3);
    if (m == 1) {
        char t4[32];
        formatMoney(change, t4, sizeof t4);
        printf("Tiền thối: %s VND\n", t4);
    }
    if (!confirm("Xác nhận thanh toán?")) {
        printf("Đã hủy thanh toán.\n");
        return 0;
    }

    /* trừ tồn kho */
    for (int i = 0; i < cart->itemCount; i++) {
        int idx = findProductById(products, prodCount, cart->items[i].id);
        if (idx >= 0) products[idx].stock -= cart->items[i].qty;
    }
    saveProducts(products, prodCount);

    cart->subtotal = subtotal;
    cart->discount = discount;
    cart->received = received;
    cart->change = change;
    getNow(cart->time, sizeof cart->time);
    nextInvoiceCode(invoices, *invCount, cart->code, sizeof cart->code);

    if (*invCount >= MAX_INVOICES) {
        printf("(!) Danh sách hóa đơn đã đầy, không thể lưu.\n");
        return 0;
    }
    invoices[*invCount] = *cart;
    (*invCount)++;
    saveInvoices(invoices, *invCount);

    printf("\n--- HÓA ĐƠN ĐÃ LƯU ---\n");
    printReceipt(&invoices[*invCount - 1]);
    return 1;
}

static void menuBanHang(SanPham *products, int prodCount,
                        HoaDon *invoices, int *invCount) {
    HoaDon cart;
    memset(&cart, 0, sizeof cart);

    for (;;) {
        clrscr();
        printf("==================== BÁN HÀNG ====================\n");
        printCart(&cart);
        printf("\n  [1] Thêm sản phẩm vào hóa đơn\n");
        printf("  [2] Sửa số lượng mặt hàng\n");
        printf("  [3] Xóa mặt hàng khỏi hóa đơn\n");
        printf("  [4] Thanh toán\n");
        printf("  [0] Hủy hóa đơn (quay lại)\n");
        int c = getInt("Chọn: ", 0, 4);
        if (c == 0) break;
        else if (c == 1) addToCart(products, prodCount, &cart);
        else if (c == 2) editCartItem(products, prodCount, &cart);
        else if (c == 3) removeCartItem(&cart);
        else if (c == 4) {
            if (cart.itemCount == 0) {
                printf("Giỏ hàng trống, chưa thể thanh toán.\n");
                pauseEnter();
                continue;
            }
            if (checkout(products, prodCount, &cart, invoices, invCount))
                break; /* thanh toán xong thì thoát menu bán hàng */
            pauseEnter();
        }
        if (c != 4) pauseEnter();
    }
}

/* ============================================================
   Quản lý hóa đơn
   ============================================================ */

static void listInvoices(const HoaDon *list, int n, const char *dateFilter) {
    if (n == 0) { printf("Chưa có hóa đơn nào.\n"); return; }
    printf("%-18s %-17s %-14s %13s\n", "Mã HĐ", "Thời gian", "Hình thức", "Tổng tiền");
    int found = 0;
    for (int i = 0; i < n; i++) {
        const HoaDon *h = &list[i];
        if (dateFilter && strncmp(h->time, dateFilter, 10) != 0) continue;
        char total[16];
        formatMoney(h->subtotal - h->discount, total, sizeof total);
        printf("%-18s %-17s %-14s %13s\n", h->code, h->time, h->method, total);
        found++;
    }
    if (!found) printf("(Không có hóa đơn nào phù hợp)\n");
}

static int findInvoiceByCode(const HoaDon *list, int n, const char *code) {
    for (int i = 0; i < n; i++)
        if (strcmp(list[i].code, code) == 0) return i;
    return -1;
}

static void viewInvoice(const HoaDon *list, int n) {
    if (n == 0) { printf("Chưa có hóa đơn nào.\n"); return; }
    char code[MAX_CODE];
    getString("Nhập mã hóa đơn cần xem: ", code, sizeof code);
    int idx = findInvoiceByCode(list, n, code);
    if (idx < 0) { printf("Không tìm thấy hóa đơn '%s'.\n", code); return; }
    printReceipt(&list[idx]);
}

static void deleteInvoice(HoaDon *list, int *n) {
    if (*n == 0) { printf("Chưa có hóa đơn nào.\n"); return; }
    char code[MAX_CODE];
    getString("Nhập mã hóa đơn cần xóa: ", code, sizeof code);
    int idx = findInvoiceByCode(list, *n, code);
    if (idx < 0) { printf("Không tìm thấy hóa đơn '%s'.\n", code); return; }
    if (!confirm("Bạn chắc chắn muốn xóa hóa đơn này?")) return;

    for (int i = idx; i < *n - 1; i++)
        list[i] = list[i + 1];
    (*n)--;
    saveInvoices(list, *n);
    printf("Đã xóa hóa đơn %s.\n", code);
}

static void menuHoaDon(HoaDon *invoices, int *invCount) {
    for (;;) {
        clrscr();
        printf("================ QUẢN LÝ HÓA ĐƠN ================\n");
        printf("  [1] Xem danh sách hóa đơn\n");
        printf("  [2] Xem chi tiết / in hóa đơn\n");
        printf("  [3] Lọc hóa đơn theo ngày (YYYY-MM-DD)\n");
        printf("  [4] Xóa hóa đơn\n");
        printf("  [0] Quay lại\n");
        int c = getInt("Chọn: ", 0, 4);
        if (c == 0) break;
        else if (c == 1) listInvoices(invoices, *invCount, NULL);
        else if (c == 2) viewInvoice(invoices, *invCount);
        else if (c == 3) {
            char d[16];
            getString("Nhập ngày (YYYY-MM-DD): ", d, sizeof d);
            listInvoices(invoices, *invCount, d);
        }
        else if (c == 4) deleteInvoice(invoices, invCount);
        pauseEnter();
    }
}

/* ============================================================
   Thống kê
   ============================================================ */

static void menuThongKe(const HoaDon *invoices, int invCount,
                        const SanPham *products, int prodCount) {
    ThongKe tk;
    tinhThongKe(invoices, invCount, products, prodCount, time(NULL), &tk);

    char m[32];
    printf("==================== THỐNG KÊ ====================\n");
    formatMoney(tk.revenueToday, m, sizeof m);
    printf("Doanh thu hôm nay        : %s VND (%d hóa đơn)\n", m, tk.countToday);
    formatMoney(tk.revenueMonth, m, sizeof m);
    printf("Doanh thu tháng này      : %s VND (%d hóa đơn)\n", m, tk.countMonth);
    printf("Tổng số sản phẩm         : %d\n", tk.productCount);
    printf("Số mặt hàng sắp hết (≤5) : %d\n", tk.lowStock);
    formatMoney(tk.stockValue, m, sizeof m);
    printf("Giá trị tồn kho (giá nhập): %s VND\n", m);

    printf("\nTop sản phẩm bán chạy (30 ngày):\n");
    if (tk.topCount == 0) printf("  (Chưa có dữ liệu)\n");
    for (int i = 0; i < tk.topCount; i++) {
        char rev[16];
        formatMoney(tk.top[i].revenue, rev, sizeof rev);
        printf("  %2d. %-28s SL: %4d | Doanh thu: %s VND\n",
               i + 1, tk.top[i].name, tk.top[i].qty, rev);
    }

    printf("\nDanh sách hàng sắp hết:\n");
    int any = 0;
    for (int i = 0; i < prodCount; i++) {
        if (products[i].stock <= 5) {
            printf("  %-8s %-28s còn %d %s\n",
                   products[i].id, products[i].name,
                   products[i].stock, products[i].unit);
            any = 1;
        }
    }
    if (!any) printf("  (Không có mặt hàng nào sắp hết)\n");
}

/* ============================================================
   Chương trình chính
   ============================================================ */

int main(void) {
    setlocale(LC_ALL, "");

    /* Cấp phát động để tránh tràn stack (mỗi hóa đơn chứa tối đa 100 mặt hàng) */
    SanPham *products = malloc(MAX_PRODUCTS * sizeof(SanPham));
    HoaDon *invoices = malloc(MAX_INVOICES * sizeof(HoaDon));
    if (!products || !invoices) {
        fprintf(stderr, "Không đủ bộ nhớ!\n");
        free(products);
        free(invoices);
        return 1;
    }

    int prodCount = loadProducts(products, MAX_PRODUCTS);
    if (prodCount == 0) {
        seedProducts(products, &prodCount);
        printf("Lần đầu chạy: đã tạo dữ liệu sản phẩm mẫu (%d mặt hàng).\n", prodCount);
        pauseEnter();
    }
    int invCount = loadInvoices(invoices, MAX_INVOICES);

    for (;;) {
        clrscr();
        printf("====================================================\n");
        printf("  HỆ THỐNG QUẢN LÝ SIÊU THỊ MINI VÀ HÓA ĐƠN BÁN LẺ (POS)\n");
        printf("====================================================\n");
        printf("  [1] Quản lý sản phẩm\n");
        printf("  [2] Bán hàng\n");
        printf("  [3] Quản lý hóa đơn\n");
        printf("  [4] Thống kê\n");
        printf("  [0] Thoát\n");
        int c = getInt("Chọn: ", 0, 4);
        if (c == 0) break;
        if (c == 1) menuSanPham(products, &prodCount);
        else if (c == 2) menuBanHang(products, prodCount, invoices, &invCount);
        else if (c == 3) menuHoaDon(invoices, &invCount);
        else if (c == 4) menuThongKe(invoices, invCount, products, prodCount);
    }

    free(products);
    free(invoices);
    printf("Tạm biệt!\n");
    return 0;
}
