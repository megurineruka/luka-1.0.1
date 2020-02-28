cc = gcc
app = bin/luka
obj = kernel/data.o kernel/func.o kernel/gcc.o kernel/luka.o 
obj += package/c/c.o 
uselib = 

$(app): $(obj)
	$(cc) -o $(app) $(obj) $(uselib)

%.o: %.c
	$(cc) -c $< -o $@

clean:
	rm -rf $(app) $(obj)
