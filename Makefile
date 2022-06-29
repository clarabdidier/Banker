banker: final.c
	gcc $< -o $@

clean: 
	rm banker result.txt