#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OPENAL_ENABLE
#ifdef OPENAL_ENABLE
/* #include <AL/al.h> */
/* #include <AL/alc.h> */
/* #include <AL/alext.h>    /\* alBufferAppendWriteData_LOKI is an extension. *\/ */
#endif /* OPENAL_ENABLE */

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "audio.h"
#include "music.h"
#include "resources.h"

#ifdef OPENAL_ENABLE
/* We'll set this flag to 1 after music has been successfully initialized. */
int music_enabled = 0;

/* We'll set this to 1 as soon as we start playback, and to 0 when there's
   no more data. */
int music_playing = 0;


/* OpenAL source and buffer for streaming music. */
static ALuint music_source = 0;
static ALuint music_buffer = 0;

/* Ogg Vorbis stream information. */
static OggVorbis_File music_file;
static vorbis_info *music_info = NULL;
static int music_section = -1;    /* Streams can have multiple sections.
				     This lets Ogg Vorbis tell us which
				     section we're dealing with. */
static int music_file_loaded = 0; /* 1 if a file is loaded, 0 if not. */

/* Buffer for decoding music. We use an ALshort because we'll always request
   16-bit samples from Vorbis. If you experience skipping or other anomalies,
   increase the size of this buffer, */
#define MUSIC_BUF_SIZE  4915764
    /// (128 * 1024)
static ALshort buf[MUSIC_BUF_SIZE];
static int buf_count = 0;         /* Number of samples in the buffer. */
static int buf_pos = -1;          /* Playback position within buffer. */


void InitMusic()
{
    /* Check that InitAudio was successful. We'll be using
       a multichannel OpenAL streaming buffer for output,
       so OpenAL needs to be initialized. */
    if (!audio_enabled)
    {
        printf("Unable to initialize music since audio isn't enabled.\n");
        return;
    }

    /* Generate a streaming buffer. */
    /// alGenStreamingBuffers_LOKI(1, &music_buffer);
    alGenBuffers(1, &music_buffer);
    /* Create a source for the music. */
    alGenSources(1, &music_source);

    /* Set the source's position to be considered relative
	   to the listener. This way we won't have to update its
	   position each time the listener moves. */
    alSourcei(music_source, AL_SOURCE_RELATIVE, AL_TRUE);

    /* Assign the streaming buffer to the music source. */
    /// alSourcei(music_source, AL_BUFFER, music_buffer);

    /* Check for errors. */
    if (alGetError() != AL_NO_ERROR)
    {
        printf("Music initialization failed.\n");
        return;
    }

    printf("Music enabled.\n");
    music_enabled = 1;
}

void CleanupMusic()
{
    if (music_enabled)
    {
        /* Stop music playback. */
        alSourceStop(music_source);

        /* Delete the buffer and the source. */
        alDeleteBuffers(1, &music_buffer);
        alDeleteSources(1, &music_source);

        /* Close the music file, if one is open. */
        if (music_file_loaded)
        {
            ov_clear(&music_file);
            music_file_loaded = 0;
        }

        music_enabled = 0;
    }
}

int LoadMusic(char *filename)
{
    FILE *f;

    /* First, open the file with the normal stdio interface. */

    if ((f = fopen(filename, "r")) ==  NULL)
    {
        printf("Unable to open music file %s.\n", filename);
        return -1;
    }

    /* Now pass it to libvorbis. */
    if (ov_open(f, &music_file, NULL, 0) < 0)
    {
        printf("Unable to attach libvorbis to %s.\n", filename);
        fclose(f);
        return -1;
    }

    /* Retrieve information about this stream. */
    music_info = ov_info(&music_file, -1);

    printf("Reading %li Hz, %i-channel music from %s.\n",
	   music_info->rate,
	   music_info->channels,
	   filename);

    music_file_loaded = 1;

    return 0;
}

void StartMusic()
{
    /* If music is enabled and a file is ready to go, start playback. */
    if (music_enabled && music_file_loaded)
    {
        /// printf ("Start to play music!\n");
        /// alSourcePlay(music_source);
        music_playing = 1;
    }
}

void StopMusic()
{
    if (music_enabled)
    {
        alSourceStop(music_source);
        music_playing = 0;
    }
}

