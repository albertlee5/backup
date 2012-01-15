/*
 *   main.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2005
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

// Standard Linux headers
#include     <stdio.h>	// Always include this header
#include     <stdlib.h>	// Always include this header
#include     <signal.h>	// Defines signal-handling functions (i.e. trap Ctrl-C)
#include     <unistd.h>

// Application headers
#include     "debug.h"
#include     "video_thread.h"
#include     "audio_thread.h"
#include     "thread.h"

/* Global thread environments */
video_thread_env video_env = {0};
audio_thread_env audio_env = {0};

/* Store previous signal handler and call it */
void (*pSigPrev)(int sig);

/* Callback called when SIGINT is sent to the process (Ctrl-C) */
void signal_handler(int sig) {
    DBG( "Ctrl-C pressed, cleaning up and exiting..\n" );

    video_env.quit = 1;
    #ifdef _DEBUG_
	sleep(1);
    #endif
    audio_env.quit = 1;

    if( pSigPrev != NULL )
        (*pSigPrev)( sig );
}

//*****************************************************************************
//*  main
//*****************************************************************************
int main(int argc, char *argv[])
{

    pthread_t audioThread, videoThread;
    void *audioThreadReturn, *videoThreadReturn;

/* The levels of initialization for initMask */
#define VIDEOTHREADCREATED      0x2
#define AUDIOTHREADCREATED      0x4
    unsigned int    initMask  = 0;
    int             status    = EXIT_SUCCESS;

    /* Set the signal callback for Ctrl-C */
    pSigPrev = signal( SIGINT, signal_handler );

    /* Make video frame buffer visible */
    system("cd ..; ./vid1Show");
    printf( "\tPress Ctrl-C to exit\n" );
    


    /* Create a thread for video */
    DBG( "Creating video thread\n" );
    //videoThreadReturn = video_thread_fxn( (void *) &video_env );
    if(launch_pthread(&videoThread, TIMESLICE, 0, video_thread_fxn, (void *) &video_env) != thread_SUCCESS){
        ERR("pthread create faild for  video thread\n");
        status = EXIT_FAILURE;
        goto cleanup;

    }
    initMask |= VIDEOTHREADCREATED;

#ifdef _DEBUG_
    sleep(1);
#endif
    
    DBG( "Creating audio thread\n" );
    if(launch_pthread(&audioThread, REALTIME, 99, audio_thread_fxn, (void *) &audio_env) != thread_SUCCESS){
        ERR("pthread create faild for  audio thread\n");
        status = EXIT_FAILURE;
        goto cleanup;

    }
    initMask |= AUDIOTHREADCREATED;

cleanup:
    /* Make video frame buffer invisible */
    
    if(initMask & VIDEOTHREADCREATED)
        pthread_join(videoThread, (void **) &videoThreadReturn);
    if(initMask & AUDIOTHREADCREATED)
        pthread_join(audioThread, (void **) &audioThreadReturn);

    system("cd ..; ./resetVideo");
    exit( status );
}
