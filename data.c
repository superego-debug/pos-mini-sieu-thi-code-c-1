#include "pos.h"

/* ------------------------------------------------------------
   Xoa ky tu trang / xuong dong o cuoi chuoi
   ------------------------------------------------------------ */
void trimStr(char *s) {
    char *end = s + strlen(s);
    while (end > s &&
           (end[-1] == '\n' || end[-1] == '\r' ||
            end[-1] == ' '  || end[-1] == '\t'))
        *--end = '\0';
}

/* ------------------------------------------------------------
   Sao chep chuoi an toan (luon ket thuc bang '\0')
   ------------------------------------------------------------ */
void copyStr(char *dst, const char *src, size_t size) {
    if (!dst || !src || size == 0) return;
    strncpy(dst, src, size - 1);
    dst[size - 1] = '\0';
}

/* ------------------------------------------------------------
   Dinh dang so tien co dau phay nghin: 1234567 -> "1,234,567"
   ------------------------------------------------------------ */
void formatMoney(long v, char *out, int size) {
    char tmp[40];
    snprintf(tmp, sizeof tmp, "%ld", v);
    int len = (int)strlen(tmp);
    int o = 0;
    for (int i = 0; i < len && o < size - 1; i++) {
        out[o++] = tmp[i];
        int rem = len - i - 1;
        if (rem > 0 && rem % 3 == 0 && o < size - 1)
            out[o++] = ',';
    }
    out[o] = '\0';
}

/* ------------------------------------------------------------
   Tim chuoi con khong phan biet hoa thuong
   ------------------------------------------------------------ */
int containsIgnoreCase(const char *hay, const char *needle) {
    if (!*needle) return 1;
    size_t n = strlen(needle);
    for (const char *p = hay; *p; p++) {
        int i = 0;
        while (i < (int)n && p[i] &&
               tolower((unsigned char)p[i]) == tolower((unsigned char)needle[i]))
            i++;
        if (i == (int)n) return 1;
    }
    return 0;
}

/* ------------------------------------------------------------
   Chuyen "YYYY-MM-DD HH:MM" sang time_t (tra -1 neu loi)
   ------------------------------------------------------------ */
time_t parseTime(const char *s) {
    int y = 0, mo = 0, d = 0, h = 0, mi = 0;
    if (sscanf(s, "%d-%d-%d %d:%d", &y, &mo, &d, &h, &mi) != 5)
        return (time_t)-1;
    struct tm t;
    memset(&t, 0, sizeof t);
    t.tm_year = y - 1900;
    t.tm_mon = mo - 1;
    t.tm_mday = d;
    t.tm_hour = h;
    t.tm_min = mi;
    t.tm_isdst = -1;
    return mktime(&t);
}

/* ------------------------------------------------------------
   Lay thoi gian hien tai dang "YYYY-MM-DD HH:MM"
   ------------------------------------------------------------ */
void getNow(char *out, int size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(out, size, "%Y-%m-%d %H:%M", t);
}

/* ============================================================
   SAN PHAM
   ============================================================ */
int loadProducts(SanPham *list, int max) {
    FILE *f = fopen(FILE_PRODUCTS, "r");
    if (!f) return 0;

    int n = 0;
    char line[MAX_LINE];
    while (n < max && fgets(line, sizeof line, f)) {
        trimStr(line);
        if (!*line) continue;

        SanPham p;
        memset(&p, 0, sizeof p);

        char *tok = strtok(line, "|");
        if (!tok) continue;
        copyStr(p.id, tok, sizeof p.id);
        tok = strtok(NULL, "|"); if (tok) copyStr(p.name, tok, sizeof p.name);
        tok = strtok(NULL, "|"); if (tok) copyStr(p.category, tok, sizeof p.category);
        tok = strtok(NULL, "|"); if (tok) copyStr(p.unit, tok, sizeof p.unit);
        tok = strtok(NULL, "|"); if (tok) p.cost = atol(tok);
        tok = strtok(NULL, "|"); if (tok) p.price = atol(tok);
        tok = strtok(NULL, "|"); if (tok) p.stock = atoi(tok);

        list[n++] = p;
    }
    fclose(f);
    return n;
}

