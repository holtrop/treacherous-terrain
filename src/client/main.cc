
#include <stdlib.h>
#include <getopt.h>
#include "Client.h"

int main(int argc, char *argv[])
{
    bool fullscreen = false;
    int width = 1200;
    int height = 900;

    struct option longopts[] = {
        {"fullscreen", no_argument, NULL, 'f'},
        {"height", required_argument, NULL, 'h'},
        {"width", required_argument, NULL, 'w'},
        {NULL, 0, NULL, 0}
    };
    for (;;)
    {
        int c = getopt_long(argc, argv, "fh:w:", longopts, NULL);
        if (c == -1)
            break;
        switch (c)
        {
            case 'f':
                fullscreen = true;
                break;
            case 'h':
                height = atoi(optarg);
                break;
            case 'w':
                width = atoi(optarg);
                break;
        }
    }

    Client client;

    client.run(fullscreen, width, height);

    return 0;
}
