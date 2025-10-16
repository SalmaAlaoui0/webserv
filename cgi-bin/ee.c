/* test_epipe.c */
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

int main()
{
    int pipefd[2];
    if (pipe(pipefd) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();
    if (pid == -1) { perror("fork"); return 1; }

    if (pid == 0) {
        // enfant : simule un CGI qui meurt tout de suite
        close(pipefd[1]); // ferme côté écriture
        close(pipefd[0]); // ferme côté lecture
        _exit(0);
    } else {
        // parent : essaye d'écrire après la mort de l'enfant
        close(pipefd[0]); // on n'a pas besoin du côté lecture
        sleep(1);         // laisse le temps à l'enfant de mourir

        ssize_t n = write(pipefd[1], "HELLO", 5);
        if (n == -1) {
            std::cerr << "❌ write() failed as expected (EPIPE or similar)\n";
            perror("write");
        } else {
            std::cout << "✅ write() wrote " << n << " bytes\n";
        }

        close(pipefd[1]);
        wait(NULL);
    }
    return 0;
}
