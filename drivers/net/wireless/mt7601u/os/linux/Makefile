ifeq ($(WIFI_MODE),)
RT28xx_MODE = STA
else
RT28xx_MODE = $(WIFI_MODE)
endif

ifeq ($(TARGET),)
TARGET = LINUX
endif

ifeq ($(CHIPSET),)
CHIPSET = 7601U
endif

MODULE = $(word 1, $(CHIPSET))

#OS ABL - YES or NO
OSABL = NO

ifneq ($(TARGET),THREADX)
#RT28xx_DIR = home directory of RT28xx source code
RT28xx_DIR = $(shell pwd)
#RT28xx_DIR = drivers/net/wireless/mt7601u
endif

#include $(RT28xx_DIR)/os/linux/config.mk
include $(RT28xx_DIR)/drivers/net/wireless/mt7601u_util/os/linux/config.mk

RTMP_SRC_DIR = $(RT28xx_DIR)/RT$(MODULE)

#PLATFORM: Target platform
#PLATFORM = PC
#PLATFORM = 5VT
#PLATFORM = IKANOS_V160
#PLATFORM = IKANOS_V180
#PLATFORM = SIGMA
#PLATFORM = SIGMA_8622
#PLATFORM = INIC
#PLATFORM = STAR
#PLATFORM = IXP
#PLATFORM = INF_TWINPASS
#PLATFORM = INF_DANUBE
#PLATFORM = INF_AR9
#PLATFORM = INF_VR9
#PLATFORM = BRCM_6358
#PLATFORM = INF_AMAZON_SE
#PLATFORM = CAVM_OCTEON
#PLATFORM = CMPC
#PLATFORM = RALINK_2880
#PLATFORM = RALINK_3052
#PLATFORM = SMDK
#PLATFORM = RMI
#PLATFORM = RMI_64
#PLATFORM = KODAK_DC
#PLATFORM = DM6446
#PLATFORM = FREESCALE8377
#PLATFORM = BL2348
#PLATFORM = BL23570
#PLATFORM = BLUBB
#PLATFORM = BLPMP
#PLATFORM = MT85XX
#PLATFORM = NXP_TV550
#PLATFORM = MVL5
#PLATFORM = RALINK_3352
#PLATFORM = UBICOM_IPX8
#PLATFORM = INTELP6
#PLATFORM = MSTARTV
PLATFORM = JZ4780

#APSOC
ifeq ($(MODULE),3050)
PLATFORM = RALINK_3050
endif
ifeq ($(MODULE),3052)
PLATFORM = RALINK_3052
endif
ifeq ($(MODULE),3350)
PLATFORM = RALINK_3050
endif
ifeq ($(MODULE),3352)
PLATFORM = RALINK_3352
endif


#RELEASE Package
RELEASE = DPA


ifeq ($(TARGET),LINUX)
MAKE = make
endif

ifeq ($(TARGET), UCOS)
MAKE = make
endif
ifeq ($(TARGET),THREADX)
MAKE = gmake
endif

ifeq ($(TARGET), ECOS)
MAKE = make
MODULE = $(shell pwd | sed "s/.*\///" ).o
export MODULE
endif

WFLAGS += -I/home/xguo/kernel-3.10.14/drivers/net/wireless/mt7601u_util/include
WFLAGS += -I/home/xguo/kernel-3.10.14/drivers/net/wireless/mt7601u_util/ate/include
EXTRA_CFLAGS := $(WFLAGS)

ifeq ($(PLATFORM), JZ4780)
NOCOPYTO = yes
#LINUX_SRC = /home/zqtang/work/dongle/kernel
LINUX_SRC = $(RT28xx_DIR)/../../../../../kernel-3.10.14
CROSS_COMPILE = mips-linux-gnu-
ARCH:=mips
export $ARCH
endif

ifeq ($(PLATFORM),5VT)
LINUX_SRC = /home/ralink-2860-sdk-5vt-distribution/linux-2.6.17
CROSS_COMPILE = /opt/crosstool/uClibc_v5te_le_gcc_4_1_1/bin/arm-linux-
endif

ifeq ($(PLATFORM),UBICOM_IPX8)
LINUX_SRC = /home/sample/Customers/UBICOM/ubicom-linux-dist-2.1.1/linux-2.6.x
CROSS_COMPILE = ubicom32-elf-
endif

ifeq ($(PLATFORM),IKANOS_V160)
LINUX_SRC = /home/sample/projects/LX_2618_RG_5_3_00r4_SRC/linux-2.6.18
CROSS_COMPILE = mips-linux-
endif

