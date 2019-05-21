#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <onion/log.h>
#include <onion/onion.h>
#include <onion/shortcuts.h>
#include <sys/sysinfo.h>

onion *o = NULL;
char *message_dir = NULL;

/**
 * To determine whether message_dir exists,
 * create it if it does not exist.
 */
void make_dir()
{
    struct stat statbuf;
    if (0 > lstat(message_dir, &statbuf) || !S_ISDIR(statbuf.st_mode)) {
        if (0 > mkdir(message_dir, 0755)) {
            ONION_ERROR("mkdir %s error.", message_dir);
            exit(1);
        }
    }
}

/**
 * write message to file
 */
void write_file(const char *message)
{
    size_t count = strlen(message);
    char path[200];
    strcpy(path, message_dir);
    strcat(path, "/");
    strcat(path, "m_test.xml");
    int fd;
    if ((fd = open(path, O_WRONLY | O_CREAT, 0755)) < 0) {
        ONION_ERROR("open file error.");
        return;
    }
    if (count > write(fd, message, count)) {
        ONION_ERROR("write error.");
        close(fd);
        return;
    }
    close(fd);
}

/**
 * handle post request
 */
int post_data(void *_, onion_request *req, onion_response *res)
{
    if (onion_request_get_flags(req) & OR_POST) {
        const char *fail = "<response>S07</response>";
        const char *success = "<response>true</response>";
        const char *message = onion_request_get_post(req, "logistics_interface");
        if (0 == strlen(message)) {
            ONION_INFO("not data.");
            onion_response_printf(res, fail);
            return OCS_PROCESSED;
        }
        ONION_INFO("message is: %s", message);
        write_file(message);
        onion_response_printf(res, success);
    } else {
        ONION_INFO("method not is support.");
        onion_response_printf(res, "Only post method requests are supported.");
    }
    return OCS_PROCESSED;
}

/**
 * exit function
 */
void onexit(int _)
{
    ONION_INFO("Exit");
    if (o)
        onion_listen_stop(o);
}

int main(int argc, char **argv)
{
    const char *cport = NULL;
    int nprocs = get_nprocs();
    int max_threads = nprocs * 10;
    if (argc < 2) {
        ONION_ERROR("usage: %s <messageDir> [<port>] [<maxThreads>]", argv[0]);
        exit(-1);
    }

    if (argc > 1)
        message_dir = argv[1];
    make_dir();

    if (argc > 2)
        cport = argv[2];

    if (argc > 3)
        max_threads = atoi(argv[3]);

    if (NULL != cport)
        ONION_INFO("port is: %s, max_threads is: %d", cport, max_threads);
    else
        ONION_INFO("port is: 8080, max_threads is: %d", max_threads);

    o = onion_new(O_POOL);

    onion_url *urls = onion_root_url(o);

    onion_url_add(urls, "EcssTran/httpRequest/messagesHttpRequest", post_data);

    if (NULL != cport)
        onion_set_port(o, cport);

    signal(SIGTERM, onexit);
    signal(SIGINT, onexit);
    onion_listen(o);

    onion_free(o);
    return 0;
}
