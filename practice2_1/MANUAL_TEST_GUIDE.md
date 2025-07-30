# 🔧 **HƯỚNG DẪN MANUAL TEST MODE**

## 📋 **Tổng Quan Cải Tiến**

Dự án đã được nâng cấp từ chế độ **Auto Test** sang **Manual Test** để cho phép kiểm chứng chi tiết từng Practice một cách linh hoạt.

### ✅ **Các Cải Tiến Chính:**

1. **Manual Test Interface** - Điều khiển qua UART commands
2. **Flexible Mode Switching** - Chuyển đổi Manual/Auto mode
3. **Detailed Feedback** - Thông tin chi tiết cho từng test case
4. **Individual Practice Testing** - Test riêng lẻ từng practice
5. **Security Status Monitoring** - Theo dõi trạng thái security unlock

---

## 🚀 **Cách Sử Dụng Manual Test**

### **1. Khởi Động Hệ Thống**
```
STM32 CAN Diagnostic Demo Started
=================================

=========== MANUAL TEST MENU ===========
Mode: MANUAL
----------------------------------------
Commands:
  1 - Test Practice 1 (ReadDataByIdentifier)
  2 - Test Practice 2a (SecurityAccess SEED)
  3 - Test Practice 2b (SecurityAccess KEY)
  4 - Test Practice 3 (WriteDataByIdentifier)
  A - Run All Practices (Auto sequence)
  M - Toggle Manual/Auto Mode
  H - Show this Help menu
  S - Show Security Status
========================================
Enter command:
```

### **2. Các Lệnh Điều Khiển**

| Lệnh | Chức Năng | Mô Tả |
|------|-----------|-------|
| `1` | Practice 1 | Test ReadDataByIdentifier (SID 0x22) |
| `2` | Practice 2a | Test SecurityAccess SEED Request |
| `3` | Practice 2b | Test SecurityAccess KEY Send |
| `4` | Practice 3 | Test WriteDataByIdentifier (SID 0x2E) |
| `A` | All Tests | Chạy tất cả practices liên tục |
| `M` | Mode Toggle | Chuyển Manual ↔ Auto mode |
| `H` | Help | Hiển thị menu trợ giúp |
| `S` | Status | Kiểm tra trạng thái Security |

---

## 🧪 **Quy Trình Test Chi Tiết**

### **Step 1: Test Practice 1 (ReadDataByIdentifier)**
```bash
Enter command: 1

>>> MANUAL TEST: Practice 1 <<<
Testing ReadDataByIdentifier (SID 0x22, DID 0x0123)
Expected: Returns current CAN ID from tester
--------------------------------------------
Sending RDBI request
31234 712: 03 22 01 23 55 55 55 55
✅ Practice 1 complete.
Enter command:
```

**🔍 Kết Quả Mong Đợi:**
- Response: `7A2: 05 62 01 23 07 12 55 55`
- Giải thích: SID 0x62 + DID 0x0123 + CAN ID 0x0712

### **Step 2: Test Practice 2a (SecurityAccess SEED)**
```bash
Enter command: 2

>>> MANUAL TEST: Practice 2a (SEED) <<<
Testing SecurityAccess SEED Request (SID 0x27, Level 0x01)
Expected: Returns 6-byte random SEED
------------------------------------------------------
Sending SECA SEED request
31456 712: 02 27 01 55 55 55 55 55
ℹ️ Note: Use received SEED to calculate KEY for Practice 2b
✅ Practice 2a complete.
Enter command:
```

**🔍 Kết Quả Mong Đợi:**
- Response: `7A2: 08 67 01 A3 5C 91 2F 84`
- Giải thích: SID 0x67 + Level 0x01 + 6 bytes SEED

