# dde-session-shell

- [Development guide](#development-guide)

## Development guide

依赖 dde-session-shell-dev 开发包。

模块化接口头文件路径: `/usr/include/dde-session-shell`，接口说明参考各接口类中的注释

- base_module_interface.h 定义了接口类必须实现且共用的方法，是所有接口类的基类。
- login_module_interface.h 定义了登录模块必须实现的方法，继承接口基类。
- tray_module_interface.h 文件定义了右下角区域模块必须实现的方法，继承接口基类。
