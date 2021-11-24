all: medico cliente balcao

balcao: balcao.c balcao.h utilis.h
	gcc balcao.c -o balcao

medico: medico.c medico.h utilis.h
	gcc medico.c -o medico

cliente: cliente.c cliente.h utilis.h
	gcc cliente.c -o cliente

clean:
	rm medico balcao cliente
