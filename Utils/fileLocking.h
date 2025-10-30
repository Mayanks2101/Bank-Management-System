
int lockFile(int fd, short lockType, off_t offset, off_t length) {
    struct flock lock;
    lock.l_type = lockType; // F_RDLCK, F_WRLCK, F_UNLCK
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;
    lock.l_len = length;

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Failed to lock file");
        return -1;
    }
    return 0;
}

int unlockFile(int fd, off_t offset, off_t length) {
    struct flock lock;
    lock.l_type = F_UNLCK; // F_RDLCK, F_WRLCK, F_UNLCK
    lock.l_whence = SEEK_SET;
    lock.l_start = offset;
    lock.l_len = length;

    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("Failed to lock file");
        return -1;
    }
    return 0;
}