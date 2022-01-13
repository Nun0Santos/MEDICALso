all: medico cliente balcao

balcao: balcao.c func_balcao.o utilis.h
	gcc balcao.c func_balcao.o -o balcao -lpthread

medico: medico.c utilis.h
	gcc medico.c -o medico -lpthread

cliente: cliente.c utilis.h
	gcc cliente.c -o cliente -lpthread

func_balcao.o: func_balcao.c utilis.h
	     gcc -c func_balcao.c -o func_balcao.o

clean:
	rm medico balcao cliente func_balcao.o server_fifo client_fifo* medico_fifo* sinal 
