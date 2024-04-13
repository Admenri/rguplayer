
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
srcdir = $(top_srcdir)/ext/json
topdir = ../..
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
INCFLAGS = -I. -I$(arch_hdrdir) -I$(hdrdir) -I$(srcdir)
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
target_prefix = 
LOCAL_LIBS = 
LIBS = $(LIBRUBYARG_SHARED)  -lpthread -lgmp -ldl -lcrypt -lm   -lc
ORIG_SRCS = 
SRCS = $(ORIG_SRCS) 
OBJS = 
HDRS = $(srcdir)/extconf.h
TARGET = 
TARGET_NAME = 
TARGET_ENTRY = Init_$(TARGET_NAME)
DLLIB = 
EXTSTATIC = 
STATIC_LIB = 

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

install-so: Makefile
install-rb: pre-install-rb install-rb-default
install-rb-default: pre-install-rb-default
pre-install-rb: Makefile
pre-install-rb-default: Makefile
pre-install-rb-default: $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.time
install-rb-default: $(RUBYLIBDIR)/json/ext.rb
$(RUBYLIBDIR)/json/ext.rb: $(srcdir)/lib/json/ext.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.time
	$(Q) $(COPY) $(srcdir)/lib/json/ext.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/ext.rb
install-rb-default: $(RUBYLIBDIR)/json/generic_object.rb
$(RUBYLIBDIR)/json/generic_object.rb: $(srcdir)/lib/json/generic_object.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.time
	$(Q) $(COPY) $(srcdir)/lib/json/generic_object.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/generic_object.rb
install-rb-default: $(RUBYLIBDIR)/json/common.rb
$(RUBYLIBDIR)/json/common.rb: $(srcdir)/lib/json/common.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.time
	$(Q) $(COPY) $(srcdir)/lib/json/common.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/common.rb
install-rb-default: $(RUBYLIBDIR)/json/version.rb
$(RUBYLIBDIR)/json/version.rb: $(srcdir)/lib/json/version.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.time
	$(Q) $(COPY) $(srcdir)/lib/json/version.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/version.rb
pre-install-rb-default: $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
install-rb-default: $(RUBYLIBDIR)/json/add/date.rb
$(RUBYLIBDIR)/json/add/date.rb: $(srcdir)/lib/json/add/date.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/date.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/date.rb
install-rb-default: $(RUBYLIBDIR)/json/add/core.rb
$(RUBYLIBDIR)/json/add/core.rb: $(srcdir)/lib/json/add/core.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/core.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/core.rb
install-rb-default: $(RUBYLIBDIR)/json/add/symbol.rb
$(RUBYLIBDIR)/json/add/symbol.rb: $(srcdir)/lib/json/add/symbol.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/symbol.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/symbol.rb
install-rb-default: $(RUBYLIBDIR)/json/add/bigdecimal.rb
$(RUBYLIBDIR)/json/add/bigdecimal.rb: $(srcdir)/lib/json/add/bigdecimal.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/bigdecimal.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/bigdecimal.rb
install-rb-default: $(RUBYLIBDIR)/json/add/rational.rb
$(RUBYLIBDIR)/json/add/rational.rb: $(srcdir)/lib/json/add/rational.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/rational.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/rational.rb
install-rb-default: $(RUBYLIBDIR)/json/add/range.rb
$(RUBYLIBDIR)/json/add/range.rb: $(srcdir)/lib/json/add/range.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/range.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/range.rb
install-rb-default: $(RUBYLIBDIR)/json/add/struct.rb
$(RUBYLIBDIR)/json/add/struct.rb: $(srcdir)/lib/json/add/struct.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/struct.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/struct.rb
install-rb-default: $(RUBYLIBDIR)/json/add/date_time.rb
$(RUBYLIBDIR)/json/add/date_time.rb: $(srcdir)/lib/json/add/date_time.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/date_time.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/date_time.rb
install-rb-default: $(RUBYLIBDIR)/json/add/ostruct.rb
$(RUBYLIBDIR)/json/add/ostruct.rb: $(srcdir)/lib/json/add/ostruct.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/ostruct.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/ostruct.rb
install-rb-default: $(RUBYLIBDIR)/json/add/exception.rb
$(RUBYLIBDIR)/json/add/exception.rb: $(srcdir)/lib/json/add/exception.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/exception.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/exception.rb
install-rb-default: $(RUBYLIBDIR)/json/add/regexp.rb
$(RUBYLIBDIR)/json/add/regexp.rb: $(srcdir)/lib/json/add/regexp.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/regexp.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/regexp.rb
install-rb-default: $(RUBYLIBDIR)/json/add/time.rb
$(RUBYLIBDIR)/json/add/time.rb: $(srcdir)/lib/json/add/time.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/time.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/time.rb
install-rb-default: $(RUBYLIBDIR)/json/add/complex.rb
$(RUBYLIBDIR)/json/add/complex.rb: $(srcdir)/lib/json/add/complex.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time
	$(Q) $(COPY) $(srcdir)/lib/json/add/complex.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json/add/complex.rb
pre-install-rb-default: $(TIMESTAMP_DIR)/.RUBYLIBDIR.time
install-rb-default: $(RUBYLIBDIR)/json.rb
$(RUBYLIBDIR)/json.rb: $(srcdir)/lib/json.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.time
	$(Q) $(COPY) $(srcdir)/lib/json.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/json.rb
pre-install-rb-default:
	$(ECHO) installing default  libraries
clean-rb-default::
	-$(Q)$(RMDIRS) $(RUBYLIBDIR)/json/add 2> /dev/null || true
	-$(Q)$(RMDIRS) $(RUBYLIBDIR)/json 2> /dev/null || true
	-$(Q)$(RMDIRS) $(RUBYLIBDIR) 2> /dev/null || true
$(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.time:
	$(Q) $(MAKEDIRS) $(@D) $(RUBYLIBDIR)/json
	$(Q) $(TOUCH) $@
$(TIMESTAMP_DIR)/.RUBYLIBDIR.-.json.-.add.time:
	$(Q) $(MAKEDIRS) $(@D) $(RUBYLIBDIR)/json/add
	$(Q) $(TOUCH) $@
$(TIMESTAMP_DIR)/.RUBYLIBDIR.time:
	$(Q) $(MAKEDIRS) $(@D) $(RUBYLIBDIR)
	$(Q) $(TOUCH) $@

site-install: site-install-so site-install-rb
site-install-so: install-so
site-install-rb: install-rb