ifeq ($(PLATFORM),IKANOS_V180)
LINUX_SRC = /home/sample/projects/LX_BSP_VX180_5_4_0r1_ALPHA_26DEC07/linux-2.6.18
CROSS_COMPILE = mips-linux-
endif

ifeq ($(PLATFORM),SIGMA)
LINUX_SRC = /root/sigma/smp86xx_kernel_source_2.7.172.0/linux-2.6.15
CROSS_COMPILE = /root/sigma/smp86xx_toolchain_2.7.172.0/build_mipsel_nofpu/staging_dir/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),SIGMA_8622)
LINUX_SRC = /home/snowpin/armutils_2.5.120.1/build_arm/linux-2.4.22-em86xx
CROSS_COMPILE = /home/snowpin/armutils_2.5.120.1/toolchain/bin/arm-elf-
CROSS_COMPILE_INCLUDE = /home/snowpin/armutils_2.5.120.1/toolchain/lib/gcc-lib/arm-elf/2.95.3
endif

ifeq ($(PLATFORM),INIC)
UCOS_SRC = /opt/uCOS/iNIC_rt2880
CROSS_COMPILE = /usr/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),STAR)
LINUX_SRC = /opt/star/kernel/linux-2.4.27-star
CROSS_COMPILE = /opt/star/tools/arm-linux/bin/arm-linux-
endif

ifeq ($(PLATFORM),RMI)
LINUX_SRC = /opt/rmi/1.7.0/linux/src/
CROSS_COMPILE = /opt/rmi/1.7.0/mipscross/nptl/bin/mips64-unknown-linux-gnu-
endif

ifeq ($(PLATFORM),RMI_64)
LINUX_SRC = /opt/rmi/1.7.0/linux_64/src/
CROSS_COMPILE = /opt/rmi/1.7.0/mipscross/nptl/bin/mips64-unknown-linux-gnu-
endif

ifeq ($(PLATFORM), RALINK_2880)
LINUX_SRC = /project/stable/RT288x/RT288x_SDK/source/linux-2.4.x
CROSS_COMPILE = /opt/buildroot-gdb/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),RALINK_3052)
LINUX_SRC = /home/peter/ap_soc/SDK_3_3_0_0/RT288x_SDK/source/linux-2.6.21.x
CROSS_COMPILE = /opt/buildroot-gcc342/bin/mipsel-linux-uclibc-
endif

ifeq ($(PLATFORM),FREESCALE8377)
LINUX_SRC = /opt/ltib-mpc8377_rds-20090309/rpm/BUILD/linux-2.6.25 
CROSS_COMPILE = /opt/freescale/usr/local/gcc-4.2.187-eglibc-2.5.187/powerpc-linux-gnu/bin/powerpc-linux-gnu-
endif

ifeq ($(PLATFORM),BL2348)
LINUX_SRC = /home/sample/Customers/BroadLight/bl234x-linux-2.6.21-small-v29
CROSS_COMPILE = mips-wrs-linux-gnu-
endif

ifeq ($(PLATFORM),BL23570)
LINUX_SRC = /home/FIBERHOME/linux-2.6.34.8
CROSS_COMPILE = mips-wrs-linux-gnu-mips_74k_softfp-glibc_small-
ARCH:=mips
export $ARCH
endif


ifeq ($(PLATFORM),BLUBB)
LINUX_SRC = /home/sample/Customers/BroadLight/UBB/gmp20/linux-2.6.21-small
CROSS_COMPILE = mips-wrs-linux-gnu-
endif

ifeq ($(PLATFORM),BLPMP)
LINUX_SRC = /home/sample/Customers/BroadLight/UBB/pmp16/bl234x-linux-2.6.21-small-v30.2
CROSS_COMPILE = mips-wrs-linux-gnu-
endif

ifeq ($(PLATFORM),PC)
# Linux 2.6
LINUX_SRC = /lib/modules/$(shell uname -r)/build
# Linux 2.4 Change to your local setting
#LINUX_SRC = /usr/src/linux-2.4
LINUX_SRC_MODULE = /lib/modules/$(shell uname -r)/kernel/drivers/net/wireless/
CROSS_COMPILE = 
endif

ifeq ($(PLATFORM),INTELP6)
LINUX_SRC = /tftpboot/IntelCE-20.0.11052.243193/project_build_i686/IntelCE/kernel-20.0.11024.238456/linux-2.6.35
CROSS_COMPILE = /tftpboot/IntelCE-20.0.11052.243193/build_i686/i686-linux-elf/bin/i686-cm-linux-
endif

ifeq ($(PLATFORM),IXP)
LINUX_SRC = /project/stable/Gmtek/snapgear-uclibc/linux-2.6.x
CROSS_COMPILE = arm-linux-
endif