int saveProducts(const SanPham *list, int n) {
    FILE *f = fopen(FILE_PRODUCTS, "w");
    if (!f) return 0;
    for (int i = 0; i < n; i++) {
        fprintf(f, "%s|%s|%s|%s|%ld|%ld|%d\n",
                list[i].id, list[i].name, list[i].category, list[i].unit,
                list[i].cost, list[i].price, list[i].stock);
    }
    fclose(f);
    return 1;
}

void seedProducts(SanPham *list, int *n) {
    static const char *data[14][7] = {
        {"SP001", "Sữa tươi Vinamilk 1L",       "Sữa",          "Hộp",  "24000", "32000", "40"},
        {"SP002", "Mì Hảo Hảo tôm chua cay",    "Mì gói",       "Gói",  "3500",  "5000",  "120"},
        {"SP003", "Nước suối Aquafina 500ml",   "Nước uống",    "Chai", "4000",  "7000",  "80"},
        {"SP004", "Coca-Cola lon 330ml",        "Nước uống",    "Lon",  "8000",  "12000", "60"},
        {"SP005", "Bánh mì sandwich",           "Bánh",         "Gói",  "12000", "18000", "25"},
        {"SP006", "Trứng gà ta (10 quả)",       "Thực phẩm",    "Vỉ",   "28000", "38000", "15"},
        {"SP007", "Gạo ST25 túi 2kg",           "Thực phẩm",    "Túi",  "52000", "68000", "18"},
        {"SP008", "Dầu ăn Simply 1L",           "Gia vị",       "Chai", "42000", "55000", "12"},
        {"SP009", "Nước mắm Nam Ngư 500ml",     "Gia vị",       "Chai", "26000", "35000", "20"},
        {"SP010", "Kem đánh răng PS 180g",      "Hóa mỹ phẩm",  "Tuýp", "22000", "31000", "16"},
        {"SP011", "Giấy ăn Bless You",          "Hóa mỹ phẩm",  "Gói",  "9000",  "14000", "35"},
        {"SP012", "Bánh Oreo gói 133g",         "Bánh",         "Gói",  "16000", "23000", "4"},
        {"SP013", "Cà phê G7 hộp 20 gói",       "Đồ uống",      "Hộp",  "48000", "65000", "9"},
        {"SP014", "Sữa chua Vinamilk (lốc 4)",  "Sữa",          "Lốc",  "20000", "28000", "22"}
    };

    *n = 14;
    for (int i = 0; i < 14; i++) {
        copyStr(list[i].id, data[i][0], sizeof list[i].id);
        copyStr(list[i].name, data[i][1], sizeof list[i].name);
        copyStr(list[i].category, data[i][2], sizeof list[i].category);
        copyStr(list[i].unit, data[i][3], sizeof list[i].unit);
        list[i].cost = atol(data[i][4]);
        list[i].price = atol(data[i][5]);
        list[i].stock = atoi(data[i][6]);
    }
    saveProducts(list, *n);
}

/* ============================================================
   HOA DON
   Dinh dang file hoa don (moi hoa don bat dau bang dau '#'):
     #MaHD|ThoiGian|HinhThuc|TamTinh|GiamGia|KhachDua|TienThoi
     MaHD|MaSP|TenSP|GiaBan|SoLuong
   ============================================================ */
