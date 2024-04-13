
SHELL = /bin/sh

# V=0 quiet, V=1 verbose.  other values don't work.
V = 0
Q1 = $(V:1=)
Q = $(Q1:0=@)
ECHO1 = $(V:1=@:)
ECHO = $(ECHO1:0=@echo)
NULLCMD = :

#### Start of system configuration section. ####
top_srcdir = $(topdir)/.
srcdir = $(top_srcdir)/ext/-test-/string
topdir = ../../..
hdrdir = $(top_srcdir)/include
arch_hdrdir = $(extout)/include/$(arch)
PATH_SEPARATOR = :
VPATH = $(srcdir):$(arch_hdrdir)/ruby:$(hdrdir)/ruby
RUBYLIB =
RUBYOPT = -
prefix = $(DESTDIR)/usr/local
rubysitearchprefix = $(rubylibprefix)/$(sitearch)
rubyarchprefix = $(rubylibprefix)/$(arch)
rubylibprefix = $(libdir)/$(RUBY_BASE_NAME)
exec_prefix = $(prefix)
vendorarchhdrdir = $(vendorhdrdir)/$(sitearch)
sitearchhdrdir = $(sitehdrdir)/$(sitearch)
rubyarchhdrdir = $(rubyhdrdir)/$(arch)
vendorhdrdir = $(rubyhdrdir)/vendor_ruby
sitehdrdir = $(rubyhdrdir)/site_ruby
rubyhdrdir = $(includedir)/$(RUBY_VERSION_NAME)
vendorarchdir = $(vendorlibdir)/$(sitearch)
vendorlibdir = $(vendordir)/$(ruby_version)
vendordir = $(rubylibprefix)/vendor_ruby
sitearchdir = $(sitelibdir)/$(sitearch)
sitelibdir = $(sitedir)/$(ruby_version)
sitedir = $(rubylibprefix)/site_ruby
rubyarchdir = $(rubylibdir)/$(arch)
rubylibdir = $(rubylibprefix)/$(ruby_version)
sitearchincludedir = $(includedir)/$(sitearch)
archincludedir = $(includedir)/$(arch)
sitearchlibdir = $(libdir)/$(sitearch)
archlibdir = $(libdir)/$(arch)
ridir = $(datarootdir)/$(RI_BASE_NAME)
mandir = $(datarootdir)/man
localedir = $(datarootdir)/locale
libdir = $(exec_prefix)/lib
psdir = $(docdir)
pdfdir = $(docdir)
dvidir = $(docdir)
htmldir = $(docdir)
infodir = $(datarootdir)/info
docdir = $(datarootdir)/doc/$(PACKAGE)
oldincludedir = $(DESTDIR)/usr/include
includedir = $(prefix)/include
localstatedir = $(prefix)/var
sharedstatedir = $(prefix)/com
sysconfdir = $(prefix)/etc
datadir = $(datarootdir)
datarootdir = $(prefix)/share
libexecdir = $(exec_prefix)/libexec
sbindir = $(exec_prefix)/sbin
bindir = $(exec_prefix)/bin
archdir = $(rubyarchdir)


CC = gcc
CXX = g++
LIBRUBY = $(LIBRUBY_SO)
LIBRUBY_A = lib$(RUBY_SO_NAME)-static.a
LIBRUBYARG_SHARED = -Wl,-R$(libdir) -L$(libdir) -l$(RUBY_SO_NAME)
LIBRUBYARG_STATIC = -Wl,-R$(libdir) -L$(libdir) -l$(RUBY_SO_NAME)-static
empty =
OUTFLAG = -o $(empty)
COUTFLAG = -o $(empty)

