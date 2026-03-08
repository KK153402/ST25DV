#include <Arduino.h>
#include <Wire.h>



#define ST25DV_ADDR_SYST  0x57

uint8_t readSystemRegister(uint16_t regAddr) {
  Wire.beginTransmission(ST25DV_ADDR_SYST);
  Wire.write((uint8_t)(regAddr >> 8));
  Wire.write((uint8_t)(regAddr & 0xFF));
  Wire.endTransmission(false);
  
  Wire.requestFrom(ST25DV_ADDR_SYST, 1);
  return Wire.available() ? Wire.read() : 0xFF;
}

void writeSystemRegister(uint16_t regAddr, uint8_t value) {
  Wire.beginTransmission(ST25DV_ADDR_SYST);
  Wire.write((uint8_t)(regAddr >> 8));
  Wire.write((uint8_t)(regAddr & 0xFF));
  Wire.write(value);
  Wire.endTransmission();
}

bool presentPassword() {
  Wire.beginTransmission(ST25DV_ADDR_SYST);
  Wire.write(0x09);
  Wire.write(0x00);
  for(int i = 0; i < 8; i++) {
    Wire.write(0x00);
  }
  Wire.endTransmission();
  delay(50);
  
  uint8_t secStatus = readSystemRegister(0x0005);
  return (secStatus & 0x01) == 0x01;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== EH設定完全確認 ===");
  
  Wire.begin(21, 22);
  Wire.setClock(100000);
  delay(100);
  
  // 接続確認
  Wire.beginTransmission(ST25DV_ADDR_SYST);
  if(Wire.endTransmission() != 0) {
    Serial.println("❌ ST25DV接続失敗");
    return;
  }
  Serial.println("✅ ST25DV接続成功\n");
  
  // EH_MODE (静的レジスタ - 永続設定)
  uint8_t ehMode = readSystemRegister(0x0002);
  Serial.print("EH_MODE (0x0002): 0x");
  Serial.println(ehMode, HEX);
  if(ehMode & 0x01) {
    Serial.println("  → EH有効 ✅");
  } else {
    Serial.println("  → EH無効 ❌");
    Serial.println("  → 有効化します...");
    
    if(presentPassword()) {
      Serial.println("  → パスワード認証OK");
      writeSystemRegister(0x0002, 0x01);
      delay(100);
      
      uint8_t check = readSystemRegister(0x0002);
      if(check & 0x01) {
        Serial.println("  → ✅ EH有効化成功!");
      } else {
        Serial.println("  → ❌ EH有効化失敗");
      }
    } else {
      Serial.println("  → ❌ パスワード認証失敗");
    }
  }
  
  // EH_CTRL_DYN (動的レジスタ - 現在の状態)
  Serial.println();
  uint8_t ehCtrl = readSystemRegister(0x2002);
  Serial.print("EH_CTRL_DYN (0x2002): 0x");
  Serial.println(ehCtrl, HEX);
  Serial.println(ehCtrl & 0x01 ? "  → EH動作中 ✅" : "  → EH停止中（RFかざすと動作）");
  
  // RF_MNGT (RF管理)
  Serial.println();
  uint8_t rfMngt = readSystemRegister(0x0003);
  Serial.print("RF_MNGT (0x0003): 0x");
  Serial.println(rfMngt, HEX);
  if(rfMngt & 0x02) {
    Serial.println("  → ⚠️ RFディセーブルON（これが原因！）");
    Serial.println("  → 解除します...");
    
    if(presentPassword()) {
      writeSystemRegister(0x0003, rfMngt & ~0x02);
      delay(100);
      Serial.println("  → ✅ RF有効化完了");
    }
  } else {
    Serial.println("  → RF有効 ✅");
  }
  
  // セキュリティステータス
  Serial.println();
  uint8_t secStatus = readSystemRegister(0x0005);
  Serial.print("Security Status: 0x");
  Serial.println(secStatus, HEX);
  
  // GPO設定
  Serial.println();
  uint8_t gpoReg = readSystemRegister(0x0000);
  Serial.print("GPO_REG (0x0000): 0x");
  Serial.println(gpoReg, HEX);
  
  Serial.println("\n=== 確認完了 ===");
  Serial.println("スマホをかざしてRF Field検出をテストします...\n");
}

void loop() {
  // RF Field検出
  uint8_t rfStatus = readSystemRegister(0x2005);  // IT_STS_DYN (動的)
  
  static bool lastRF = false;
  bool currentRF = rfStatus & 0x01;
  
  if(currentRF != lastRF) {
    if(currentRF) {
      Serial.println("📱 RF Field検出! スマホがかざされました");
      
      // EH状態も確認
      uint8_t ehStatus = readSystemRegister(0x2002);
      Serial.print("  → EH動作: ");
      Serial.println(ehStatus & 0x01 ? "ON ✅" : "OFF ❌");
      
      if(!(ehStatus & 0x01)) {
        Serial.println("  → ⚠️ EHが動作していません");
        Serial.println("  → V_EHから電力が出ていない可能性");
      }
      
    } else {
      Serial.println("❌ RF Field消失");
    }
    lastRF = currentRF;
  }
  
  delay(100);
}