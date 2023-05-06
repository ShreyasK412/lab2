#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers;

  /* Additional fields here */
  u32 remaining_time;
  u32 response_time;
  u32 waiting_time; 
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */
  // initialize the remaining tiem of each process
  struct process *p;
  for (u32 i = 0; i < size; i++) {
    p = &data[i];
    p->response_time = -69;
    p->remaining_time = p->burst_time;
  }
  
  u32 t = 0;
  u32 quantum = quantum_length;
  bool finished = false;
  // each iteration is 1 unit of time
  while (!finished) {
    // "schedule our process if it arrives"
    for (u32 i = 0; i < size; i++){
      p = &data[i];
      if (p->arrival_time == 0 && t == 0) {
        TAILQ_INSERT_TAIL(&list, p, pointers);
      } else if (p->arrival_time == t + 1) {
        TAILQ_INSERT_TAIL(&list, p, pointers);
      }
    }

    // figure out the process to run
    if (!TAILQ_EMPTY(&list)) {
      p = TAILQ_FIRST(&list);
      
      // calculate response time: first time it's accessed to run
      // current time - the time the process arrived
      if (p->response_time == -69) {
        p->response_time = t - p->arrival_time;
        // printf("Running %d, response time %d\n", p->pid, p->response_time);
      }
      // before end of time slice, check if remaining time became 0
      p->remaining_time -= 1;
      quantum--;
      if (p->remaining_time == 0) {
        p->waiting_time = t - p->arrival_time - p->burst_time + 1;
        TAILQ_REMOVE(&list, p, pointers);
        quantum = quantum_length;
      }
    }
    

    // "check if we still have processes to run"
    finished = true;
    for (u32 i = 0; i < size; i++) {
      p = &data[i];
      if (p->remaining_time != 0) {
        finished = false;
        break;
      }
    }
    // if it's at the end of the time slice
    if (quantum == 0) {
      p = TAILQ_FIRST(&list);
      // move to next process
      TAILQ_REMOVE(&list, p, pointers);
      TAILQ_INSERT_TAIL(&list, p, pointers);
      quantum = quantum_length;
    }
    
    t++;
  }

  // calculate the total waiting/response time
  for (u32 i = 0; i < size; i++) {
    p = &data[i];
    total_response_time += p->response_time;
    total_waiting_time += p->waiting_time;
  }


  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
