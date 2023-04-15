banker: banker.c 
	gcc -o banker banker.c 

clean:
	rm -f *.o banker *.dat