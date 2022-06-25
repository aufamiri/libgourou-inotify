#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

static const char *deviceFile = "device.xml";
static const char *activationFile = "activation.xml";
static const char *devicekeyFile = "devicesalt";
static const char *acsmFile = 0;
static const int eventSize = sizeof(struct inotify_event);
static const int eventBufferLength = 1024 * (eventSize + 16);
static bool exportPrivateKey = false;
static const char *outputFile = 0;
static const char *outputDir = 0;
static bool resume = false;

class ACSMInotifyDownloader
{
public:
    int run()
    {
        int fd;
        int wd;
        char buffer[eventBufferLength] __attribute__((aligned(__alignof__(struct inotify_event))));
        struct inotify_event *event;
        ssize_t length;

        fd = inotify_init();
        if (fd == -1)
        {
            perror("inotify_init1");
            exit(EXIT_FAILURE);
        }

        wd = inotify_add_watch(fd, "/home/aufanabilamiri/Projects/libgourou-inotify/utils/testing", IN_CREATE);
        if (wd == -1)
        {
            fprintf(stderr, "Cannot watch '%s': %s\n", "duar", strerror(errno));
            exit(EXIT_FAILURE);
        }

        length = read(fd, buffer, sizeof(buffer));
        if (length == -1 && errno != EAGAIN)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

        if (length < 0)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }

        for (char *ptr = buffer; ptr < buffer + length; ptr += sizeof(struct inotify_event) + event->len)
        {
            event = (struct inotify_event *)ptr;

            if (event->len)
            {
                printf("New file has been created : %s", event->name);
            }
        }

        inotify_rm_watch(fd, wd);
        close(fd);
        return 0;
    }
};

int main(int argc, char *argv[])
{
    ACSMInotifyDownloader inotifyDownloader;

    inotifyDownloader.run();
    return 0;
}
