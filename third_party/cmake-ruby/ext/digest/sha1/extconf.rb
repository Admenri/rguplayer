# -*- coding: us-ascii -*-
# $RoughId: extconf.rb,v 1.3 2001/08/14 19:54:51 knu Exp $
# $Id: extconf.rb 47802 2014-10-05 02:03:55Z nobu $

require "mkmf"

$defs << "-DHAVE_CONFIG_H"
$INCFLAGS << " -I$(srcdir)/.."

$objs = [ "sha1init.#{$OBJEXT}" ]

if !with_config("bundled-sha1") &&
    (dir_config("openssl")
     pkg_config("openssl")
     require File.expand_path('../../../openssl/deprecation', __FILE__)
     have_library("crypto")) &&
    OpenSSL.check_func("SHA1_Transform", "openssl/sha.h")
  $objs << "sha1ossl.#{$OBJEXT}"
else
  $objs << "sha1.#{$OBJEXT}"
end

have_header("sys/cdefs.h")

$preload = %w[digest]

create_makefile("digest/sha1")
