#---------------------------------------------------------
# Parser 
# generate my DFG representation of the arithmetic expression
#
# cmd: make Parser
#---------------------------------------------------------
VPATH = Src Benchmark TestData DFG Dot
Parser : ExprParser.h State.h MyParser.c Test_ExprParser.c util.c
	gcc -g -Wall -I./Src $(filter-out %.h, $^) -o $@
	#gdb Parser
	#valgrind --tool=memcheck --leak-check=full --show-reachable=yes ./$@ | tee log
	./$@

#---------------------------------------------------------
# Test Parser 
#
# cmd: make -B Test arg1="UUCI"
#---------------------------------------------------------
VPATH = Src Benchmark TestData DFG Dot
Test : Test_DFG.h Benchmark.c Test_DFG.c
	gcc -g -Wall -D$(arg1) -I./Src -I./DFG $(filter-out %.h, $^) -o $@ 
	#gdb Test
	./$@
	diff TestData/$(arg1).out TestData/$(arg1)_sw_result.txt

#---------------------------------------------------------
# Draw DFG using dot lang 
#
# cmd: make Draw arg1="uuci"
#---------------------------------------------------------
Draw : 
	dot -Tps Dot/$(arg1).dot -o Dot/$(arg1).ps

clean :
	@rm *.exe *.stackdump

VPATH = Src Benchmark TestData DFG Dot Stack
Walk : ExprParser.h State.h mystack.h TreeWalk.h TreeWalk.c Test_ExprParser.c util.c mystack.c
	gcc -g -Wall -I./Src -I./Stack $(filter-out %.h, $^) -o $@
	#gdb $@
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes ./$@ | tee log
