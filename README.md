# DDE Session Shell

DDE session shell provides two applications: dde-lock and lightdm-deepin-greeter. dde-lock provides screen lock function, and lightdm-deepin-greeter provides login function.

## Build

debian/rules

```makefile
override_dh_auto_configure:
	dh_auto_configure -- \
		-DWAIT_DEEPIN_ACCOUNTS_SERVICE=1
```
lightdm-deepin-greeter strongly relies on the com.deepin.daemon.Accounts service(created by dde-system-daemon). We use WAIT_DEEPIN_ACCOUNTS_SERVICE to control whether to wait for the service to start when the program starts. If dde-system-daemon will not be started in the desktop environment, please set this value to 0, otherwise the deepin-lightdm-greeter will get stuck.


## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

[**Contribution guide for developers**](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)

[**开发者代码贡献指南**](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)
