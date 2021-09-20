LIBPCRE2_OBJs = \
       pcre2_auto_possess.o \
       pcre2_chartables.o \
       pcre2_compile.o \
       pcre2_config.o \
       pcre2_context.o \
       pcre2_convert.o \
       pcre2_dfa_match.o \
       pcre2_error.o \
       pcre2_extuni.o \
       pcre2_find_bracket.o \
       pcre2_jit_compile.o \
       pcre2_maketables.o \
       pcre2_match.o \
       pcre2_match_data.o \
       pcre2_newline.o \
       pcre2_ord2utf.o \
       pcre2_pattern_info.o \
       pcre2_script_run.o \
       pcre2_serialize.o \
       pcre2_string_utils.o \
       pcre2_study.o \
       pcre2_substitute.o \
       pcre2_substring.o \
       pcre2_tables.o \
       pcre2_ucd.o \
       pcre2_valid_utf.o \
       pcre2_xclass.o


CFLAGS += \
	-DPCRE2_CODE_UNIT_WIDTH=8 \
	-DHAVE_CONFIG_H \
	-DPCRE2_EXP_DECL= \
	-DPCRE2_EXP_DEFN=


libpcre2.a: $(LIBPCRE2_OBJs)
	ar -crs $@ $^
