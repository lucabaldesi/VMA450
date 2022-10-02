vma450cli:
	$(CC) -o vma450cli userspace/VMA450_userspace.c

clean:
	rm -f vma450cli
