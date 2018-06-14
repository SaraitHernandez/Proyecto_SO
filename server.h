typedef struct{
  	struct pollfd fds[200];
  	int nfds, nfds_dsp, dsp[200];
}shmem_data;
