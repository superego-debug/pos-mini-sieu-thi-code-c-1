#ifndef POS_H
#define POS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <locale.h>

#define MAX_PRODUCTS 1000
#define MAX_INVOICES 2000
#define MAX_ITEMS 100
#define MAX_NAME 80
#define MAX_CAT 40
#define MAX_UNIT 20
#define MAX_CODE 32
#define MAX_LINE 512

#define FILE_PRODUCTS "sanpham.txt"
#define FILE_INVOICES "hoadon.txt"

typedef struct {
    char id[12];          /* ma san pham, vi du "SP001" */
    char name[MAX_NAME];
    char category[MAX_CAT];
    char unit[MAX_UNIT];
    long cost;            /* gia nhap */
    long price;           /* gia ban */
    int stock;            /* ton kho */
} SanPham;

typedef struct {
    char id[12];
    char name[MAX_NAME];
    long price;
    int qty;
} MatHang;

typedef struct {
    char code[MAX_CODE];  /* ma hoa don, vi du "HD20251102-001" */
    char time[32];        /* "YYYY-MM-DD HH:MM" */
    char method[16];      /* Tien mat / Chuyen khoan / The */
    MatHang items[MAX_ITEMS];
    int itemCount;
    long subtotal;
    long discount;
    long received;
    long change;
} HoaDon;

typedef struct {
    char name[MAX_NAME];
    int qty;
    long revenue;
} TopItem;

typedef struct {
    long revenueToday;
    long revenueMonth;
    int countToday;
    int countMonth;
    int lowStock;
    long stockValue;
    int productCount;
    TopItem top[8];
    int topCount;
} ThongKe;

/* ---------- tien ich chung (data.c) ---------- */
void copyStr(char *dst, const char *src, size_t size);
void trimStr(char *s);
void formatMoney(long v, char *out, int size);
int containsIgnoreCase(const char *haystack, const char *needle);
time_t parseTime(const char *s);
void getNow(char *out, int size);

/* ---------- san pham (data.c) ---------- */
int loadProducts(SanPham *list, int max);
int saveProducts(const SanPham *list, int n);
void seedProducts(SanPham *list, int *n);

/* ---------- hoa don (data.c) ---------- */
int loadInvoices(HoaDon *list, int max);
int saveInvoices(const HoaDon *list, int n);

/* ---------- thong ke (data.c) ---------- */
void tinhThongKe(const HoaDon *invoices, int invCount,
                 const SanPham *products, int prodCount,
                 time_t now, ThongKe *tk);

#endif
