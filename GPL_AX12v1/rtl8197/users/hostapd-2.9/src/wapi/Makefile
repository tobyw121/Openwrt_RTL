all: libwapi.a

clean:
	rm -f *~ *.o *.d *.gcno *.gcda *.gcov libwapi.a

install:
	@echo Nothing to be made.

include ../lib.rules

LIB_OBJS= \
	wapi_common.o \
	wapi_timeout.o \
	wapi_ae.o \
	x509.o \
	wapi_alogrithm.o \
	wapi_sms4.o \
	ecc.o

libwapi.a: $(LIB_OBJS)
	$(AR) crT $@ $?

-include $(OBJS:%.o=%.d)