ifeq ($(PLATFORM),INF_TWINPASS)
# Linux 2.6
#LINUX_SRC = /lib/modules/$(shell uname -r)/build
# Linux 2.4 Change to your local setting
LINUX_SRC = /project/stable/twinpass/release/2.0.1/source/kernel/opensource/linux-2.4.31/
CROSS_COMPILE = mips-linux-
endif

ifeq ($(PLATFORM),INF_DANUBE)
LINUX_SRC = /opt/danube/sdk/linux-2.6.16.x
CROSS_COMPILE = mips-linux-
ROOTDIR = /opt/danube/sdk
export ROOTDIR
endif

ifeq ($(PLATFORM),INF_AR9)
LINUX_SRC = /root/ar9/xR9_BSP1.2.2.0/source/kernel/opensource/linux-2.6.20/
CROSS_COMPILE = /root/ar9/ifx-lxdb26-1.0.2/gcc-3.4.4/toolchain-mips/bin/
endif

ifeq ($(PLATFORM),INF_VR9)
LINUX_SRC = /home/public/lantiq/VR9/UGW-4.2/build_dir/linux-ifxcpe_platform_vr9/linux-2.6.20.19
CROSS_COMPILE = /home/public/lantiq/VR9/UGW-4.2/staging_dir/toolchain-mips_gcc-3.4.6_uClibc-0.9.29/bin/mips-linux-
endif

ifeq ($(PLATFORM),BRCM_6358)
LINUX_SRC = 
CROSS_COMPILE = 
endif

ifeq ($(PLATFORM),INF_AMAZON_SE)
# Linux 2.6
#LINUX_SRC = /lib/modules/$(shell uname -r)/build
# Linux 2.4 Change to your local setting
LINUX_SRC = /backup/ifx/3.6.2.2/source/kernel/opensource/linux-2.4.31
#CROSS_COMPILE = mips-linux-
#LINUX_SRC = /project/Infineon/3.6.2.2/source/kernel/opensource/linux-2.4.31
CROSS_COMPILE = /opt/uclibc-toolchain/ifx-lxdb-1-2-3-external/gcc-3.3.6/toolchain-mips/R0208V35/mips-linux-uclibc/bin/
endif

ifeq ($(PLATFORM),ST)
LINUX_SRC = /opt/STM/STLinux-2.2/devkit/sources/kernel/linux0039
CROSS_COMPILE = /opt/STM/STLinux-2.2/devkit/sh4/bin/sh4-linux-
ARCH := sh
export ARCH
endif

ifeq ($(PLATFORM),CAVM_OCTEON)
OCTEON_ROOT = /usr/local/Cavium_Networks/OCTEON-SDK
LINUX_SRC = $(OCTEON_ROOT)/linux/kernel_2.6/linux
CROSS_COMPILE = mips64-octeon-linux-gnu-
endif

ifeq ($(PLATFORM),CMPC)
LINUX_SRC = /opt/fvt_11N_SDK_0807/fvt131x_SDK_11n/linux-2.6.17
CROSS_COMPILE =
endif

ifeq ($(PLATFORM),SMDK)
LINUX_SRC = /home/bhushan/itcenter/may28/linux-2.6-samsung
CROSS_COMPILE = /usr/local/arm/4.2.2-eabi/usr/bin/arm-linux-
endif

ifeq ($(PLATFORM),RALINK_3352)
LINUX_SRC = /home/sample/3352/RT288x_SDK/source/linux-2.6.21.x
CROSS_COMPILE = /opt/buildroot-gcc342/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),KODAK_DC)
SKD_SRC = C:/SigmaTel/DC1250_SDK_v1-9/sdk
CROSS_COMPILE = $(cc)
endif

ifeq ($(PLATFORM),DM6446)
LINUX_SRC = /home/fonchi/work/soc/ti-davinci
endif

ifeq ($(PLATFORM),MT85XX)
LINUX_SRC = /home/john/MTK/BDP_Linux/linux-2.6.27
CROSS_COMPILE = armv6z-mediatek-linux-gnueabi-
endif

ifeq ($(PLATFORM),NXP_TV550) 
LINUX_SRC = /data/tv550/kernel/linux-2.6.28.9
LINUX_SRC_MODULE = /data/tv550/kernel/linux-2.6.28.9/drivers/net/wireless
CROSS_COMPILE = /opt/embeddedalley/nxp_tv550/bin/mipsel-linux-
endif

ifeq ($(PLATFORM),MVL5)
LINUX_SRC = /home2/charlestu/AP-VT3426/linux-2.6.18
CROSS_COMPILE = /opt/montavista/pro/devkit/arm/v5t_le_mvl5/bin/arm_v5t_le-
endif

