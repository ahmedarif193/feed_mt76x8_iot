include $(TOPDIR)/rules.mk

PKG_NAME:=tr157-softwaremodules
PKG_VERSION:=v4.8.1
SHORT_DESCRIPTION:=TR181 firewall manager

PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=master
PKG_SOURCE_DATE:=2022-02-22
PKG_SOURCE_URL:=git@github.com:ahmedarif193/amx-tr157-softwaremodules.git
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=COPYING

COMPONENT:=tr157-softwaremodules

PKG_RELEASE:=1

PKG_BUILD_DEPENDS:=  +lxc:LXC_KERNEL_OPTIONS +CONFIG_KERNEL_LXC_MISC +KERNEL_LXC_MISC

define SAHInit/Install
	install -d ${PKG_INSTALL_DIR}/etc/rc.d/
	ln -sfr ${PKG_INSTALL_DIR}/etc/init.d/$(COMPONENT) ${PKG_INSTALL_DIR}/etc/rc.d/S$(CONFIG_SAH_AMX_TR181_FIREWALL_ORDER)$(COMPONENT)
endef

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=ambiorix
  SUBMENU:=Plugins
  TITLE:=$(SHORT_DESCRIPTION)
  URL:=https://gitlab.com/prpl-foundation/components/core/plugins/tr157-softwaremodules
  DEPENDS += +libamxc
  DEPENDS += +libamxp
  DEPENDS += +libamxd
  DEPENDS += +libamxb
  DEPENDS += +libamxo
  DEPENDS += +libsahtrace
  DEPENDS += +amxrt
  DEPENDS += +mod-dmext
  DEPENDS += +libfwrules
  DEPENDS += +libfwinterface
  DEPENDS += +libnetmodel
  DEPENDS += +libmnl
  DEPENDS += +libnetfilter-queue
  DEPENDS += +mod-pcm-svc
  DEPENDS += +libipat
  DEPENDS += +@LXC_SECCOMP +@LXC_BUSYBOX_OPTIONS +@LXC_NETWORKING +@LXC_KERNEL_OPTIONS +@KERNEL_LXC_MISC \
    +liblxc +lxc +lxc-checkconfig +lxc-templates +lxc-info +lxc-create +lxc-destroy +lxc-attach +lxc-ls +lxc-start +lxc-stop +lxc-monitord +lxc-monitor
  MENU:=1
endef

define Package/$(PKG_NAME)/description
	TR181 firewall manager
endef

define Build/Compile
	$(call Build/Compile/Default, STAGINGDIR=$(STAGING_DIR) CONFIGDIR=$(STAGING_DIR) PKG_CONFIG_PATH=$(STAGING_DIR)/usr/lib/pkgconfig RAW_VERSION=$(PKG_VERSION) HARDCO_HAL_DIR=$(STAGING_DIR)/usr/include CONFIG_SAH_AMX_TR181_FIREWALL_ORDER=$(CONFIG_SAH_AMX_TR181_FIREWALL_ORDER))
endef

define Build/Install
	$(call Build/Install/Default, install INSTALL=install D=$(PKG_INSTALL_DIR) DEST=$(PKG_INSTALL_DIR) CONFIGDIR=$(STAGING_DIR) PV=$(PKG_VERSION) PKG_CONFIG_LIBDIR=$(STAGING_DIR)/usr/lib/pkgconfig LIBDIR=/usr/lib INSTALL_LIB_DIR=/lib INSTALL_BIN_DIR=/bin RAW_VERSION=$(PKG_VERSION) HARDCO_HAL_DIR=$(STAGING_DIR)/usr/include CONFIG_SAH_AMX_TR181_FIREWALL_ORDER=$(CONFIG_SAH_AMX_TR181_FIREWALL_ORDER))
	find $(PKG_INSTALL_DIR) -name *.a -exec rm {} +;
	find $(PKG_INSTALL_DIR) -name *.h -exec rm {} +;
	find $(PKG_INSTALL_DIR) -name *.pc -exec rm {} +;

	$(call SAHInit/Install)
endef

define Build/InstallDev
	$(call Build/Install/Default, install INSTALL=install D=$(STAGING_DIR) DEST=$(STAGING_DIR) CONFIGDIR=$(STAGING_DIR) PV=$(PKG_VERSION) PKG_CONFIG_LIBDIR=$(STAGING_DIR)/usr/lib/pkgconfig LIBDIR=/usr/lib INSTALL_LIB_DIR=/lib INSTALL_BIN_DIR=/bin RAW_VERSION=$(PKG_VERSION) HARDCO_HAL_DIR=$(STAGING_DIR)/usr/include CONFIG_SAH_AMX_TR181_FIREWALL_ORDER=$(CONFIG_SAH_AMX_TR181_FIREWALL_ORDER))
endef

define Package/$(PKG_NAME)/install
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
	if [ -d ./files ]; then \
		$(CP) ./files/* $(1)/; \
	fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