RUBY_EXTCONF_H = extconf.h
cflags   =  $(optflags) $(debugflags) $(warnflags)
optflags = -O3 -fno-fast-math
debugflags = -ggdb3
warnflags = -Wall -Wextra -Wno-unused-parameter -Wno-parentheses -Wno-long-long -Wno-missing-field-initializers -Wunused-variable -Wpointer-arith -Wwrite-strings -Wdeclaration-after-statement -Wimplicit-function-declaration -Wdeprecated-declarations -Wno-packed-bitfield-compat
CCDLFLAGS = -fPIC
CFLAGS   = $(CCDLFLAGS) $(cflags)  -fPIC $(ARCH_FLAG)
INCFLAGS = -I. -I$(arch_hdrdir) -I$(hdrdir) -I$(srcdir) -I$(topdir) -I$(top_srcdir)
DEFS     = 
CPPFLAGS = -DRUBY_EXTCONF_H=\"$(RUBY_EXTCONF_H)\"  $(DEFS) $(cppflags)
CXXFLAGS = $(CCDLFLAGS) $(cxxflags) $(ARCH_FLAG)
ldflags  = -L. -fstack-protector -rdynamic -Wl,-export-dynamic
dldflags =  
ARCH_FLAG = 
DLDFLAGS = $(ldflags) $(dldflags) $(ARCH_FLAG)
LDSHARED = $(CC) -shared
LDSHAREDXX = $(CXX) -shared
AR = ar
EXEEXT = 

RUBY_INSTALL_NAME = $(RUBY_BASE_NAME)
RUBY_SO_NAME = ruby
RUBYW_INSTALL_NAME = 
RUBY_VERSION_NAME = $(RUBY_BASE_NAME)-$(ruby_version)
RUBYW_BASE_NAME = rubyw
RUBY_BASE_NAME = ruby

arch = x86_64-linux
sitearch = $(arch)
ruby_version = 2.2.0
ruby = $(topdir)/miniruby -I'$(topdir)' -I'$(top_srcdir)/lib' -I'$(extout)/$(arch)' -I'$(extout)/common'
RUBY = $(ruby)
ruby_headers = $(hdrdir)/ruby.h $(hdrdir)/ruby/ruby.h $(hdrdir)/ruby/defines.h $(hdrdir)/ruby/missing.h $(hdrdir)/ruby/intern.h $(hdrdir)/ruby/st.h $(hdrdir)/ruby/subst.h $(arch_hdrdir)/ruby/config.h $(RUBY_EXTCONF_H)

RM = rm -f
RM_RF = $(RUBY) -run -e rm -- -rf
RMDIRS = rmdir --ignore-fail-on-non-empty -p
MAKEDIRS = /bin/mkdir -p
INSTALL = /usr/bin/install -c
INSTALL_PROG = $(INSTALL) -m 0755
INSTALL_DATA = $(INSTALL) -m 644
COPY = cp
TOUCH = exit >

#### End of system configuration section. ####

preload = 

libpath = . $(topdir)
LIBPATH =  -L. -L$(topdir)
DEFFILE = 

CLEANFILES = mkmf.log
DISTCLEANFILES = 
DISTCLEANDIRS = 

extout = $(topdir)/.ext
extout_prefix = $(extout)$(target_prefix)/
target_prefix = /-test-/string
LOCAL_LIBS = 
LIBS = $(LIBRUBYARG_SHARED)  -lpthread -lgmp -ldl -lcrypt -lm   -lc
ORIG_SRCS = enc_associate.c qsort.c cstr.c init.c coderange.c normalize.c set_len.c nofree.c enc_str_buf_cat.c ellipsize.c modify.c
SRCS = $(ORIG_SRCS) 
OBJS = enc_associate.o qsort.o cstr.o init.o coderange.o normalize.o set_len.o nofree.o enc_str_buf_cat.o ellipsize.o modify.o
HDRS = $(srcdir)/extconf.h
TARGET = string
TARGET_NAME = string
TARGET_ENTRY = Init_$(TARGET_NAME)
DLLIB = $(TARGET).so
EXTSTATIC = 
STATIC_LIB = $(TARGET).a

