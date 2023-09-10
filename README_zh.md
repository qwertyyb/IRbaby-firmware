# IRbaby-Firmware

## 编译状态
[![Build Status](https://www.travis-ci.org/Caffreyfans/IRbaby-firmware.svg?branch=master)](https://www.travis-ci.org/Caffreyfans/IRbaby-firmware)

[English doc](README.md) | [中文文档](README_zh.md)

## 特性
- [x] OTA
- [x] MQTT
- [x] UDP
- [x] IRext
- [x] IRsend
- [x] IRreceive

## mqtt topic

### 设备发出 device/${mac}/send
1. device/${mac}/send/system/info

   上报设备信息, 包含MAC和IP
    ```json
    {
      "MAC":"BC:DD:C2:FA:AC:EC",
      "IP":"192.168.31.112"
    }
    ```

2. device/${mac}/send/system/running

    上报设备运行信息
    ```json
    {
      "free_mem":"21KB",
      "chip_id":"FAACEC",
      "cpu_freq":"80MHz",
      "flash_speed":"40",
      "flash_size":"1024KB",
      "reset_reason":"Power On",
      "sketch_space":"304KB",
      "fs_total_bytes":"192KB",
      "fs_used_bytes":"28KB",
      "version_name":"v1.0",
      "version_code":10,
      "ip":"192.168.31.112"
    }
    ```

3. device/${mac}/send/state/$all

    所有的业务状态 savedIRList
    ```json
    {
      "recvingIR":false,"savedIRList":"f:ac-light\ns:398\nf:ac-startswingv\ns:398\nf:ac-stopswingv\ns:398\n", // savedIRList 是所有的已保存的IR信号, f为filename名字, s为size
      "power":false,
      "temperature":25,
      "mode":0,
      "fanspeed":0,
      "swingv":false,
      "swing":false,
      "light":false
    }
    ```

4. device/${mac}/send/state/${state}

    单个状态返回，目前此设备仅支持 savedIRList

5. device/${mac}/send/event/${eventName}

    设备抛出的事件，目前支持ir-received, 已收到新的 IR 信号，需要先调用 device/${mac}/receive/control/state/recvingIR 把状态置为接收信号
    ```json
    {
      "ptl":"COOLIX",
      "raw":"4422, 4454, 512, 1684, 510, 574, 506, 1682, 512, 1680, 508, 1682, 510, 554, 528, 572, 510, 1682, 510, 576, 508, 1680, 512, 570, 512, 572, 510, 570, 512, 1684, 506, 1682, 512, 570, 512, 1684, 508, 1682, 512, 1680, 510, 1664, 528, 574, 506, 1682, 510, 574, 508, 1682, 510, 572, 510, 572, 510, 570, 512, 572, 510, 1684, 506, 572, 510, 1684, 510, 574, 506, 572, 510, 574, 510, 572, 510, 570, 514, 570, 512, 1680, 512, 576, 506, 1682, 510, 1680, 512, 1678, 514, 1680, 510, 1682, 512, 1682, 510, 570, 512, 1680, 510, 574, 510, 5274, 4422, 4456, 510, 1680, 512, 568, 512, 1682, 510, 1680, 510, 1682, 510, 570, 510, 576, 508, 1680, 512, 570, 512, 1664, 526, 556, 528, 572, 510, 572, 510, 1686, 506, 1682, 510, 574, 508, 1682, 510, 1680, 514, 1680, 510, 1680, 512, 572, 510, 1682, 510, 576, 506, 1682, 510, 574, 510, 570, 512, 572, 512, 570, 510, 1682, 510, 574, 508, 1684, 510, 572, 512, 570, 514, 568, 512, 574, 508, 572, 510, 574, 506, 1686, 508, 574, 508, 1682, 510, 1682, 510, 1664, 526, 1680, 512, 1684, 508, 1680, 510, 574, 508, 1682, 510, 570, 514",
      "len":199,
      "val":"0xB9F505"
    }
    ```

### 空调订阅 device/${mac}/receive/#
1. device/${mac}/receive/$request/system/info
2. device/${mac}/receive/$request/system/running
3. device/${mac}/receive/$request/state/$all
4. device/${mac}/receive/$request/state/${state}

5. device/${mac}/receive/control/state/$all
6. device/${mac}/receive/control/state/${state}

7. device/${mac}/receive/command/${commandName}

    目前支持的 commandName 有 sendIR 和 saveIR

    7.1 saveIR 支持两种格式
    -  传入 payload 为 {"name": "xxx"}, 把收到的信号存为 xxx, 具体地址为 /bin/xxx
    - 传入 payload 为 {"name": "xxx", "raw": "xxx", "len": 111 }, 把 raw 存储到 /bin/xxx ，此能力尚未测试

    7.2 sendIR 也支持两种格式, 和上面的 saveIR 对应

    7.3 其余待需要支持的为 removeIR  