### **Step 3: Test Practice 2b (SecurityAccess KEY)**
```bash
Enter command: 3

>>> MANUAL TEST: Practice 2b (KEY) <<<
Testing SecurityAccess KEY Send (SID 0x27, Level 0x02)
Expected: LED turns ON for 5 seconds if KEY is correct
--------------------------------------------------------
SEED-KEY Algorithm:
KEY[0] = SEED[0] XOR SEED[1]
KEY[1] = SEED[1] + SEED[2]
KEY[2] = SEED[2] XOR SEED[3]
KEY[3] = SEED[3] + SEED[0]
KEY[4] = SEED[4] AND 0xF0
KEY[5] = SEED[5] AND 0x0F
------------------------
Sending SECA KEY
31678 712: 08 27 02 FF ED BE D2 80
🔍 Check LED status: Should be ON if KEY is correct
✅ Practice 2b complete.
Enter command:
```

**🔍 Kết Quả Mong Đợi:**
- Response: `7A2: 02 67 02 55 55 55 55 55`
- **LED PB0 bật sáng 5 giây** ← Quan trọng!

### **Step 4: Test Practice 3 (WriteDataByIdentifier)**
```bash
Enter command: 4

>>> MANUAL TEST: Practice 3 <<<
Testing WriteDataByIdentifier (SID 0x2E, DID 0x0123)
Expected: Writes new CAN ID (0x0456) - requires security unlock
------------------------------------------------------------------
✅ Security unlocked. Should write successfully.
Sending WDBI request
31890 712: 05 2E 01 23 04 56 55 55
ℹ️ Note: New CAN ID applied after User Button press (Ignition Cycle)
✅ Practice 3 complete.
Enter command:
```

**🔍 Kết Quả Mong Đợi:**
- Response: `7A2: 03 6E 01 23 55 55 55 55`
- Nếu chưa unlock security: `7A2: 03 7F 2E 33 55 55 55 55`

---

## 🔄 **Chế Độ Auto Mode**

### **Kích Hoạt Auto Mode:**
```bash
Enter command: M

>>> MODE CHANGED <<<
Switched to: AUTO MODE
- Auto testing every 15 seconds
- Manual commands still available
```

**Trong Auto Mode:**
- Hệ thống tự động chạy tất cả practices mỗi 15 giây
- Vẫn có thể sử dụng manual commands
- Heartbeat message mỗi 10 giây

---

## 🔍 **Kiểm Tra Trạng Thái**

### **Security Status:**
```bash
Enter command: S

>>> SECURITY STATUS <<<
Security Level 1: UNLOCKED (LED ON)
Enter command:
```

### **User Button (Ignition Cycle):**
- Nhấn User Button để áp dụng CAN ID mới từ Practice 3
- Màn hình sẽ hiển thị: `IG OFF -> IG ON` và `New CAN ID Applied`

---

## 🛠️ **Troubleshooting**

### **Vấn đề thường gặp:**

1. **LED không bật sau Practice 2:**
   - Kiểm tra thuật toán SEED-KEY
   - Verify KEY calculation đúng

2. **Practice 3 trả về NRC 0x33:**
   - Chạy Practice 2 trước để unlock security
   - Đảm bảo LED đang sáng

3. **Không thấy response:**
   - Kiểm tra CAN connection
   - Verify CAN ID 0x712 (request) và 0x7A2 (response)

### **Debug Commands:**
- `H` - Hiển thị help menu
- `S` - Kiểm tra security status
- `A` - Chạy full sequence để verify tất cả

---

## 📊 **Expected Test Results**

| Practice | Request | Expected Response | Status Indicator |
|----------|---------|-------------------|------------------|
| 1 | `03 22 01 23` | `05 62 01 23 07 12` | ✅ Data returned |
| 2a | `02 27 01` | `08 67 01 [SEED]` | ✅ SEED received |
| 2b | `08 27 02 [KEY]` | `02 67 02` | 🔴 LED ON 5 sec |
| 3 | `05 2E 01 23 04 56` | `03 6E 01 23` | ✅ Data written |

---

## 💡 **Tips & Best Practices**

1. **Luôn chạy Practice 2 trước Practice 3**
2. **Theo dõi LED để confirm security unlock**
3. **Sử dụng User Button để test ignition cycle**
4. **Check timestamps để verify timing requirements**
5. **Dùng manual mode để debug chi tiết từng practice**

---

**📝 Ghi chú:** Manual test mode giúp kiểm chứng chi tiết logic của từng practice và phát hiện lỗi một cách dễ dàng hơn so với auto mode.
