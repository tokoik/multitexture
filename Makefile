CXXFLAGS	= -I/usr/X11R6/include -DX11 -Wall
LDLIBS	= -L/usr/X11R6/lib -lglut -lGLU -lGL -lm
OBJECTS	= $(patsubst %.c,%.o,$(wildcard *.c))
TARGET	= multitexture

.PHONY: clean depend

$(TARGET): $(OBJECTS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	-$(RM) $(TARGET) *.o *~ .*~ core

depend:
	$(CXX) $(CXXFLAGS) -MM *.c > $(TARGET).d

-include $(wildcard *.d)
