PROG=ms2bin
SRCS=ms2bin.c main.c
TARGETS=all $(PROG) %.o %.d run line clean
NONEED_DEP_TARGETS+=clean line
LDLIBS_LOCAL+=-lm -lms

.PHONY: all
all: $(PROG)

OBJS=$(SRCS:%.c=%.c.o)
DEPS=$(SRCS:%.c=%.c.d)
ALLDEP=$(MAKEFILE_LIST_SANS_DEPS)
TOCLEAN=gmon.out

EXTRA_TARGETS=$(filter-out $(TARGETS), $(MAKECMDGOALS))
ifneq '$(EXTRA_TARGETS)' ''
 $(error No rule to make target `$(word 1, $(EXTRA_TARGETS))')
else
 # check whether NONEED_DEP_TARGETS are in MAKECMDGOALS
 ifeq '$(filter-out $(NONEED_DEP_TARGETS), $(MAKECMDGOALS))' '$(MAKECMDGOALS)'
  sinclude $(DEPS)
 else
  # if so and there are more than 1 targets in MAKECMDGOALS, it would cause dependency files missing so say error
  ifneq '$(words $(MAKECMDGOALS))' '1'
   $(error Specify only one target if you want to make target that needs no dependency file)
  endif
 endif
endif

MAKEFILE_LIST_SANS_DEPS=$(filter-out %.d, $(MAKEFILE_LIST))

CC:=gcc
HEADERFLAGS:=-I.
OPTFLAGS:=-pipe -O2 -g
WARNFLAGS:=-ansi -pedantic -posix -Wall -Wextra -W -Wundef -Wshadow -Wcast-qual -Winline -Wno-long-long -fsigned-char
RM:=rm -r -f
WC:=wc -c -l

COMPILE.c=$(CC) $(HEADERFLAGS) $(OPTFLAGS) $(WARNFLAGS) $(ADDCFLAGS) $(CFLAGS) -c
COMPILE.dep=$(CC) $(HEADERFLAGS) $(OPTFLAGS) $(WARNFLAGS) $(ADDCFLAGS) $(CFLAGS) -M -MP -MT $<.o -MF $@
LINK.o=$(CC) $(OPTFLAGS) $(WARNFLAGS) $(LDFLAGS)

$(PROG): $(OBJS) $(ALLDEP)
	$(LINK.o) $(OUTPUT_OPTION) $(OBJS) $(LOADLIBES) $(LDLIBS) $(LDLIBS_LOCAL)

%.c.o: %.c $(ALLDEP)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

%.c.d: %.c $(ALLDEP)
	$(COMPILE.dep) $<

.PHONY: line
line:
	$(WC) $(SRCS) $(MAKEFILE_LIST_SANS_DEPS)

.PHONY: clean
clean:
	$(RM) $(PROG) $(OBJS)
	$(RM) $(DEPS)
	$(RM) $(TOCLEAN)
