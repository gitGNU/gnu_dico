# This file is part of GNU Dico
# Copyright (C) 1998-2017 Sergey Poznyakoff
#
# GNU Dico is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Dico is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Dico.  If not, see <http://www.gnu.org/licenses/>.

moddir=@DICO_MODDIR@

mod_LTLIBRARIES=dictorg.la

dictorg_la_SOURCES = \
 crc.c\
 data.c\
 dictorg.c\
 dictstr.c

noinst_HEADERS = \
 crc.h\
 dictorg.h
dictorg_la_LIBADD = ../../lib/libdico.la @LIBZ@
AM_LDFLAGS = -module -avoid-version -no-undefined
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include
EXTRA_DIST=module.ac
