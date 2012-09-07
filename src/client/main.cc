
#include <stdlib.h>
#include <getopt.h>
#include "Client.h"

int main(int argc, char *argv[])
{
    bool fullscreen = false;
    unsigned int antialias_level = 0u;
    bool compatibility_context = false;

    struct option longopts[] = {
        {"antialias-level", required_argument, NULL, 'a'},
        {"compatibility", no_argument, NULL, 'c'},
        {"fullscreen", no_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}
    };
    for (;;)
    {
        int c = getopt_long(argc, argv, "a:cf", longopts, NULL);
        if (c == -1)
            break;
        switch (c)
        {
            case 'a':
                antialias_level = atoi(optarg);
                break;
            case 'c':
                compatibility_context = true;
                break;
            case 'f':
                fullscreen = true;
                break;
        }
    }

    Client client(fullscreen, compatibility_context, antialias_level);

    client.run();

    return 0;
}
