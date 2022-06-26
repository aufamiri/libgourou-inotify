#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

#include <libgourou.h>
#include <libgourou_common.h>

#include "drmprocessorclientimpl.h"
#include "utils_common.h"

static const char *deviceFile = "device.xml";
static const char *activationFile = "activation.xml";
static const char *devicekeyFile = "devicesalt";

// inotify stuff
static const int eventSize = sizeof(struct inotify_event);
static const int eventBufferLength = 1024 * (eventSize + 16);

class ACSMInotifyDownloader
{
private:
    DRMProcessorClientImpl client;

public:
    int run(const char *path)
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

        wd = inotify_add_watch(fd, path, IN_CREATE);
        if (wd == -1)
        {
            fprintf(stderr, "Cannot watch '%s': %s\n", path, strerror(errno));
            exit(EXIT_FAILURE);
        }

        while (1)
        {
            length = read(fd, buffer, sizeof(buffer));
            if ((length == -1 && errno != EAGAIN) || length < 0)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }

            for (char *ptr = buffer; ptr < buffer + length; ptr += sizeof(struct inotify_event) + event->len)
            {
                event = (struct inotify_event *)ptr;

                if (event->len)
                {
                    char *acsmFile = event->name;

                    if (endsWith(acsmFile, ".acsm"))
                    {
                        printf("new acsm file has been detected : %s \n", event->name);
                        retreiveContent(path, acsmFile);
                    }
                }
            }
        }

        inotify_rm_watch(fd, wd);
        close(fd);
        return 0;
    }

    int retreiveContent(const char *path, const char *acsmFile)
    {
        try
        {
            gourou::DRMProcessor processor(&client, deviceFile, activationFile, devicekeyFile);

            // fulfilling metadata
            std::string acsmPathFile = std::string(path) + "/" + std::string(acsmFile);
            gourou::FulfillmentItem *item = processor.fulfill(acsmPathFile);

            std::string filename = item->getMetadata("title");
            if (filename == "")
                filename = "output";
            else
            {
                // Remove invalid characters
                std::replace(filename.begin(), filename.end(), '/', '_');
            }

            // we want to put the resulting file into the same path
            filename = std::string(path) + "/" + filename;

            // downloading content
            gourou::DRMProcessor::ITEM_TYPE type = processor.download(item, filename, false);
            std::cout << "success downloading : " << filename << std::endl;

            std::string finalName = filename;
            if (type == gourou::DRMProcessor::ITEM_TYPE::PDF)
            {
                finalName += ".pdf";
                processor.removeDRM(filename, finalName, type);
            }
            else
            {
                finalName += ".epub";
                rename(filename.c_str(), finalName.c_str());
                processor.removeDRM(finalName, finalName, type);
            }

            std::cout << "" << std::endl;
            std::cout << filename << "drm removed, result :" << finalName << std::endl;
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
        }

        return 0;
    }

    static inline bool endsWith(const std::string &s, const std::string &suffix)
    {
        return s.rfind(suffix) == std::abs((int)(s.size() - suffix.size()));
    }
};

int main(int argc, char *argv[])
{
    ACSMInotifyDownloader inotifyDownloader;

    if (argc != 2)
    {
        printf("path is required ! \n");
        return 0;
    }

    printf("%s \n", argv[1]);
    inotifyDownloader.run(argv[1]);
    return 0;
}
