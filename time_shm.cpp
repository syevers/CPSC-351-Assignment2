#include <iomanip>
#include <iostream>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  // Get arguments from cli
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <command [args...]>" << std::endl;
    return 1;
  }

  // Create shared memory
  // 0666 refers to permission setting for shm
  int shmId = shmget(IPC_PRIVATE, sizeof(timeval), IPC_CREAT | 0666);
  if (shmId < 0) {
    std::cerr << "Failed to create shared memory" << std::endl;
    return 1;
  }

  // Attach shared memory
  timeval *startTime = (timeval *) shmat(shmId, nullptr, 0);
  if (startTime == (void *) -1) {
    std::cerr << "Failed to attach shared memory" << std::endl;
    return 1;
  }

  // create child process
  pid_t pid = fork();
  if (pid < 0) {
    std::cerr << "Error: Failed to fork child process" << std::endl;
    return 1;
  } else if (pid == 0) {
    // Child process using execvp
    gettimeofday(startTime, nullptr);
    execvp(argv[1], argv + 1);
    std::cerr << "Failed to execute command" << std::endl;
    return 1;

  } else {
    // Parent process
    wait(nullptr);
    timeval endTime;
    gettimeofday(&endTime, nullptr);

    timeval elapsedTime;
    timersub(&endTime, startTime, &elapsedTime);

    std::cout << "Elapsed time: " << elapsedTime.tv_sec << "."
              << std::setw(6) << std::setfill('0') << elapsedTime.tv_usec
              << " seconds" << std::endl;

    // Detach and remove shared memory
    //    shmdt(startTime);
    //    shmctl(shmId, IPC_RMID, nullptr);
  }

  return 0;
}
