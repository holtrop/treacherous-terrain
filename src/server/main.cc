#include <getopt.h>
#include <stdlib.h>
#include "Server.h"

int main(int argc, char *argv[])
{
    int port = 59243;
    for (;;)
    {
        static struct option long_options[] = {
            {"port", required_argument, 0, 'p'},
            {NULL, 0, 0, 0}
        };
        int opt_index = 0;
        int c = getopt_long(argc, argv, "p:",
                long_options, &opt_index);
        if (c == -1)
            break;
        switch (c)
        {
            case 'p':
                port = atoi(optarg);
                break;
        }
    }

    Server server(port);

    server.run();

    return 0;
}
