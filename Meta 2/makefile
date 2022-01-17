all: medico cliente balcao

balcao: balcao.c utilis.h
	gcc balcao.c  -o balcao -lpthread

medico: medico.c utilis.h
	gcc medico.c -o medico -lpthread

cliente: cliente.c utilis.h
	gcc cliente.c -o cliente -lpthread

clean:
	rm medico balcao cliente server_fifo client_fifo* medico_fifo* sinal
