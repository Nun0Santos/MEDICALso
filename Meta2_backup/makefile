all: medico cliente balcao func_balcao.c

balcao: balcao.c balcao.h utilis.h
	gcc balcao.c -o balcao

medico: medico.c medico.h utilis.h
	gcc medico.c -o medico

cliente: cliente.c cliente.h utilis.h
	gcc cliente.c -o cliente

func_balcao: func_balcao.c func_balcao.h
	     gcc func_balcao.c -o funcoes	 

clean:
	rm medico balcao cliente func_balcao
