
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
srcdir = $(top_srcdir)/ext/ripper
topdir = ../..
hdrdir = $(top_srcdir)/include
arch_hdrdir = $(extout)/include/$(arch)
PATH_SEPARATOR = :
VPATH = $(srcdir):$(arch_hdrdir)/ruby:$(hdrdir)/ruby:$(topdir):$(top_srcdir)
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

CLEANFILES = mkmf.log ripper.y ripper.c ripper.E ripper.output y.output eventids1.c eventids2table.c
DISTCLEANFILES = 
DISTCLEANDIRS = 

extout = $(topdir)/.ext
extout_prefix = $(extout)$(target_prefix)/
target_prefix = 
LOCAL_LIBS = 
LIBS = $(LIBRUBYARG_SHARED)  -lpthread -lgmp -ldl -lcrypt -lm   -lc
ORIG_SRCS = eventids2.c
SRCS = $(ORIG_SRCS) ripper.c
OBJS = ripper.o
HDRS = $(srcdir)/extconf.h
TARGET = ripper
TARGET_NAME = ripper
TARGET_ENTRY = Init_$(TARGET_NAME)
DLLIB = $(TARGET).so
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
pre-install-rb-default: $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.ripper.time
install-rb-default: $(RUBYLIBDIR)/ripper/lexer.rb
$(RUBYLIBDIR)/ripper/lexer.rb: $(srcdir)/lib/ripper/lexer.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.ripper.time
	$(Q) $(COPY) $(srcdir)/lib/ripper/lexer.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/ripper/lexer.rb
install-rb-default: $(RUBYLIBDIR)/ripper/core.rb
$(RUBYLIBDIR)/ripper/core.rb: $(srcdir)/lib/ripper/core.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.ripper.time
	$(Q) $(COPY) $(srcdir)/lib/ripper/core.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/ripper/core.rb
install-rb-default: $(RUBYLIBDIR)/ripper/filter.rb
$(RUBYLIBDIR)/ripper/filter.rb: $(srcdir)/lib/ripper/filter.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.ripper.time
	$(Q) $(COPY) $(srcdir)/lib/ripper/filter.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/ripper/filter.rb
install-rb-default: $(RUBYLIBDIR)/ripper/sexp.rb
$(RUBYLIBDIR)/ripper/sexp.rb: $(srcdir)/lib/ripper/sexp.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.-.ripper.time
	$(Q) $(COPY) $(srcdir)/lib/ripper/sexp.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/ripper/sexp.rb
pre-install-rb-default: $(TIMESTAMP_DIR)/.RUBYLIBDIR.time
install-rb-default: $(RUBYLIBDIR)/ripper.rb
$(RUBYLIBDIR)/ripper.rb: $(srcdir)/lib/ripper.rb $(TIMESTAMP_DIR)/.RUBYLIBDIR.time
	$(Q) $(COPY) $(srcdir)/lib/ripper.rb $(@D)
clean-rb-default::
	-$(Q)$(RM) $(RUBYLIBDIR)/ripper.rb
pre-install-rb-default:
	$(ECHO) installing default ripper libraries
clean-rb-default::
	-$(Q)$(RMDIRS) $(RUBYLIBDIR)/ripper 2> /dev/null || true
	-$(Q)$(RMDIRS) $(RUBYLIBDIR) 2> /dev/null || true
$(TIMESTAMP_DIR)/.RUBYARCHDIR.time:
	$(Q) $(MAKEDIRS) $(@D) $(RUBYARCHDIR)
	$(Q) $(TOUCH) $@
$(TIMESTAMP_DIR)/.RUBYLIBDIR.-.ripper.time:
	$(Q) $(MAKEDIRS) $(@D) $(RUBYLIBDIR)/ripper
	$(Q) $(TOUCH) $@
$(TIMESTAMP_DIR)/.RUBYLIBDIR.time:
	$(Q) $(MAKEDIRS) $(@D) $(RUBYLIBDIR)
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

