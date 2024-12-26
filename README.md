# HỆ THỐNG BÃI ĐỖ XE THÔNG MINH VÀ AN TOÀN

## Đặt bài toán
__Nhu cầu thực tế__

Trong các khu đô thị hiện đại, sự gia tăng nhanh chóng của phương tiện cá nhân khiến các bãi đỗ xe truyền thống trở nên quá tải và thiếu hiệu quả. Đồng thời, việc quản lý thủ công không chỉ tốn kém nhân lực mà còn dễ dẫn đến sai sót, mất an ninh và khó kiểm soát khi có sự cố xảy ra như cháy. Những vấn đề này đặt ra yêu cầu về một hệ thống bãi đỗ xe thông minh với đa dạng chức năng như:
- Quản lý tối ưu chỗ trống và số lượng xe: Giúp chủ bãi xe và người dùng dễ dàng xác định vị trí đỗ trống và sử dụng không gian hiệu quả.
- Tăng cường an toàn: Phát hiện và cảnh báo kịp thời các nguy cơ như cháy, khói, hoặc xe đỗ không đúng quy định.
- Tự động hóa và tiết kiệm chi phí vận hành: Giảm thiểu sự can thiệp của con người thông qua IoT.

__Cảm hứng cho đề tài__

Thực trạng cháy nổ tại các bãi xe: Các vụ cháy lớn tại các bãi đỗ xe trong hầm làm nổi bật nhu cầu về hệ thống phát hiện và xử lý cháy tự động.

Kinh nghiệm từ các thành phố lớn: Nhiều thành phố hiện đại đã triển khai bãi đỗ xe thông minh, nhưng chi phí đầu tư cao khiến các hệ thống này khó áp dụng rộng rãi tại Việt Nam. Việc xây dựng một mô hình đơn giản, chi phí thấp sẽ giúp tiếp cận được với các bãi xe nhỏ và vừa.

# Nội dung chính

1. Phần cứng
- Vi điều khiển: MH-ET LIVE MiniKit for ESP32
- Cửa vào/ra: 
    + 2 x Servo SG90: Điều khiển đóng/mở ở 2 cửa
    + 2 x Cảm biến phát hiện vật cản: Phát hiện xe vào/ra
- Bãi đỗ xe:
    + 4 x Cảm biến siêu âm HC-SR04: Phát hiện trạng thái xe đỗ ở 4 vị trí
    + 2 x LDR: Điều khiển bật/tắt đèn dựa vào ánh sáng môi trường
- Hệ thống an toàn: 
    + 1 x DHT11: Theo dõi nhiệt độ, độ ẩm môi trường
    + 1 x MQ-2: Phát hiện khí gas, khói
    + 1 x Buzzer: Thông báo khi phát hiện nguy hiểm

2. Phần mềm
- Node-RED:
    + Hiển thị: Số lượng xe, trạng thái vị trí đỗ.
    + Quản lý: Thời gian đỗ xe, bật/tắt đèn, đóng/mở cổng.
    + Kiểm tra an toàn: Theo dõi nhiệt độ, độ ẩm, khí ga.
- MQTT Broker (EMQX)
- Platform.IO

3. Ý tưởng
- Cửa ra/vào: 
    + Cảm biến phát hiện vạt thể được sử dụng để phát hiện xe ra/vào, từ đó vi điều khiển sẽ kích hoạt servo sg90 mở/đóng
	+ Trên dashboard có thể kích hoạt mở/đóng servo sg90
- Bãi đỗ xe: 
    + 4 bãi đỗ xe <-> 4 cảm biến siêu âm HC-SR04 để kiểm tra vị trí trống/có xe
	+ Hiển thị trạng thái của từng vị trí trên dashboard: trống/có xe/ 
	+ Hiển thị thời gian đỗ xe
	+ LDR bật/tắt đèn khi trời tối/sáng; dashboard cũng có thể bật/tắt đèn
	+ Đo nhiệt độ, độ ẩm, khí ga của bãi đỗ xe + buzzer cảnh báo khi gặp nguy hiểm
	+ Hiển thị nhiệt độ, độ ẩm, khí ga lên dashboard

