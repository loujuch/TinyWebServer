SHELL = /bin/sh
INC=-I./include
CC=g++ ${INC} -std=c++11 -g

Dependent=./dependent_libraries
DependentDir=${wildcard ${Dependent}/*}
DependentLib=${addsuffix /lib, ${DependentDir}}
DependentInclude=${addsuffix /include, ${DependentDir}}
DependentLiberaies=${foreach file, ${DependentLib}, ${wildcard ${file}/*.a}}
DependentLibA=${notdir ${DependentLiberaies}}

INC=-I./include ${addprefix -I, ${DependentInclude}}

BinDir=./bin

Target=WebServer

SrcDir=./src
Src=${notdir ${wildcard ${SrcDir}/*.cpp}}

ObjDir=./obj
Objs=${patsubst %.cpp, %.o, ${Src}}

Libs=-lpthread

vpath %_test ${BinDir}
vpath %.a ${DependentLib}
vpath %.cpp ${SrcDir}
vpath %.o ${ObjDir}

$(shell mkdir -p ${BinDir})
$(shell mkdir -p ${ObjDir})

${Target}: ${Objs} ${DependentLibA}
	${CC} ${addprefix ${ObjDir}/, ${Objs}} ${DependentLiberaies} -o ${BinDir}/$@ ${Libs}

${DependentLibA}:
	@+make -C ${DependentDir}

%.o: %.cpp
	${CC} -c $< -o ${ObjDir}/${notdir $@}

.PHONY: clean
clean:
	-rm -rf ${ObjDir} ${BinDir}