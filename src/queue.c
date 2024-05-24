#include "../include/queue.h"

#define key_zin "../src/queue.c"
#define key_id 65

int createQueue()
{
    key_t key = ftok(key_zin, key_id);
    if (key == -1)
    {
        fprintf(stderr, "Error from ftok: %s\n", strerror(errno));
        return -1;
    }

    int queueID = msgget(key, 0666 | IPC_CREAT);//ket = key, 0666 = permission for all, IPC_CREAT = create the queue if it doesn't exist
    if (queueID == -1)
    {
        fprintf(stderr, "Error from msgget: %s\n", strerror(errno));
        return -1;
    }
    return queueID;
}

int openQueue()
{
    key_t key = ftok(key_zin, key_id);
    if (key == -1)
    {
        fprintf(stderr, "Error from ftok: %s\n", strerror(errno));
        return -1;
    }

    int queueID = msgget(key, 0666);
    if (queueID == -1)
    {
        fprintf(stderr, "Error from msgget: %s\n", strerror(errno));
        return -1;
    }

    return queueID;
}

int closeQueue(int queueID)
{
    return msgctl(queueID, IPC_RMID, NULL);
}