$(RUBYARCHDIR)/$(DLLIB): $(OBJS) Makefile $(TIMESTAMP_DIR)/.RUBYARCHDIR.time
	$(ECHO) linking shared-object $(DLLIB)
	-$(Q)$(RM) $(@)
	$(Q) $(LDSHARED) -o $@ $(OBJS) $(LIBPATH) $(DLDFLAGS) $(LOCAL_LIBS) $(LIBS)



###
$(OBJS): $(RUBY_EXTCONF_H)

.SUFFIXES: .y .c

GEN  = $(srcdir)/tools/generate.rb
SRC1 = $(top_srcdir)/parse.y
SRC2 = $(srcdir)/eventids2.c
BISON = bison

.SUFFIXES: .y

src: ripper.c eventids1.c eventids2table.c

ripper.o: ripper.c

.y.c:
	$(ECHO) compiling compiler $<
	$(Q) $(BISON) -t -v -oy.tab.c $<
	$(Q) sed -f $(top_srcdir)/tool/ytab.sed -e "/^#/s!y\.tab\.c!$@!" y.tab.c > $@
	@$(RM) y.tab.c

all: check
static: check

ripper.y: $(srcdir)/tools/preproc.rb $(top_srcdir)/parse.y
	$(ECHO) extracting $@ from $(top_srcdir)/parse.y
	$(Q) $(RUBY) $(top_srcdir)/tool/id2token.rb --path-separator=.$(PATH_SEPARATOR)./ --vpath=$(VPATH) id.h $(top_srcdir)/parse.y > ripper.tmp.y
	$(Q) $(RUBY) $(srcdir)/tools/preproc.rb ripper.tmp.y --output=$@
	$(Q) $(RM) ripper.tmp.y

check: $(GEN) $(SRC1) $(SRC2)
	$(ECHO) checking $(SRC1) and $(SRC2)
	$(Q) $(RUBY) $(GEN) --mode=check --ids1src=$(SRC1) --ids2src=$(SRC2)

eventids1.c: $(srcdir)/tools/generate.rb $(SRC1)
	$(ECHO) generating $@ from $(SRC1)
	$(Q) $(RUBY) $(GEN) --mode=eventids1 --ids1src=$(SRC1) --output=$@

eventids2table.c: $(srcdir)/tools/generate.rb $(SRC2)
	$(ECHO) generating $@ from $(SRC2)
	$(Q) $(RUBY) $(GEN) --mode=eventids2table --ids2src=$(SRC2) --output=$@

# Entries for Ripper maintainer

preproc: ripper.E
ripper.E: ripper.c
	$(ECHO) preprocessing ripper.c
	$(Q) $(CC) -E $(CPPFLAGS) ripper.c | $(RUBY) $(srcdir)/tools/strip.rb > $@

# AUTOGENERATED DEPENDENCIES START
ripper.o: $(RUBY_EXTCONF_H)
ripper.o: $(arch_hdrdir)/ruby/config.h
ripper.o: $(hdrdir)/ruby/defines.h
ripper.o: $(hdrdir)/ruby/encoding.h
ripper.o: $(hdrdir)/ruby/intern.h
ripper.o: $(hdrdir)/ruby/io.h
ripper.o: $(hdrdir)/ruby/missing.h
ripper.o: $(hdrdir)/ruby/oniguruma.h
ripper.o: $(hdrdir)/ruby/regex.h
ripper.o: $(hdrdir)/ruby/ruby.h
ripper.o: $(hdrdir)/ruby/st.h
ripper.o: $(hdrdir)/ruby/subst.h
ripper.o: $(hdrdir)/ruby/util.h
ripper.o: $(top_srcdir)/include/ruby.h
ripper.o: $(top_srcdir)/internal.h
ripper.o: $(top_srcdir)/node.h
ripper.o: $(top_srcdir)/regenc.h
ripper.o: $(top_srcdir)/symbol.h
ripper.o: $(top_srcdir)/vm_opts.h
ripper.o: ../../probes.h
ripper.o: eventids2.c
ripper.o: ripper.y
ripper.o: eventids1.c
ripper.o: eventids2table.c
ripper.o: id.h
ripper.o: lex.c
ripper.o: parse.h
ripper.o: ripper.c
# AUTOGENERATED DEPENDENCIES END
