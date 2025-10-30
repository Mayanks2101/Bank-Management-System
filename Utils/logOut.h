

int log_out(int nsd){
    char writeBuffer[BUFF_SIZE], readBuffer[BUFF_SIZE];
    int writeBytes, readBytes;

    // Exit
    // memset(writeBuffer, 0, BUFF_SIZE);
    // strcpy(writeBuffer, "Logging out...\n");
    // writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    // if(writeBytes < 0){
    //     perror("Write to client failed");
    //     return 0;
    // }
    // readBytes = read(nsd, readBuffer, sizeof(readBuffer));
    return 1;
}

int exit_client(int nsd){
    char writeBuffer[BUFF_SIZE];
    int writeBytes;

    // Exit
    // memset(writeBuffer, 0, BUFF_SIZE);
    // strcpy(writeBuffer, "Exiting...\n");
    // writeBytes = write(nsd, writeBuffer, strlen(writeBuffer));
    // if(writeBytes < 0){
    //     perror("Write to client failed");
    //     return 0;
    // }

    close(nsd);
    exit(0);
}