TIMESTAMP_DIR = $(extout)/.timestamp
BINDIR        = $(extout)/bin
RUBYCOMMONDIR = $(extout)/common
RUBYLIBDIR    = $(RUBYCOMMONDIR)$(target_prefix)
RUBYARCHDIR   = $(extout)/$(arch)$(target_prefix)
HDRDIR        = $(extout)/include/ruby$(target_prefix)
ARCHHDRDIR    = $(extout)/include/$(arch)/ruby$(target_prefix)

TARGET_SO     = $(RUBYARCHDIR)/$(DLLIB)
CLEANLIBS     = $(RUBYARCHDIR)/$(TARGET).so 
CLEANOBJS     = *.o  *.bak

all:    install
static: all
.PHONY: all install static install-so install-rb
.PHONY: clean clean-so clean-static clean-rb

clean-static::
clean-rb-default::
clean-rb::
clean-so::
clean: clean-so clean-static clean-rb-default clean-rb
		-$(Q)$(RM) $(CLEANLIBS) $(CLEANOBJS) $(CLEANFILES) .*.time

distclean-rb-default::
distclean-rb::
distclean-so::
distclean-static::
distclean: clean distclean-so distclean-static distclean-rb-default distclean-rb
		-$(Q)$(RM) Makefile $(RUBY_EXTCONF_H) conftest.* mkmf.log
		-$(Q)$(RM) core ruby$(EXEEXT) *~ $(DISTCLEANFILES)
		-$(Q)$(RMDIRS) $(DISTCLEANDIRS) 2> /dev/null || true

realclean: distclean
install: install-so install-rb

install-so: $(RUBYARCHDIR)/$(DLLIB)
clean-so::
	-$(Q)$(RM) $(RUBYARCHDIR)/$(DLLIB)
	-$(Q)$(RMDIRS) $(RUBYARCHDIR) 2> /dev/null || true
clean-static::
	-$(Q)$(RM) $(STATIC_LIB)
install-rb: pre-install-rb install-rb-default
install-rb-default: pre-install-rb-default
pre-install-rb: Makefile
pre-install-rb-default: Makefile
pre-install-rb-default:
	@$(NULLCMD)
$(TIMESTAMP_DIR)/.RUBYARCHDIR.-.-test-.-.string.time:
	$(Q) $(MAKEDIRS) $(@D) $(RUBYARCHDIR)
	$(Q) $(TOUCH) $@

site-install: site-install-so site-install-rb
site-install-so: install-so
site-install-rb: install-rb

.SUFFIXES: .c .m .cc .mm .cxx .cpp .o .S

