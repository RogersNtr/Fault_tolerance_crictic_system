

app: main_thread.o
	@echo "Le Maitre"
	gcc -o app main_thread.o -lpthread

main_thread.o: src/main_thread.c header/main_thread.h
	@echo "Bonjour"
	gcc -c src/main_thread.c

clean: 
	rm -f *.o src/*.o exec/*.o exec/app