void UpdateMusic()
{
    /* ALsizei size,freq; */
    /* ALenum  format_new; */
    /* ALvoid  *data; */
    /* ALboolean  loop; /// important */
    /* int error; */
    /* printf ("before load wave\n"); */
    /* alutLoadWAVFile("reflux.wav", &format_new, &data, &size, &freq, &loop); */
    /* if(alGetError() != AL_NO_ERROR) */
    /* { */
    /*     printf("- Error creating 1 !!\n"); */
    /*     exit(2); */
    /* } */
    /* else */
    /* { */
    /*     printf("init - no errors after 1\n"); */
    /* } */
    /* printf ("%d %d %d\n",format_new, size, freq); */
    /* alBufferData(music_buffer, format_new, data, size, freq); */

    /* if ((error = alGetError()) != AL_NO_ERROR) */
    /* { */
    /*     printf("alBufferData buffer 0 : %d\n", error); */
    /*     // Delete buffers */
    /*     return 0; */
    /* } */

    /* alutUnloadWAV(format_new, data, size, freq); */
    /* if(alGetError() != AL_NO_ERROR) */
    /* { */
    /*     printf("- Error creating 3 !!\n"); */
    /*     exit(2); */
    /* } */
    /* else */
    /* { */
    /*     printf("init - no errors after 3\n"); */
    /* } */

    /* alSourcei(music_source, AL_BUFFER, music_buffer); */

    /* alSourcePlay(music_source); */
    /* if(alGetError() != AL_NO_ERROR) */
    /* { */
    /*     printf("- Error creating 4 !!\n"); */
    /*     exit(2); */
    /* } */
    /* else */
    /* { */
    /*     printf("init - no errors after 4\n"); */
    /* } */
    /* return; */

    int written;
    int format;

    if (music_enabled && music_file_loaded)
    {
        /* Do we need to fetch more data? */
        if (buf_pos == -1)
        {
            buf_count = 0;
            buf_pos = 0;

            if (music_playing)
            {
                /* libvorbisfile does not always return the full amount of
                   data requested, so loop until we have a full block. */
                while (music_playing && buf_count < MUSIC_BUF_SIZE)
                {
                    int amt;
                    amt = ov_read(&music_file,
                                  (char *)&buf[buf_count],
                                  (MUSIC_BUF_SIZE - buf_count) * 2,
                                  0, 2, 1, &music_section) / 2;

                    buf_count += amt;

                    /* End of the stream? */
                    if (amt == 0)
                    {
                        printf("End of music stream.\n");
                        music_playing = 0;
                        break;
                    }

                    /* Slow down the loop a bit. Otherwise Vorbis decoding
                       will take huge spikes of CPU and cause noticeable jolts
                       in the main loop, even though this is running in a
                       separate thread. usleep won't waste time; it'll give
                       control back to the kernel briefly. */
                    usleep(10);
                }

            }
            else
            {
                /* No more music, so fill the buffer with zeroes. */
                buf_count = MUSIC_BUF_SIZE;
                memset(buf, 0, MUSIC_BUF_SIZE*2);
            }

        }

        /* Determine the correct format. This can change at any time.
           (it probably won't, but Vorbis allows for this) */
        if (music_info->channels == 1)
            format = AL_FORMAT_MONO16;
        else
            format = AL_FORMAT_STEREO16;

        /* If we have a buffer of data, append it to the playback buffer.
           alBufferAppendWriteData_LOKI is similar to the well-documented
           alBufferAppendData, but it allows us to specify the internal storage
           format for the data. This prevents OpenAL from converting stereo data
           to mono. (With this function, we should get stereo playback.) */
        if (buf_count != 0)
        {

            /* written = alBufferAppendWriteData_LOKI(music_buffer, */
            /* 				   format, */
            /* 				   &buf[buf_pos], */
            /* 				   MUSIC_BUF_SIZE-buf_pos, */
            /* 				   music_info->rate, */
            /* 				   format); */

            alBufferData(music_buffer,
                         (ALenum) format,
                         (ALvoid *) buf,
                         (ALsizei) MUSIC_BUF_SIZE,
                         (ALsizei) music_info->rate);

            /* Check for (unlikely) errors. If something went wrong,
               disable music. */
            /* if (written < 0 || alGetError() != AL_NO_ERROR) { */
            /* printf("OpenAL error, disabling music.\n"); */
            /* CleanupMusic(); */
            /* } */

            if (alGetError() != AL_NO_ERROR)
            {
                printf("OpenAL error, disabling music.\n");
                CleanupMusic();
            }
            else
            {
                /// written = MUSIC_BUF_SIZE-buf_pos;
            }

            /* Update the buffer position based on how much data we wrote.
               If we've played the entire buffer, set the position to -1 so
               that the next call to UpdateMusic will refill the buffer. */

            /* buf_pos += written; */

            /* if (buf_pos >= buf_count) */
            /*     buf_pos = -1; */
            alSourcei(music_source, AL_BUFFER, music_buffer);
            alSourcePlay(music_source);
        }
    }
}

#else
#include "SDL/SDL_mixer.h"
/// Use SDL audio default
Mix_Music * background_music = NULL;

void InitMusic()
{
    printf ("Init music using SDL\n");
    if (!audio_enabled)
    {
        printf("Unable to initialize music since audio isn't enabled.\n");
        return;
    }

};
void CleanupMusic()
{
    Mix_FreeMusic(background_music);
};
int LoadMusic(char *filename)
{
    if ((background_music = Mix_LoadMUS(filename)) == NULL)
    {
        printf ("Unable to load music\n");
        return -1;
    }
    return 0;
};
void StartMusic()
{
    if(Mix_PlayingMusic() == 0)
    {
        //Play the music
        if(Mix_PlayMusic(background_music, -1) == -1)
        {
            printf ("Unable to play music!\n");
        }
    }
};
void StopMusic()
{
    //If the music is paused
    if( Mix_PausedMusic() != 1 )
    {
        Mix_HaltMusic();
        /// Mix_PauseMusic();
    }
};
void UpdateMusic()
{
    //If the music is paused
    if( Mix_PausedMusic() == 1 )
    {
        //Resume the music
        Mix_ResumeMusic();
    }
};

#endif /* OPENAL_ENABLE */
