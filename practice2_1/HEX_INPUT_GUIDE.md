# 🔧 **HEX INPUT MODE - HERCULES STYLE TESTING**

## 📋 **Tổng Quan**

Bây giờ bạn có thể nhập trực tiếp các giá trị hex giống như trong Hercules SETUP utility! Hệ thống hỗ trợ nhập raw CAN messages và thực thi chúng real-time.

---

## 🚀 **Cách Sử Dụng Hex Input Mode**

### **1. Vào Hex Input Mode**
```
Enter command: X

>>> ENTERING HEX INPUT MODE <<<
========== HEX INPUT MODE ==========
Enter CAN message in hex format:
Format: XX XX XX XX XX XX XX XX
Examples:
  03 22 01 23 55 55 55 55  (Practice 1)
  02 27 01 55 55 55 55 55  (Practice 2a)
  08 27 02 XX XX XX XX XX  (Practice 2b)
  05 2E 01 23 04 56 55 55  (Practice 3)
Commands:
  ENTER - Execute message
  ESC   - Exit hex mode
  SPACE - Separator (optional)
===================================
HEX>
```

### **2. Nhập Hex Values**

Bạn có thể nhập theo nhiều format:

**Format 1: Với spaces (recommended)**
```
HEX> 03 22 01 23 55 55 55 55
```

**Format 2: Liền nhau**
```
HEX> 0322012355555555
```

**Format 3: Mixed**
```
HEX> 03220123 55 55 55 55
```

### **3. Thực Thi Message**
Nhấn **ENTER** để thực thi:
```
HEX> 03 22 01 23 55 55 55 55
Executing: 03 22 01 23 55 55 55 55 
Processing diagnostic request...
31234 712: 03 22 01 23 55 55 55 55
[Response sẽ hiển thị ở đây]

HEX>
```

---

## 🧪 **Test Cases Từ Hercules**

### **Test 1: Practice 1 - ReadDataByIdentifier**
```
HEX> 03 22 01 23 55 55 55 55
Expected Response: 05 62 01 23 07 12 55 55
```

### **Test 2: Practice 2a - SecurityAccess SEED Request**  
```
HEX> 02 27 01 55 55 55 55 55
Expected Response: 08 67 01 A3 5C 91 2F 84
```

### **Test 3: Practice 2b - SecurityAccess KEY Send**
```
HEX> 08 27 02 FF ED BE D2 80
Expected Response: 02 67 02 55 55 55 55 55
Expected: LED bật sáng 5 giây
```

### **Test 4: Practice 3 - WriteDataByIdentifier**
```
HEX> 05 2E 01 23 04 56 55 55
Expected Response: 03 6E 01 23 55 55 55 55 (if security unlocked)
Expected Response: 03 7F 2E 33 55 55 55 55 (if security locked)
```

---

## 🔍 **Error Test Cases**

### **Test Invalid DID (NRC 0x31)**
```
HEX> 03 22 01 24 55 55 55 55
Expected Response: 03 7F 22 31 55 55 55 55
```

### **Test Invalid Length (NRC 0x13)**
```
HEX> 02 22 01 55 55 55 55 55
Expected Response: 03 7F 22 13 55 55 55 55
```

### **Test Security Access Denied (NRC 0x33)**
```
HEX> 05 2E 01 23 04 56 55 55  (without security unlock)
Expected Response: 03 7F 2E 33 55 55 55 55
```

### **Test Invalid KEY (NRC 0x35)**
```
HEX> 08 27 02 00 00 00 00 00  (wrong key)
Expected Response: 03 7F 27 35 55 55 55 55
```

---

## ⌨️ **Keyboard Controls**

| Key | Function | Description |
|-----|----------|-------------|
| **0-9, A-F** | Hex Input | Enter hex digits |
| **SPACE** | Separator | Optional spacing (ignored) |
| **ENTER** | Execute | Process the hex message |
| **ESC** | Exit | Return to command mode |
| **BACKSPACE** | Delete | Remove last character |

---

## 📊 **Message Format Analysis**

### **CAN-TP Single Frame Format**
```
Byte 0: [0N] - PCI + SF_DL (Single Frame Data Length)
Byte 1-7: Data bytes
```

**Examples:**
- `03 22 01 23`: PCI=0, SF_DL=3, Data=22 01 23
- `08 27 02 FF ED BE D2 80`: PCI=0, SF_DL=8, Data=27 02 FF ED BE D2 80 (nhưng chỉ 7 bytes data vì byte 0 là PCI)

### **UDS Message Structure**
```
SID (Service ID): 22, 27, 2E
Data Identifier (DID): 01 23 (for 0x0123)
Additional Data: Tùy theo service
```

---

## 🛠️ **Debug Features**

### **Real-time Feedback**
- Hệ thống hiển thị message sẽ được thực thi
- CAN log format: `timestamp CANID: D0 D1 D2 D3 D4 D5 D6 D7`
- Response được hiển thị ngay lập tức

### **Auto-padding**
- Messages ngắn hơn 8 bytes tự động được pad với 0x55
- Byte count được tự động detect từ input

### **Error Handling**
- Invalid hex characters được reject
- Buffer overflow protection
- Clear error messages

---

## 💡 **Pro Tips**

### **1. Rapid Testing**
```bash
# Sequence test nhanh:
X                                    # Enter hex mode
03 22 01 23 55 55 55 55             # Practice 1
02 27 01 55 55 55 55 55             # Practice 2a  
08 27 02 FF ED BE D2 80             # Practice 2b
05 2E 01 23 04 56 55 55             # Practice 3
ESC                                  # Exit hex mode
```

### **2. SEED-KEY Calculation**
Khi nhận được SEED từ Practice 2a, tính KEY:
```
SEED: A3 5C 91 2F 84 67
KEY[0] = A3 XOR 5C = FF
KEY[1] = 5C + 91 = ED  
KEY[2] = 91 XOR 2F = BE
KEY[3] = 2F + A3 = D2
KEY[4] = 84 AND F0 = 80
KEY[5] = 67 AND 0F = 07
→ KEY: FF ED BE D2 80 07
```

### **3. Copy-Paste từ Hercules**
Bạn có thể copy hex values từ Hercules và paste vào:
- Remove spaces hoặc giữ nguyên đều được
- Hệ thống tự động parse

---

## 🔄 **Workflow Integration**

### **Hercules → STM32 Test Flow**
1. **Prepare message in Hercules** (như trong hình bạn gửi)
2. **Copy hex values**
3. **Enter STM32 hex mode** (`X` command)
4. **Paste/type hex values**
5. **Execute with ENTER**
6. **Verify response**
7. **Continue with next test**

### **Example từ Hình của Bạn**
```
From Hercules: 27 02 41 1A A2 39
To STM32 HEX mode:
HEX> 27 02 41 1A A2 39 55 55
```

---

## 🎯 **Advantages Over Fixed Commands**

✅ **Flexibility**: Test bất kỳ hex combination nào  
✅ **Real-world**: Giống như tools thật (Hercules, CANoe)  
✅ **Debug**: Test edge cases và invalid inputs  
✅ **Learning**: Hiểu rõ message structure  
✅ **Validation**: Verify exact hex sequences  

**Bây giờ bạn có thể test giống hệt như Hercules SETUP utility!** 🚀
