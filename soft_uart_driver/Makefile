include $(TOPDIR)/rules.mk

PKG_NAME:=soft-uart-driver
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

# Kernel module definition
define KernelPackage/soft-uart-driver
  SUBMENU:=Other modules
  TITLE:=Sooall PWM Driver
  FILES:=$(PKG_BUILD_DIR)/soft_uart.ko
  AUTOLOAD:=$(call AutoLoad,30,soft-uart-drv)
endef

define KernelPackage/soft-uart-driver/description
  Kernel module for controlling software serial for embedded devices
endef

# Userspace application definition
define Package/soft-uart-driver-app
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Sooall PWM Userspace Utility
endef

define Package/soft-uart-driver-app/description
  Userspace utility for controlling trailling edge dimmer
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

define Package/soft-uart-driver-app/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sooall_pwm_app $(1)/usr/bin/
endef

$(eval $(call KernelPackage,soft-uart-driver))
$(eval $(call BuildPackage,soft-uart-driver-app))
