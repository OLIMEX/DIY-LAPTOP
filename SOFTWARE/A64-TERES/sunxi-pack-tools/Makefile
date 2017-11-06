SUBDIRS := merge_uboot update_uboot_fdt update_uboot script update_boot0 update_scp update_toc0

all: $(SUBDIRS)

bin:
	mkdir -p bin/

$(SUBDIRS): bin
	$(MAKE) -C $@
	cp -va $@/$@ bin/

clean:
	-for d in $(SUBDIRS); do ($(MAKE) -C $$d clean); done
	-$(RM) -rf bin

.PHONY: all clean $(SUBDIRS)