int loadInvoices(HoaDon *list, int max) {
    FILE *f = fopen(FILE_INVOICES, "r");
    if (!f) return 0;

    int n = 0;
    HoaDon *cur = NULL;
    char line[MAX_LINE];
    while (fgets(line, sizeof line, f)) {
        trimStr(line);
        if (!*line) continue;

        if (line[0] == '#') {
            if (n >= max) break;
            cur = &list[n++];
            memset(cur, 0, sizeof *cur);

            char *tok = strtok(line + 1, "|");
            if (tok) copyStr(cur->code, tok, sizeof cur->code);
            tok = strtok(NULL, "|"); if (tok) copyStr(cur->time, tok, sizeof cur->time);
            tok = strtok(NULL, "|"); if (tok) copyStr(cur->method, tok, sizeof cur->method);
            tok = strtok(NULL, "|"); if (tok) cur->subtotal = atol(tok);
            tok = strtok(NULL, "|"); if (tok) cur->discount = atol(tok);
            tok = strtok(NULL, "|"); if (tok) cur->received = atol(tok);
            tok = strtok(NULL, "|"); if (tok) cur->change = atol(tok);
        } else if (cur && cur->itemCount < MAX_ITEMS) {
            MatHang *m = &cur->items[cur->itemCount++];
            memset(m, 0, sizeof *m);

            char *tok = strtok(line, "|");              /* bo cot ma hoa don */
            tok = strtok(NULL, "|"); if (tok) copyStr(m->id, tok, sizeof m->id);
            tok = strtok(NULL, "|"); if (tok) copyStr(m->name, tok, sizeof m->name);
            tok = strtok(NULL, "|"); if (tok) m->price = atol(tok);
            tok = strtok(NULL, "|"); if (tok) m->qty = atoi(tok);
        }
    }
    fclose(f);
    return n;
}

int saveInvoices(const HoaDon *list, int n) {
    FILE *f = fopen(FILE_INVOICES, "w");
    if (!f) return 0;
    for (int i = 0; i < n; i++) {
        const HoaDon *h = &list[i];
        fprintf(f, "#%s|%s|%s|%ld|%ld|%ld|%ld\n",
                h->code, h->time, h->method,
                h->subtotal, h->discount, h->received, h->change);
        for (int j = 0; j < h->itemCount; j++) {
            const MatHang *m = &h->items[j];
            fprintf(f, "%s|%s|%s|%ld|%d\n",
                    h->code, m->id, m->name, m->price, m->qty);
        }
    }
    fclose(f);
    return 1;
}

/* ============================================================
   THONG KE
   ============================================================ */
void tinhThongKe(const HoaDon *invoices, int invCount,
                 const SanPham *products, int prodCount,
                 time_t now, ThongKe *tk) {
    memset(tk, 0, sizeof *tk);

    struct tm *t = localtime(&now);
    char todayStr[11], monthStr[8];
    strftime(todayStr, sizeof todayStr, "%Y-%m-%d", t);
    strftime(monthStr, sizeof monthStr, "%Y-%m", t);

    time_t cutoff = now - (time_t)30 * 24 * 3600;

    /* bang tam de gom san pham ban chay trong 30 ngay */
    static TopItem sold[MAX_PRODUCTS];
    int soldCount = 0;

    for (int i = 0; i < invCount; i++) {
        const HoaDon *h = &invoices[i];

        if (strncmp(h->time, todayStr, 10) == 0) {
            tk->revenueToday += h->subtotal;
            tk->countToday++;
        }
        if (strncmp(h->time, monthStr, 7) == 0) {
            tk->revenueMonth += h->subtotal;
            tk->countMonth++;
        }

        time_t it = parseTime(h->time);
        if (it != (time_t)-1 && it >= cutoff) {
            for (int j = 0; j < h->itemCount; j++) {
                const MatHang *m = &h->items[j];
                int idx = -1;
                for (int k = 0; k < soldCount; k++) {
                    if (strcmp(sold[k].name, m->name) == 0) { idx = k; break; }
                }
                if (idx < 0) {
                    if (soldCount >= MAX_PRODUCTS) continue;
                    idx = soldCount++;
                    copyStr(sold[idx].name, m->name, sizeof sold[idx].name);
                }
                sold[idx].qty += m->qty;
                sold[idx].revenue += m->qty * m->price;
            }
        }
    }

    /* sap xep giam dan theo so luong ban */
    for (int i = 0; i < soldCount - 1; i++) {
        for (int j = i + 1; j < soldCount; j++) {
            if (sold[j].qty > sold[i].qty) {
                TopItem tmp = sold[i];
                sold[i] = sold[j];
                sold[j] = tmp;
            }
        }
    }

    tk->topCount = soldCount < 8 ? soldCount : 8;
    for (int i = 0; i < tk->topCount; i++)
        tk->top[i] = sold[i];

    tk->productCount = prodCount;
    for (int i = 0; i < prodCount; i++) {
        if (products[i].stock <= 5)
            tk->lowStock++;
        tk->stockValue += products[i].cost * products[i].stock;
    }
}
