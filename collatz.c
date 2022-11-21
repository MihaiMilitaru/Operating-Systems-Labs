#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#define dim 4096

int main(int argc, char* argv[]){
	
	int shm_fd;
	char *shm_name = "collatz";

	shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	
	if(shm_fd == -1){
		perror("Problem with shm_open\n");
		return errno;
	}
	
	int shm_size = argc * dim;

	int truncc = ftruncate(shm_fd, shm_size);

	if(truncc==-1){
		perror("Problem with ftruncate\n");
		shm_unlink(shm_name);
		return errno;
	}

	printf("Starting parent: %d\n", getpid());
	for(int i=1;i<argc;++i){
		pid_t pid=fork();
		if(pid<0){
			perror("Didn't fork\n");
			return errno;
		} else if(pid == 0){
				char * shm_ptr = mmap(NULL, dim, PROT_WRITE, MAP_SHARED, shm_fd, (i-1) * dim);
				
				if(shm_ptr == MAP_FAILED){
					perror("Problem with memory map in child process\n");
					shm_unlink(shm_name);
					return errno;
				}

				int argm = atoi(argv[i]);
				shm_ptr += sprintf(shm_ptr, "For number %d:", argm);
				shm_ptr += sprintf(shm_ptr, "%d ", argm);
				while(argm > 1){
					if(argm & 1){
						argm = 3 * argm +1;
					}else{
						argm = argm/2;
					}
					shm_ptr += sprintf(shm_ptr, "%d ", argm);
				}
				shm_ptr += sprintf(shm_ptr, "\n");
				printf("Done child with id = %d, parent id = %d\n", getpid(), getppid());
				munmap(shm_ptr, dim);
				exit(0);
		}
	}
	for(int i=1;i<argc;++i){
		wait(NULL);
	}
	for(int i=1;i<argc;++i){
		char* shm_ptr = mmap(NULL, dim, PROT_READ, MAP_SHARED, shm_fd, (i-1)*dim);
		if (shm_ptr == MAP_FAILED){
			perror("Problem with memory map in parent process\n");
			shm_unlink(shm_name);
			return errno;
		}
		printf("%s", shm_ptr);
		munmap(shm_ptr, dim);
	}
	printf("Done parent, id = %d\n", getppid());
	shm_unlink(shm_name);
	return 0;
}