ifeq ($(PLATFORM),ALLWINNER)
LINUX_SRC =/opt/from_spreadwin/A31s/lichee/linux-3.3
#LINUX_SRC =/opt/Yangmin/A20/A20_V1.3/lichee/linux-3.3
CROSS_COMPILE =/opt/from_spreadwin/A31s/lichee/buildroot/output/external-toolchain/bin/arm-linux-gnueabi-
endif









#ifdef CONFIG_AP_SUPPORT
ifeq ($(RT28xx_MODE),AP)
MOD_NAME = rtutil$(MODULE)ap
endif
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
ifeq ($(RT28xx_MODE), STA)
ifneq ($(findstring 7601,$(CHIPSET)),)
MOD_NAME = mtutil$(MODULE)sta
else
MOD_NAME = rtutil$(MODULE)sta
endif
endif
#endif // CONFIG_STA_SUPPORT //

#ifdef CONFIG_APSTA_SUPPORT
ifeq ($(RT28xx_MODE), APSTA)
MOD_NAME = rtutil$(MODULE)apsta
endif
#endif // CONFIG_APSTA_SUPPORT //

obj-y := $(MOD_NAME).o

#ifdef CONFIG_AP_SUPPORT
ifeq ($(RT28xx_MODE),AP)
$(MOD_NAME)-objs := \
	../../common/rt_os_util.o\
	../../os/linux/rt_linux_symb.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/rt_usb_util.o\
	../../os/linux/rt_linux.o

ifeq ($(PLATFORM),BL2348)
$(MOD_NAME)-objs += \
	../../os/linux/vr_bdlt.o
endif

ifeq ($(PLATFORM),BLUBB)
$(MOD_NAME)-objs += \
	../../os/linux/vr_bdlt.o
endif

ifeq ($(HAS_BGFP_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../os/linux/br_ftph.o
endif
endif
#endif // CONFIG_AP_SUPPORT //


#ifdef CONFIG_STA_SUPPORT
ifeq ($(RT28xx_MODE), STA)
$(MOD_NAME)-objs := \
	../../common/rt_os_util.o\
	../../os/linux/rt_linux_symb.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/rt_usb_util.o\
	../../os/linux/rt_linux.o
endif
#endif // CONFIG_STA_SUPPORT //


#ifdef CONFIG_APSTA_SUPPORT
ifeq ($(RT28xx_MODE), APSTA)
$(MOD_NAME)-objs := \
	../../common/rt_os_util.o\
	../../os/linux/rt_linux_symb.o\
	../../os/linux/rt_rbus_pci_util.o\
	../../os/linux/rt_usb_util.o\
	../../os/linux/rt_linux.o

ifeq ($(PLATFORM),BL2348)
$(MOD_NAME)-objs += \
	../../os/linux/vr_bdlt.o
endif

ifeq ($(PLATFORM),BLUBB)
$(MOD_NAME)-objs += \
	../../os/linux/vr_bdlt.o
endif

ifeq ($(HAS_BGFP_SUPPORT),y)
$(MOD_NAME)-objs += \
	../../os/linux/br_ftph.o
endif
endif
#endif // CONFIG_APSTA_SUPPORT //

clean:
	rm -f ../../common/*.o
	rm -f ../../common/.*.{cmd,flags,d}
	rm -f ../../os/linux/*.{o,ko,mod.{o,c}}
	rm -f ../../os/linux/.*.{cmd,flags,d}
	rm -fr ../../os/linux/.tmp_versions
	rm -f ../../os/linux/Module.symvers
	rm -f ../../os/linux/Modules.symvers
	rm -f ../../os/linux/Module.markers
	rm -f ../../os/linux/modules.order
	rm -f ../../chips/*.o
	rm -f ../../chips/.*.{cmd,flags,d}
ifeq ($(RT28xx_MODE),AP)
	rm -f ../../ap/*.o
	rm -f ../../ap/.*.{cmd,flags,d}
else
ifeq ($(RT28xx_MODE),STA)
	rm -f ../../sta/*.o
	rm -f ../../sta/.*.{cmd,flags,d}
endif
endif

install:
	install -d $(LINUX_SRC_MODULE)
	install -m 644 -c $(addsuffix .ko,$(MOD_NAME)) $(LINUX_SRC_MODULE)
	/sbin/depmod -a ${shell uname -r}

uninstall:
	rm -rf $(addprefix $(LINUX_SRC_MODULE),$(addsuffix .ko,$(MOD_NAME)))
	/sbin/depmod -a ${shell uname -r}