# Sơ đồ khối
1. Sơ đồ khối
![Hình 1](./images/3.png)

2. Lưu đồ thuật toán:
- Phần cửa ra/vào

![Hình 2](./images/1.png)

-Phần bãi đỗ xe

![Hình 3](./images/2.png)

# Sơ đồ chân
Sơ đồ chân được mô phỏng trên phần mềm Fritzing
![Hình 9](./images/9.png)

Bảng kết nối với các chân của esp32

| Thiết bị   | Chân          | ESP32         |
|------------|---------------|---------------|
| **Servo**  | Vcc           | 5V            |
|            | GND           | GND           |
|            | PWM entry     | 4             |
|            | PWM exit      | 2             |
| **HC-SR04**| Vcc           | 5V            |
|            | GND           | GND           |
| Entry      | trig          | 17            |
|            | echo          | 16            |
| Exit       | trig          | 33            |
|            | echo          | 14            |
| Pos 1      | trig          | 26            |
|            | echo          | 13            |
| Pos 2      | trig          | 19            |
|            | echo          | 18            |
| Pos 3      | trig          | 23            |
|            | echo          | 5             |
| Pos 4      | trig          | 32            |
|            | echo          | 12            |
| **DHT**    | Vcc           | 5V            |
|            | GND           | GND           |
|            | Data          | 10            |
|   **MQ2**  | Vcc           | 5V            |
|            | GND           | GND           |
|            | Analog Out    | 35            |
|**LDR**     | Vcc           | 3.3V          |
|            | GND           | GND           |
|            | Analog Out    | 39            |
| **LED**    | LED 1         | 25            |
|            | LED 2         | 15            |
| **Buzzer** | Signal        | 9             |

## Kết quả 
1. Mô hình:
![Hình 8](./images/8.png)

2. Node-red dashboard 2.0
- Node-red dashboard gồm 5 group: Home, Door Status, Position, Temperature/Humadity/Gas, Brightness
![Hình h1](./images/h1.png)
- Trên giao diện **Home** được hiển thị số lượng xe, thống kê số lượng xe theo thời gian, giờ hiện tại(theo giờ Việt Nam), tác giả, kiểm tra kết nối MQTT
![Hình h2](./images/h2.png)
- Giao diện **Door Status** được dùng để hiển thị và điều khiển trạng thái cửa vào/ra của bãi đỗ xe
![Hình h3](./images/h3.png)
- Giao diện **Position** gồm các chức năng:
    + Kiểm tra trạng thái từng vị trí: Trống (Empty), Đã có xe (Occupied), Đã được đặt trước (Reserved)
    + Đặt trước chỗ trên các vị trí trống
    + Nếu vị trí đã có xe: tính thời gian xe đặt tại vị trí đó
![Hình 4](./images/4.png)
- Giao diện **Temperature/Humidity/Gas** để hiển thị nhiệt độ, độ ẩm và giá trị đo được từ cảm biến khí gas, đồng thời thống kê các giá trị đó theo thời gian
![Hình 5](./images/5.png)
- Giao diện **Brightness** được dùng để hiển thị trạng thái đèn, trạng thái sáng tối của bãi đỗ xe và điều khiển đèn bật/tắt
![Hình 6](./images/6.png)

## Kết luận
Đã xây dựng thành công một hệ thống bãi đỗ xe thông minh với các chức năng:
- Quản lý vị trí đỗ xe, số lượng xe và thời gian đỗ.
- Phát hiện cháy nổ hoặc sự cố bất thường (khói, nhiệt độ cao).
- Tự động hóa quá trình ra/vào bằng cảm biến và cửa servo.
- Tích hợp dashboard qua Node-RED để hiển thị và điều khiển.


## Video demo: [Video demo](https://youtu.be/6vYeLfkQ3tU)
