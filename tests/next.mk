ifeq ($(compiler), gcc)
  CXX=g++
else
  CXX=clang++
  ifeq ($(sanitize), 1)
    EXTRA_CXXFLAGS+=-fsanitize=memory -fsanitize-recover=memory
  endif
	EXTRA_CXXFLAGS+=-stdlib=libc++
endif
EXTRA_CXXFLAGS+=-g3 -O0

include ../../EpoxyDuino.mk
