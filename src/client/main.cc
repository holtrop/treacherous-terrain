
#include <getopt.h>
#include "Client.h"

int main(int argc, char *argv[])
{
    bool fullscreen = false;
    for (;;)
    {
        int c = getopt_long(argc, argv, "f", NULL, NULL);
        if (c == -1)
            break;
        switch (c)
        {
            case 'f':
                fullscreen = true;
                break;
        }
    }

    Client client(fullscreen);

    client.run();

    return 0;
}
