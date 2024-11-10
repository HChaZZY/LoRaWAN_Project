
# LoRaWAN 智慧农业监控系统

> 基于STM32L476VG微控制器的LoRaWAN农业环境监测与预警系统
>
> 本项目是杭州科技大学`生产实习`的课程最终作业

## 项目简介

本项目是一个基于STM32L476VG微控制器的农业物联网环境监测系统，集成多传感器数据采集、LoRaWAN通信和降雨预警功能。系统能实时监测温度、湿度、气压和光照等环境参数，并通过LoRaWAN网络传输数据。

## 项目特点

- 多传感器数据采集
- LoRaWAN网络通信
- 降雨预警算法
- 低功耗设计
- 实时数据显示

## 文件结构

```shell
.
├── app                # 应用层代码
│   ├── app.c          # 应用层代码
│   └── main.c         # 主循环代码
├── Drivers            # 硬件驱动
├── Inc                # 头文件
├── MDK-ARM            # Keil工程文件
├── src                # 源代码
│   ├── board          # 板级支持包
│   ├── common         # 通用组件
│   └── mcu            # 微控制器相关代码
└── EWARM              # IAR工程文件
```

## 硬件配置

- **主控制器**：STM32L476VG
- **通信模块**：LoRaWAN
- **传感器**：
  - HDC1000 温湿度传感器
  - OPT3001 光强传感器
  - MPL3115 气压传感器
  - MMA8451 加速度传感器

## 软件依赖

- Keil MDK
- STM32CubeMX
- ST HAL库

## 编译与烧录

1. 使用Keil MDK打开 `MDK-ARM/Project.uvprojx`
2. 配置工程参数
3. 编译项目
4. 通过J-Link烧录固件

## 使用说明

1. 检查硬件连接
2. 确认LoRaWAN网络配置
3. 上电运行
4. 通过LCD查看实时数据
5. 接收LoRaWAN云端推送

## 开发环境

- **开发平台**：Windows 11
- **编译工具**：Keil MDK v5.36
- **调试器**：J-Link V9
- **固件版本**：V1.0.0

## 未来计划

- 优化降雨预警算法
- 提供更多边缘计算功能
- 支持更多传感器接入

## 许可证

本项目基于 AGPL v3 开源许可证。详情请参阅 `LICENSE` 文件。

## 贡献

欢迎提交 Issues 和 Pull Requests。

## 联系方式

如有任何问题，请联系：[电子邮件](mailto:hcha@hcha.top)

## 致谢

感谢开源社区和ST微电子提供的帮助。

感谢利尔达科技集团股份有限公司提供的开发板和相关技术支持。

我们深深感谢来自社区和组织的贡献，是这些贡献使得这样的项目成为可能。
