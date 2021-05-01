Grupo: SO-023

Alunos:
	Francisco Martins nº 51073
	Filipe Pedroso nº 51958
	Tiago Lourenço nº 46670

Comandos makefile:
	make - criar os ficheiros
	make clean - eliminar os ficheiros (.o e executável)
	make exec - executar o executável (os parâmetros são definidos no makefile)
	make valgrind - correr o valgrind (os parâmetros são definidos no makefile)


Implementação:
	Ao correr o projeto, não parece que haja limitações a este, sendo que executa as operações pedidas.
	Contudo:	
	- Os tempos parecem iguais mas é por causa do processo rápido.
	- Ao realizar a operação read os alarmes ficam temporariamente parados, ate esta estar terminada
	- O ficheiro de configuração deve ser colocado na pasta bin, sendo que os outros ficheiros
	serão igualmente criados nessa pasta
	- Para maior facilidade em apagar (com "make clean"), devem ser colocados os nomes na variável "CLEAN"
	os nomes pretendidos para os ficheiros de log e de estatisticas
	
