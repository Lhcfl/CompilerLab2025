# GNU make手册：http://www.gnu.org/software/make/manual/make.html
# ************ 遇到不明白的地方请google以及阅读手册 *************

# 编译器设定和编译选项
# CC = gcc -DCMM_DEBUG_FLAGTRACE -O0 -DDEBUG -ggdb -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -lrt -fno-sanitize-recover -fstack-protector -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
CC := gcc

NODE = node
FLEX = flex
BISON = bison
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic

# 编译目标：src目录下的所有.c文件
CFILES = $(shell find Code/ -name "*.c")
OBJS = $(CFILES:.c=.o)
LFILE = $(shell find Code/ -name "*.l")
YFILE = $(shell find Code/ -name "*.y")
LFC = $(shell find Code/ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find Code/ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)

parser: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) -o parser $(filter-out $(LFO),$(OBJS)) -lfl -ly
parser-noly: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) -o parser $(filter-out $(LFO),$(OBJS)) -lfl

syntax: lexical syntax-c
	$(CC) -c $(YFC) -o $(YFO)

lexical: $(LFILE)
	$(NODE) make_lexical.js
	$(FLEX) -o $(LFC) $(LFILE)

syntax-c: $(YFILE)
	$(NODE) make_syntax.js
	$(BISON) -o $(YFC) -d -v $(YFILE)

-include $(patsubst %.o, %.d, $(OBJS))

# 定义的一些伪目标
.PHONY: clean test
test:
	./parser ../Test/test1.cmm
clean:
	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f *~