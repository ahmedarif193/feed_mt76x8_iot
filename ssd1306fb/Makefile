#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ssd1306fb
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ssd1306fb
  SUBMENU:=OLED Display Support
  TITLE:=SSD1306/07 framebuffer driver
  FILES:=$(PKG_BUILD_DIR)/ssd1306fb.ko
  AUTOLOAD:=$(call AutoLoad,30,ssd1306fb,1)
  KCONFIG:=
  DEPENDS:=+kmod-fb-cfb-imgblt +kmod-fb-cfb-fillrect +kmod-i2c-core
endef

define KernelPackage/ssd1306fb/description
	Drives ssd1306/07 oled displays.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,ssd1306fb))
