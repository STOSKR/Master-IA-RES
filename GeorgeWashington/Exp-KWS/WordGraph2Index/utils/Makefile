MOD := StringTokenizer LogsRobust LevDistance gzstream

OBJ = $(addsuffix .o, $(MOD))

# pull in dependency info for *existing* .o files
-include $(OBJ:.o=.d)

%.o: %.cc %.h
	$(CC) $(CXXFLAGS) -c $< -o $@
	@$(CC) $(CXXFLAGS) -MM $*.cc > $*.d

libUtils.a: $(OBJ)
	@$(PREFIX)ar rv $@ $(OBJ)
	@ranlib $@

#ar -t libUtils.a
#nm --print-armap libUtils.a

clean:
	@-rm -f *.o *.d *.a *~ gmon.out
