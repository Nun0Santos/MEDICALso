all: medico cliente balcao func_balcao.o

balcao: func_balcao.o
	gcc balcao.c func_balcao.o -o balcao

medico:
	gcc medico.c -o medico

cliente:
	gcc cliente.c -o cliente

func_balcao.o: 
	     gcc -c func_balcao.c -o func_balcao.o

clean:
	rm medico balcao cliente func_balcao.o
