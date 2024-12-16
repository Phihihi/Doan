#define BLYNK_TEMPLATE_ID "TMPL6qaOoM0vC"
#define BLYNK_TEMPLATE_NAME "Doan"
#define BLYNK_AUTH_TOKEN "8XY0G9mFhgWhKB1NUK5w8p1K87qaBydF"
#include <WiFi.h>//Thư viện Wifi tiêu chuẩn cho ESP32
#include <WiFiClient.h>// Thư viện cho phép ESP32 hoạt động như một client để giao tiếp với các server qua Wi-Fi
#include <BlynkSimpleEsp32.h> // Kết nối ESP32 với ứng dụng Blynk và sử dụng các tính năng của Blynk
#include <DHT.h> // Thư viện hỗ trợ đọc giá trị nhiệt độ và độ ẩm từ cảm biến DHT11
#include <TimeLib.h> // Thư viện quản lý và thao tác với thời gian trong chương trình
#include <WidgetRTC.h> // Thư viện hỗ trợ đồng bộ hóa thời gian giữa thiết bị ESP32 và ứng dụng Blynk

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Happy house";  // Tên WiFi
char pass[] = "khongcho";     // Mật khẩu WiFi

BlynkTimer timer;// Tạo bộ đếm thời gian
WidgetRTC rtc;//Tạo kết nối và đồng bộ hóa thời gian thực giữa ESP32 và Blynk

#define DHT_PIN 32     // Chân kết nối DHT11 với ESP32
#define DHT_TYPE DHT22 // Loại cảm biến: DHT11
#define cam_bien_do_am_dat 35 //Chân cảm biến độ ẩm đất

#define Quat_pin  4  // Chân kết nối với nút nhấn quạt thủ công
#define Quat     17   // Chân kết nối với quạt
#define Bom_pin  16   // Chân kết nối với nút nhấn bơm thủ công
#define Bom     5  // Chân kết nối với bơm


int Trang_thai_quat = 0;
int Trang_thai_bom = 0;
float nhiet_do_thuc_te; // Nhiệt độ đo được từ DHT11
int nhiet_do_nguong =  35; // Giá trị nhiệt độ thiết lập từ Blynk( Mặc định: 35)
int do_am_khong_khi;
int do_am_nguong = 60;  // Giá trị độ ẩm thiết lập từ Blynk ( Mặc định: 60)
int gia_tri_do_am;// Giá trị phần trăm độ ẩm 
float do_am_thuc_te; // Độ ẩm đo được từ cảm biến độ ẩm đất
int dem1 = 0;
int dem2 = 0;
int dem3 = 0;

int che_do_thu_cong = 0;//Chế độ thủ công (Mặc định tắt)

DHT dht(DHT_PIN, DHT_TYPE);//Thiết lập các thông số để ESP32 giao tiếp với DHT11

void setup() {
  pinMode(Quat_pin, INPUT_PULLUP);  // Cấu hình chân nút nhấn quạt thủ công là INPUT với PULLUP
  pinMode(Quat, OUTPUT);           // Cấu hình chân quạt là OUTPUT
  digitalWrite(Quat, 0);         // Tắt quạt ban đầu
  pinMode(Bom_pin, INPUT_PULLUP);  // Cấu hình chân nút nhấn bơm thủ công là INPUT với PULLUP
  pinMode(Bom, OUTPUT);           // Cấu hình chân bơm là OUTPUT
  digitalWrite(Bom, 0);         // Tắt bơm ban đầu
  Serial.begin(9600);               
  Blynk.begin(auth, ssid, pass);

  dht.begin(); // Khởi tạo cảm biến DHT11
  rtc.begin();// Đồng bộ thời gian từ Blynk server
  Blynk.syncAll();// Đồng bộ tất cả các giá trị từ Blynk, bao gồm thời gian

  timer.setInterval(1000L, doc_nhiet_do); // Đọc giá trị nhiệt độ mỗi 1 giây
  timer.setInterval(1000L, doc_do_am_dat);// Đọc giá trị độ ẩm mỗi 1 giây
  timer.setInterval(100L, Quat_thu_cong);// Đọc giá nút bấm quạt thủ công mỗi 0,1 giây
  timer.setInterval(100L, Bom_thu_cong);// Đọc giá nút bấm bơm thủ công mỗi 0,1 giây
}

// Nhận giá trị nhiệt độ ngưỡng từ Blynk
BLYNK_WRITE(V7) {
  nhiet_do_nguong = param.asInt(); 
}
// Nhận giá trị độ ẩm ngưỡng từ Blynk
BLYNK_WRITE(V6) {
  do_am_nguong = param.asInt(); 
}

// Nút bấm quạt thủ công trên Blynk
BLYNK_WRITE(V3) {
  if(che_do_thu_cong == 1){ //  Chỉ hoạt động ở chế độ thủ công
  Trang_thai_quat = param.asInt();}
}


