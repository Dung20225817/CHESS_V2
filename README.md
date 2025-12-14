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

---

## 📌 1. Cài Đặt Python & Môi Trường Ảo

### **Bước 1: Tạo môi trường ảo**
```bash
python -m venv myvenv
```

### **Bước 2: Kích hoạt môi trường ảo**

**Windows:**
```bash
myvenv\Scripts\activate
```

### **Bước 3: Cài đặt thư viện Python**
```bash
pip install -r requirements.txt
```

> **💡 Lưu ý:** Đảm bảo file `requirements.txt` tồn tại trong thư mục gốc của dự án.

---

## 📌 2. Cài Đặt Node.js & npm

### **Bước 1: Tải Node.js**
- Truy cập: [https://nodejs.org](https://nodejs.org)
- Tải phiên bản **LTS** (khuyến nghị)

### **Bước 2: Cài đặt**
- **Windows:** Chạy file `.msi` và làm theo hướng dẫn

### **Bước 3: Kiểm tra cài đặt**
```bash
node -v
npm -v
```

> **✅ Kết quả mong đợi:** Hiển thị phiên bản Node.js và npm

---

## 📌 3. Chạy Chương Trình

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

### **Bước 2: Chạy Bridge Python**

Mở **terminal mới** (giữ C Server đang chạy):

```bash
python Bridge.py
```

hoặc

```bash
py Bridge.py
```

> **⚠️ Lưu ý:** Đảm bảo môi trường ảo Python đã được kích hoạt.

---

### **Bước 3: Chạy Frontend**

Mở **terminal thứ ba**:

```bash
cd fe
```

**Lần chạy đầu tiên:**
```bash
npm install
npm start
```

**Các lần sau:**
```bash
npm start
```

> **🌐 Truy cập ứng dụng:** Mở trình duyệt tại `http://localhost:3000`

---

## ✅ Hoàn Tất

Hệ thống bao gồm 3 thành phần đã sẵn sàng:

1. ✔️ **C Server** - Backend xử lý logic chính
2. ✔️ **Python Bridge** - Cầu nối giữa C và Frontend
3. ✔️ **React Frontend** - Giao diện người dùng

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
  pip install --upgrade -r requirements.txt
  ```

### **Lỗi: npm install thất bại**
- **Giải pháp:**
  ```bash
  rm -rf node_modules package-lock.json
  npm cache clean --force
  npm install
  ```

