SUBDIRS = common porting_layer

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/include/internal \
	-I$(top_srcdir)/src/porting_layer/include \
	-DTAIGI_DATADIR=\"$(datadir)/libtaigi\" \
	$(USERPHRASE_CPPFLAGS) \
	$(DEFAULT_CPPFLAGS) \
	$(NULL)

noinst_HEADERS = \
	private.h \
	$(NULL)

lib_LTLIBRARIES = libtaigi.la
libtaigi_la_SOURCES = \
	compat.c \
	taigiio.c \
	taigiutil.c \
	choice.c \
	dict.c \
	tree.c \
	lomaji.c \
	mod_aux.c \
	userphrase.c \
	$(USERPHRASE_SOURCES) \
	$(NULL)

libtaigi_la_LIBADD = \
	$(top_builddir)/src/common/libcommon.la \
	$(top_builddir)/src/porting_layer/src/libporting_layer.la \
	$(NULL)

libtaigi_la_LDFLAGS = \
	-version-number $(LIBTAIGI_MAJOR):$(LIBTAIGI_MINOR):$(LIBTAIGI_REVISION) \
	-rpath $(libdir) \
	-no-undefined \
	$(NO_UNDEFINED_LDFLAGS) \
	-export-symbols-regex "^(taigi)_" \
	$(NULL)

if WITH_SQLITE3
libtaigi_la_SOURCES += \
	taigi-sql.c \
	userphrase-sql.c \
	$(NULL)

if WITH_INTERNAL_SQLITE3
libtaigi_la_LIBADD += \
	$(top_builddir)/thirdparty/sqlite-amalgamation/libsqlite3-internal.la \
	$(NULL)
endif

else
libtaigi_la_SOURCES += \
	hash.c \
	userphrase-hash.c \
	$(NULL)
endif
