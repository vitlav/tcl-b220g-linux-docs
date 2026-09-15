# Build against the exact patched kernel source tree used on the target.
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
ARCH ?= arm64
CROSS_COMPILE ?= aarch64-linux-gnu-

ifneq ($(KERNELRELEASE),)
obj-m += tcl_acpi_audio.o
obj-m += tcl_acpi_wcd.o
obj-m += tcl_acpi_card.o
obj-m += tcl_acpi_audio_power_hold.o
obj-m += tcl_acpi_codec_reset_hold.o
obj-m += tcl_lpi_provider.o
ccflags-y += -I$(KERNEL_SRC)/sound/soc/qcom
else
.PHONY: all clean
all:
	$(MAKE) -C $(KERNEL_SRC) M=$(CURDIR) ARCH=$(ARCH) \
		CROSS_COMPILE=$(CROSS_COMPILE) KERNEL_SRC=$(KERNEL_SRC) W=1 modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(CURDIR) clean
endif