.cc.o:
	$(ECHO) compiling $(<)
	$(Q) $(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(COUTFLAG)$@ -c $<

.cc.S:
	$(ECHO) translating $(<)
	$(Q) $(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(COUTFLAG)$@ -S $<

.mm.o:
	$(ECHO) compiling $(<)
	$(Q) $(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(COUTFLAG)$@ -c $<

.mm.S:
	$(ECHO) translating $(<)
	$(Q) $(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(COUTFLAG)$@ -S $<

.cxx.o:
	$(ECHO) compiling $(<)
	$(Q) $(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(COUTFLAG)$@ -c $<

.cxx.S:
	$(ECHO) translating $(<)
	$(Q) $(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(COUTFLAG)$@ -S $<

.cpp.o:
	$(ECHO) compiling $(<)
	$(Q) $(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(COUTFLAG)$@ -c $<

.cpp.S:
	$(ECHO) translating $(<)
	$(Q) $(CXX) $(INCFLAGS) $(CPPFLAGS) $(CXXFLAGS) $(COUTFLAG)$@ -S $<

.c.o:
	$(ECHO) compiling $(<)
	$(Q) $(CC) $(INCFLAGS) $(CPPFLAGS) $(CFLAGS) $(COUTFLAG)$@ -c $<

.c.S:
	$(ECHO) translating $(<)
	$(Q) $(CC) $(INCFLAGS) $(CPPFLAGS) $(CFLAGS) $(COUTFLAG)$@ -S $<

.m.o:
	$(ECHO) compiling $(<)
	$(Q) $(CC) $(INCFLAGS) $(CPPFLAGS) $(CFLAGS) $(COUTFLAG)$@ -c $<

.m.S:
	$(ECHO) translating $(<)
	$(Q) $(CC) $(INCFLAGS) $(CPPFLAGS) $(CFLAGS) $(COUTFLAG)$@ -S $<

$(RUBYARCHDIR)/$(DLLIB): $(OBJS) Makefile $(TIMESTAMP_DIR)/.RUBYARCHDIR.-.-test-.-.string.time
	$(ECHO) linking shared-object -test-/string/$(DLLIB)
	-$(Q)$(RM) $(@)
	$(Q) $(LDSHARED) -o $@ $(OBJS) $(LIBPATH) $(DLDFLAGS) $(LOCAL_LIBS) $(LIBS)

$(STATIC_LIB): $(OBJS)
	-$(Q)$(RM) $(@)
	$(ECHO) linking static-library $(@)
	$(Q) $(AR) cru $@ $(OBJS)
	-$(Q)ranlib $(@) 2> /dev/null || true

###
$(OBJS): $(RUBY_EXTCONF_H)

# AUTOGENERATED DEPENDENCIES START
coderange.o: $(RUBY_EXTCONF_H)
coderange.o: $(arch_hdrdir)/ruby/config.h
coderange.o: $(hdrdir)/ruby/defines.h
coderange.o: $(hdrdir)/ruby/encoding.h
coderange.o: $(hdrdir)/ruby/intern.h
coderange.o: $(hdrdir)/ruby/missing.h
coderange.o: $(hdrdir)/ruby/oniguruma.h
coderange.o: $(hdrdir)/ruby/ruby.h
coderange.o: $(hdrdir)/ruby/st.h
coderange.o: $(hdrdir)/ruby/subst.h
coderange.o: coderange.c
cstr.o: $(RUBY_EXTCONF_H)
cstr.o: $(arch_hdrdir)/ruby/config.h
cstr.o: $(hdrdir)/ruby/defines.h
cstr.o: $(hdrdir)/ruby/encoding.h
cstr.o: $(hdrdir)/ruby/intern.h
cstr.o: $(hdrdir)/ruby/missing.h
cstr.o: $(hdrdir)/ruby/oniguruma.h
cstr.o: $(hdrdir)/ruby/ruby.h
cstr.o: $(hdrdir)/ruby/st.h
cstr.o: $(hdrdir)/ruby/subst.h
cstr.o: $(top_srcdir)/include/ruby.h
cstr.o: cstr.c
ellipsize.o: $(RUBY_EXTCONF_H)
ellipsize.o: $(arch_hdrdir)/ruby/config.h
ellipsize.o: $(hdrdir)/ruby/defines.h
ellipsize.o: $(hdrdir)/ruby/intern.h
ellipsize.o: $(hdrdir)/ruby/missing.h
ellipsize.o: $(hdrdir)/ruby/ruby.h
ellipsize.o: $(hdrdir)/ruby/st.h
ellipsize.o: $(hdrdir)/ruby/subst.h
ellipsize.o: $(top_srcdir)/include/ruby.h
ellipsize.o: ellipsize.c
enc_associate.o: $(RUBY_EXTCONF_H)
enc_associate.o: $(arch_hdrdir)/ruby/config.h
enc_associate.o: $(hdrdir)/ruby/defines.h
enc_associate.o: $(hdrdir)/ruby/encoding.h
enc_associate.o: $(hdrdir)/ruby/intern.h
enc_associate.o: $(hdrdir)/ruby/missing.h
enc_associate.o: $(hdrdir)/ruby/oniguruma.h
enc_associate.o: $(hdrdir)/ruby/ruby.h
enc_associate.o: $(hdrdir)/ruby/st.h
enc_associate.o: $(hdrdir)/ruby/subst.h
enc_associate.o: $(top_srcdir)/include/ruby.h
enc_associate.o: enc_associate.c
enc_str_buf_cat.o: $(RUBY_EXTCONF_H)
enc_str_buf_cat.o: $(arch_hdrdir)/ruby/config.h
enc_str_buf_cat.o: $(hdrdir)/ruby/defines.h
enc_str_buf_cat.o: $(hdrdir)/ruby/encoding.h
enc_str_buf_cat.o: $(hdrdir)/ruby/intern.h
enc_str_buf_cat.o: $(hdrdir)/ruby/missing.h
enc_str_buf_cat.o: $(hdrdir)/ruby/oniguruma.h
enc_str_buf_cat.o: $(hdrdir)/ruby/ruby.h
enc_str_buf_cat.o: $(hdrdir)/ruby/st.h
enc_str_buf_cat.o: $(hdrdir)/ruby/subst.h
enc_str_buf_cat.o: enc_str_buf_cat.c
init.o: $(RUBY_EXTCONF_H)
init.o: $(arch_hdrdir)/ruby/config.h
init.o: $(hdrdir)/ruby/defines.h
init.o: $(hdrdir)/ruby/intern.h
init.o: $(hdrdir)/ruby/missing.h
init.o: $(hdrdir)/ruby/ruby.h
init.o: $(hdrdir)/ruby/st.h
init.o: $(hdrdir)/ruby/subst.h
init.o: $(top_srcdir)/include/ruby.h
init.o: init.c
modify.o: $(RUBY_EXTCONF_H)
modify.o: $(arch_hdrdir)/ruby/config.h
modify.o: $(hdrdir)/ruby/defines.h
modify.o: $(hdrdir)/ruby/intern.h
modify.o: $(hdrdir)/ruby/missing.h
modify.o: $(hdrdir)/ruby/ruby.h
modify.o: $(hdrdir)/ruby/st.h
modify.o: $(hdrdir)/ruby/subst.h
modify.o: $(top_srcdir)/include/ruby.h
modify.o: modify.c
normalize.o: $(RUBY_EXTCONF_H)
normalize.o: $(arch_hdrdir)/ruby/config.h
normalize.o: $(hdrdir)/ruby/defines.h
normalize.o: $(hdrdir)/ruby/encoding.h
normalize.o: $(hdrdir)/ruby/intern.h
normalize.o: $(hdrdir)/ruby/io.h
normalize.o: $(hdrdir)/ruby/missing.h
normalize.o: $(hdrdir)/ruby/oniguruma.h
normalize.o: $(hdrdir)/ruby/ruby.h
normalize.o: $(hdrdir)/ruby/st.h
normalize.o: $(hdrdir)/ruby/subst.h
normalize.o: $(top_srcdir)/include/ruby.h
normalize.o: $(top_srcdir)/internal.h
normalize.o: normalize.c
qsort.o: $(RUBY_EXTCONF_H)
qsort.o: $(arch_hdrdir)/ruby/config.h
qsort.o: $(hdrdir)/ruby/defines.h
qsort.o: $(hdrdir)/ruby/encoding.h
qsort.o: $(hdrdir)/ruby/intern.h
qsort.o: $(hdrdir)/ruby/missing.h
qsort.o: $(hdrdir)/ruby/oniguruma.h
qsort.o: $(hdrdir)/ruby/ruby.h
qsort.o: $(hdrdir)/ruby/st.h
qsort.o: $(hdrdir)/ruby/subst.h
qsort.o: $(hdrdir)/ruby/util.h
qsort.o: $(top_srcdir)/include/ruby.h
qsort.o: qsort.c
set_len.o: $(RUBY_EXTCONF_H)
set_len.o: $(arch_hdrdir)/ruby/config.h
set_len.o: $(hdrdir)/ruby/defines.h
set_len.o: $(hdrdir)/ruby/intern.h
set_len.o: $(hdrdir)/ruby/missing.h
set_len.o: $(hdrdir)/ruby/ruby.h
set_len.o: $(hdrdir)/ruby/st.h
set_len.o: $(hdrdir)/ruby/subst.h
set_len.o: $(top_srcdir)/include/ruby.h
set_len.o: set_len.c
# AUTOGENERATED DEPENDENCIES END