// Nút bấm bơm thủ công trên Blynk
BLYNK_WRITE(V8){
  if(che_do_thu_cong == 1){// Chỉ hoạt động ở chế độ thủ công
  Trang_thai_bom = param.asInt();}
}

// Chế độ thủ công
BLYNK_WRITE(V9) { 
  che_do_thu_cong = param.asInt();
}

void doc_nhiet_do() {
  nhiet_do_thuc_te = dht.readTemperature(); // Đọc nhiệt độ từ DHT11
  do_am_khong_khi = dht.readHumidity(); // Đọc độ ẩm kk từ DHT11
  Blynk.virtualWrite(V0, nhiet_do_thuc_te); // Gửi nhiệt độ tới Blynk 
  Blynk.virtualWrite(V4, do_am_khong_khi); // Gửi độ ẩm kk tới Blynk 
  Serial.print("Nhiệt độ: ");
  Serial.print(nhiet_do_thuc_te);
  Serial.print("°C, Độ ẩm không khí: ");
  Serial.print(do_am_khong_khi);
  Serial.println("%");
  }

void doc_do_am_dat() {
  gia_tri_do_am = analogRead(cam_bien_do_am_dat); // Đọc giá trị cảm biến độ ẩm đất
  do_am_thuc_te = map(gia_tri_do_am, 4095,1500, 0 , 100); // Chuyển đổi sang phần trăm độ ẩm
  Blynk.virtualWrite(V1, do_am_thuc_te); // Gửi giá trị độ ẩm lên Blynk
  Serial.print("Độ ẩm đất: ");
  Serial.print(do_am_thuc_te);
  Serial.println("%");
}


// Điều khiển quạt thủ công
void Quat_thu_cong() {
  if(che_do_thu_cong == 0){
    if (digitalRead(Quat_pin) == 0) {
    while (digitalRead(Quat_pin) == 0); // Đợi cho đến khi nút nhấn được thả ra
    dem1++;
    // Đổi trạng thái của quạt và nút bấm Blynk dựa trên số lần nhấn
  if (dem1 % 2 == 1) {
    Trang_thai_quat = 1; // Nếu số lần nhấn là lẻ, bật quạt
  } else {
    Trang_thai_quat = 0; // Nếu số lần nhấn là chẵn, tắt quạt
    dem1=0;
  }
  }
   // Cập nhật trạng thái của quạt
  digitalWrite(Quat, Trang_thai_quat);
  // Cập nhật trạng thái của quạt và nút bấm trên Blynk
  Blynk.virtualWrite(V3, Trang_thai_quat);
  Blynk.virtualWrite(V2, Trang_thai_quat);}
  }
// Điều khiển bơm thủ công
void Bom_thu_cong() {
  if(che_do_thu_cong == 1){
  if (digitalRead(Bom_pin) == 0) {
    while (digitalRead(Bom_pin) == 0); // Đợi cho đến khi nút nhấn được thả ra
    dem3++;
    // Đổi trạng thái của bơm và nút bấm Blynk dựa trên số lần nhấn
  if (dem3 % 2 == 1) {
    Trang_thai_bom = 1; // Nếu số lần nhấn là lẻ, bật bơm
  } else {
    Trang_thai_bom = 0; // Nếu số lần nhấn là chẵn, tắt bơm
    dem3=0;
  }}
  // Cập nhật trạng thái của bơm
  digitalWrite(Bom, Trang_thai_bom);
  // Cập nhật trạng thái của bơm và nút bấm trên Blynk
  Blynk.virtualWrite(V5, Trang_thai_bom);
  Blynk.virtualWrite(V8, Trang_thai_bom);
}}

void loop() {
  Blynk.run(); // Duy trì kết nối ESP32 với máy chủ Blynk
  timer.run(); // Quản lý và thực thi các bộ đếm thời gian
  Quat_thu_cong();
  Bom_thu_cong();
  doc_nhiet_do();
  doc_do_am_dat();
  if (che_do_thu_cong == 0) { // Chế độ tự động

  // Điều khiển quạt tự động
    if (nhiet_do_thuc_te > nhiet_do_nguong) {
      digitalWrite(Quat, 1); // Bật quạt
      Trang_thai_quat = 1;
    } else {
      digitalWrite(Quat, 0);  // Tắt quạt
      Trang_thai_quat = 0;
    }
    Blynk.virtualWrite(V2, Trang_thai_quat); // Cập nhật trạng thái quạt lên Blynk

  // Điều khiển quạt tự động
    if (do_am_thuc_te < do_am_nguong) {
      digitalWrite(Bom, 1); // Bật máy bơm
      Trang_thai_bom = 1;
    } else {
      digitalWrite(Bom, 0);  // Tắt máy bơm
      Trang_thai_bom = 0;
    }
    Blynk.virtualWrite(V5, Trang_thai_bom); // Cập nhật trạng thái bơm lên Blynk
}
}

