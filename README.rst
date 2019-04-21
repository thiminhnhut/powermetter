Đo điện năng tiêu thụ của thiết bị điện AC
##########################################

* **Thực hiện:**

* **Thời gian: Ngày 21 tháng 04 năm 2019**

.. sectnum::

.. contents:: Nội dung

Đo điện năng tiêu thụ của thiết bị điện AC
******************************************

Giới thiệu
==========

* Điều khiển và theo dõi điện năng tiêu thụ của thiết bị điện AC qua Internet với App Blynk:

  * Vi điều khiển: ESP8266 Node MCU 1.0.

  * App điều khiển: Blynk (Android, iOS).

  * Cảm biến điện năng: PZEM004T

  * Sử dụng Relay để điều khiển đóng, ngắt tải AC.

  * Cảnh báo quá tải.

Mạch nguyên lý
==============

.. image:: 10.Schematic/Schematic_schem.png

Chương trình điều khiển
=======================

* File config PlatformIO: `platformio.ini <https://github.com/thiminhnhut/powermetter/blob/master/20.Firmware/platformio.ini>`_

* File `config.h <https://github.com/thiminhnhut/powermetter/blob/master/20.Firmware/src/config.h>`_

* File chương trình chính: `main.cpp <https://github.com/thiminhnhut/powermetter/tree/master/20.Firmware/src>`_

Giải thích hoạt động của code
=============================

Kết quả
=======
