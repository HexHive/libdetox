include ../Makedefs

.PHONY: all clean

all: $(EXECNAME) $(LIBNAME).so.$(LIBVERS).$(LIBMIN) $(RTLDNAME)

$(EXECNAME):
	@echo "\n>>>>>>>>>> COMPILE <<<<<<<<<<\n"
	make -C fastbt exec
	make -C secureloader exec
	make -C libs exec
	@echo "\n>>>>>>>>>> BUILD & LINK EXEC <<<<<<<<<<\n"
	$(CC) $(CFLAGS) $(CFLAGSEXEC) -c true.c
	$(LD) $(LDFLAGSEXEC) -o $(EXECNAME) true.o secureloader/*.o fastbt/*.o libs/*.o $(LDLIBSEXEC)
	cp $(EXECNAME) $(BINDIR)

$(LIBNAME).so.$(LIBVERS).$(LIBMIN):
	@echo "\n>>>>>>>>>> CLEANUP <<<<<<<<<<\n"
	make cleanobjs
	@echo "\n>>>>>>>>>> COMPILE <<<<<<<<<<\n"
	make -C fastbt lib
	make -C secureloader lib
	make -C libs lib
	@echo "\n>>>>>>>>>> BUILD & LINK LIBRARY <<<<<<<<<<\n"
	$(CC) $(CFLAGS) $(CFLAGSLIB) -c libtrue.c
	$(LD) -shared -soname=$(LIBNAME).so.$(LIBVERS) $(LDFLAGSLIB) -o $(LIBNAME).so.$(LIBVERS).$(LIBMIN) libtrue.o fastbt/*.o libs/*.o $(LDLIBSLIB)
	cp $(LIBNAME).so.$(LIBVERS).$(LIBMIN) $(LIBDIR)

$(RTLDNAME):
	@echo "\n>>>>>>>>>> CLEANUP <<<<<<<<<<\n"
	make cleanobjs
	@echo "\n>>>>>>>>>> COMPILE <<<<<<<<<<\n"
	make -C fastbt rtld
	make -C secureloader rtld
	make -C libs rtld
	@echo "\n>>>>>>>>>> BUILD & LINK RTLD <<<<<<<<<<\n"
	$(CC) $(CFLAGS) $(CFLAGSRTLD) -c trueRTLD.c
	$(LD) -shared $(LDFLAGSRTLD) -o $(RTLDNAME).so trueRTLD.o secureloader/*.o fastbt/*.o libs/*.o $(LDLIBSRTLD)
	cp $(RTLDNAME).so $(LIBDIR)

cleanobjs:
	make -C fastbt clean
	make -C secureloader clean
	rm -f *.o *~

clean:
	make -C fastbt clean
	make -C secureloader clean
	rm -f *.o *~
	rm -f $(EXECNAME)
	rm -f $(LIBNAME).so.$(LIBVERS).$(LIBMIN)
	rm -f $(RTLDNAME).so
