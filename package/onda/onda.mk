#############################################################
#
# onda app
#
#############################################################
ONDA_VERSION = 1.0
ONDA_OVERRIDE_SRCDIR = $(TOPDIR)/onda
ONDA_INSTALL_TARGET = YES
ONDA_DEPENDENCIES = linux

define ONDA_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules
endef

define ONDA_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(LINUX_MAKE_FLAGS) -C $(LINUX_DIR) M=$(@D) modules_install
endef

$(eval $(call GENTARGETS))
