AM_CPPFLAGS = -I $(srcdir)/include

AM_CFLAGS = -O3 -g0 -Wall

ARFLAGS = cr

lib_LIBRARIES = lib/libsvg.a
lib_libsvg_a_SOURCES = src/svg.c include/svg.h

include_HEADERS = include/svg.h

EXTRA_DIST = tests samples acdoxygen.m4 amdoxygen.am doxygen.svg.cfg svg.pc .gitignore .gitattributes

super-clean: clean distclean
	@find . -name 'Makefile.in' -delete
	@find . -name 'config.h.in' -delete
	@find . -name '*.tar' -delete
	@find . -name '*.tar.*' -delete
	@find . -name '*~' -delete
	@rm -rf autom4te.cache
	@rm -f aclocal.m4
	@rm -f configure
	@rm -rf src/.deps
	@rm -f texput.log

include amdoxygen.am

MOSTLYCLEANFILES = $(DX_CLEANFILES)

EXTRA_DIST += $(DX_CONFIG)

install-data-hook:
	@if test -d $(DX_HTML_DIR); \
	 then \
	   echo installing $(DX_HTML_DIR) to $(prefix)/share/doc/svg; \
	   mkdir -p $(prefix)/share/doc/svg; \
	   cp -r $(DX_HTML_DIR) $(prefix)/share/doc/svg; \
	 fi
	@echo installing svg.pc
	@mkdir -p $(prefix)/share/pkgconfig
	@cp svg.pc $(prefix)/share/pkgconfig/svg.pc

uninstall-hook:
	@rm -rf $(prefix)/share/doc/svg
	@rm -rf $(prefix)/share/pkgconfig/svg.pc

code-stats:
	@echo -n " total lines of source code: "; wc -l src/*.[ch] include/*.[ch] | tail -1 | sed -e 's/ total//' -e 's/^  *//'
	@echo -n "actual lines of source code: "; (gcc -fpreprocessed -dD -E src/*.[ch] include/*.[ch] | sed -e '/^$$/d' -e '/^# [0-9]/d' | wc -l)

