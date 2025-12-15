# 🚀 Hướng Dẫn Cài Đặt & Chạy Dự Án

## 📋 Mục Lục
- [Yêu Cầu Hệ Thống](#-yêu-cầu-hệ-thống)
- [Cài Đặt Python & Môi Trường Ảo](#-1-cài-đặt-python--môi-trường-ảo)
- [Cài Đặt Node.js & npm](#-2-cài-đặt-nodejs--npm)
- [Chạy Chương Trình](#-3-chạy-chương-trình)
- [Xử Lý Lỗi Thường Gặp](#-xử-lý-lỗi-thường-gặp)

---

## 🔧 Yêu Cầu Hệ Thống

- **Python**: 3.7 trở lên
- **Node.js**: 14.x trở lên (bao gồm npm)
- **Compiler C**: GCC hoặc MinGW (Windows)
- **Hệ điều hành**: Windows, macOS, Linux
- **Lưu ý**: Bạn nên dùng hệ điều hành linux, code được tôi ưu trên linux thay vì window
---


## 📌 1. Chạy Chương Trình

### **Bước 1: Biên dịch & chạy C Server**
```bash
cd server
make
```

**Windows:**
```bash
./c_server.exe
```

**macOS/Linux:**
```bash
./c_server
```

> **⚠️ Lưu ý:** Giữ cửa sổ terminal này mở, server cần chạy liên tục.

---

### **Bước 2: Biên dịch & chạy C Client**

Mở **terminal mới** (giữ C Server đang chạy):

```bash
cd client
make
```

---

### **Bước 3: Chạy Frontend**

Mở **terminal thứ ba**:

```bash
cd client
```

Mở **Chạy python**

```bash
python3 main.py
```

---

## ✅ Hoàn Tất

Hệ thống bao gồm 3 thành phần đã sẵn sàng:

1. ✔️ **C Server** - Backend xử lý logic chính
2. ✔️ **C Client** - Gửi và nhận thông tin tới server
3. ✔️ **GUI Python** - Giao diện người dùng

---

## 🐛 Xử Lý Lỗi Thường Gặp

### **Lỗi: `python: command not found`**
- **Giải pháp:** Thử `python3` hoặc `py` thay vì `python`

### **Lỗi: `make: command not found`**
- **Windows:** Cài đặt MinGW hoặc sử dụng WSL
- **macOS:** Cài đặt Xcode Command Line Tools: `xcode-select --install`
- **Linux:** Cài đặt build-essential: `sudo apt install build-essential`

### **Lỗi: Port đã được sử dụng**
- **Giải pháp:** 
  - Kiểm tra xem có chương trình nào đang chạy trên cùng port
  - Thay đổi port trong file cấu hình

### **Lỗi: Module không tìm thấy (Python)**
- **Giải pháp:** 
  ```bash
  sudo apt install ... (tên thư viện muốn tải)
  ```

