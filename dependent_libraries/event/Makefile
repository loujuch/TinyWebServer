SHELL = /bin/sh
INC=-I./include
CC=g++ ${INC} -std=c++11 -g
AR=ar rcs

Dependent=./dependent_libraries
DependentDir=${wildcard ${Dependent}/*}
DependentLib=${addsuffix /lib, ${DependentDir}}
DependentInclude=${addsuffix /include, ${DependentDir}}
DependentLiberaies=${foreach file, ${DependentLib}, ${wildcard ${file}/*.a}}
DependentLibA=${notdir ${DependentLiberaies}}

INC=-I./include ${addprefix -I, ${DependentInclude}}

LibDir=./lib
Lib=libevent.a

BinDir=./bin

TestSrcDir=./test
TestSrc=${notdir ${wildcard ${TestSrcDir}/*.cpp}}
TestTarget=${addsuffix _test, ${basename ${TestSrc}}}

LibSrcDir=./src
LibSrc=${notdir ${wildcard ${LibSrcDir}/*.cpp}}

ObjDir=./obj
LibObjs=${patsubst %.cpp, %.o, ${LibSrc}}

Libs=-lpthread

vpath %_test ${BinDir}
vpath %.a ${LibDir}:${DependentLib}
vpath %.cpp ${LibSrcDir}:${TestSrcDir}
vpath %.o ${ObjDir}

$(shell mkdir -p ${BinDir})
$(shell mkdir -p ${LibDir})
$(shell mkdir -p ${ObjDir})

${Lib}: ${LibObjs}
	${AR} ${LibDir}/${Lib} ${addprefix ${ObjDir}/, ${LibObjs}}

.PHONY: all
all: ${TestTarget}

%_test: %.o ${Lib} ${DependentLibA}
	${CC} ${ObjDir}/${notdir $<} ${LibDir}/${Lib} ${DependentLiberaies} -o ${BinDir}/$@ ${Libs}

${DependentLibA}:
	@+make -C ${DependentDir}

%.o: %.cpp
	${CC} -c $< -o ${ObjDir}/${notdir $@}

.PHONY: clean
clean:
	-rm -rf ${ObjDir} ${LibDir} ${BinDir}