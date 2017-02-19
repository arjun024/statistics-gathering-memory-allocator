UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
all:
	g++ -std=c++1y -g -fPIC -D_REENTRANT=1 -c gnuwrapper.cpp
	g++ -std=c++1y -g -fPIC -D_REENTRANT=1 -c driver.cpp
	g++ -std=c++1y -g -fPIC -D_REENTRANT=1 -shared gnuwrapper.o driver.o -Bsymbolic -o libstatsalloc.so -ldl -lpthread
	g++ -std=c++1y -g testme.cpp -L. -lstatsalloc -o testme
endif

ifeq ($(UNAME_S),Darwin)
all:
	clang++ -std=c++14 -g -fPIC -D_REENTRANT=1 -c macwrapper.cpp
	clang++ -std=c++14 -g -fPIC -D_REENTRANT=1 -c driver.cpp
	clang++ -std=c++14 -g -fPIC -D_REENTRANT=1 -compatibility_version 1 -current_version 1 -dynamiclib -install_name './libstatsalloc.dylib' macwrapper.o driver.o -o libstatsalloc.dylib
	clang++ -std=c++14 -g testme.cpp -L. -lstatsalloc -o testme
endif
