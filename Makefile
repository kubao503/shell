.RECIPEPREFIX = >

prog: main.o fooParser.tab.o lex.yy.o
> g++ $^ -o $@

main.o: main.cc FooLexer.hh fooParser.tab.hh
> g++ -c $< -o $@

lex.yy.o: lex.yy.cc FooLexer.hh fooParser.tab.hh
> g++ -c $< -o $@

fooParser.tab.o: fooParser.tab.cc FooLexer.hh
> g++ -c $< -o $@

lex.yy.cc: fooLexer.ll
> flex $<

fooParser.tab.hh fooParser.tab.cc fooParser.output: fooParser.yy
> bison $<

.PHONY: clean
clean:
> rm -f prog main.o lex.* fooParser.tab.* stack.hh
