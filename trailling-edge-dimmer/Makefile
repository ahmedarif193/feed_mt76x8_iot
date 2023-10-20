#
# This software is licensed under the Public Domain.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=trailling-edge-dimmer
PKG_VERSION:=1.0.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_MAINTAINER:=Ahmed ARIF <arif193@gmail.com>
PKG_LICENSE:=GPL3
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/trailling-edge-dimmer
	SECTION:=net
	# Select package by default
	DEFAULT:=y
	CATEGORY:=Network
	TITLE:=Some other dummy application.
	URL:=https://www.example.com
	SUBMENU:=Utils
	DEPENDS:= +libstdcpp +mosquitto-ssl +jsoncpp +libgpiod +libmosquitto-ssl +libmosquittopp +mosquitto +@KERNEL_DEVMEM
endef

define Package/trailling-edge-dimmer/description
	Some example Programm called trailling-edge-dimmer
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/trailling-edge-dimmer/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tedd $(1)/usr/bin/
endef


$(eval $(call BuildPackage,trailling-edge-dimmer))
