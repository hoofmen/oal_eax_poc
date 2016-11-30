//
//  main.cpp
//  oal_eax_poc
//
//  Created by Osman H. Romero on 11/29/16.
//  Copyright © 2016 Osman H. All rights reserved.
//

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <fstream>
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

ALCdevice *device;
ALCcontext *context;

char* data;
ALuint buffer;
ALuint source;

ALfloat listenerPos[]={0.0f,0.0f,0.0f};
ALfloat listenerVel[]={0.0f,0.0f,0.0f};
ALfloat listenerOri[]={0.0f,0.0f,1.0f,0.0f,1.0f,0.0f};



int initAL(){
    device = alcOpenDevice(NULL);
    if(!device){
        printf("Error: No sound device detected\n");
        return 0;
    }
    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);
    if(!context){
        printf("Error: No sound context\n");
        return 0;
    }
    
    alListenerfv(AL_POSITION,listenerPos);
    alListenerfv(AL_VELOCITY,listenerVel);
    alListenerfv(AL_ORIENTATION,listenerOri);
    
    printf("OpenAL initialized correctly...\n");
    
    return 1;
}

void quitAL(){
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    delete[] data;
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

//check for endianness
bool isBigEndian(){
    int i=1;
    return !((char*)&i)[0];
}

int16_t to_int16(char* buffer, int length){
    int16_t i=0;
    if(!isBigEndian())
        for(int j=0;j<length;j++)
            ((char*)&i)[j]=buffer[j];
    else
        for(int j=0;j<length;j++)
            ((char*)&i)[3-j]=buffer[j];
    return i;
}

int32_t to_int32(char* buffer, int length)
{
    int32_t i=0;
    if(!isBigEndian())
        for(int j=0;j<length;j++)
            ((char*)&i)[j]=buffer[j];
    else
        for(int j=0;j<length;j++)
            ((char*)&i)[sizeof(int)-1-j]=buffer[j];
    return i;
}

int loadWAVFile(char* filename){
    std::ifstream wav_file;
    wav_file.open(filename,std::ios::binary);
    if(wav_file.bad()){
        printf("Error: while opening the WAV file %s\n",filename);
        return 0;
    }
    
    char* data;
    int16_t format_type, channels, byte_sample, bit_sample;
    int32_t chunk_size, sample_rate, byte_rate, data_size;
    
    printf("Trying to read %s\n", filename);
    
    /*
     read WAV file information, thanks to:
     https://github.com/confuzedskull/wavLoader/blob/master/main.cpp
     */
    wav_file.seekg(0,std::ios::beg);
    
    char file_info[4];
    
    wav_file.read(file_info,4);//RIFF
    wav_file.read(file_info,4);//chunks
    wav_file.read(file_info,4);//"WAVE" label
    wav_file.read(file_info,4);//"fmt" label
    
    wav_file.read(file_info,4);//chunk size
    chunk_size=to_int32(file_info,4);
    
    wav_file.read(file_info,2);//format type
    format_type=to_int16(file_info,2);
    
    wav_file.read(file_info,2);//channels
    channels=to_int16(file_info,2);
    
    wav_file.read(file_info,4);//sample rate
    sample_rate=to_int32(file_info,4);
    
    wav_file.read(file_info,4);//byte rate
    byte_rate=to_int32(file_info,4);
    
    wav_file.read(file_info,2);//byte sample
    byte_sample=to_int16(file_info,2);
    
    wav_file.read(file_info,2);//bit sample
    bit_sample=to_int16(file_info,2);
    
    wav_file.read(file_info,4);//"data" label
    
    wav_file.read(file_info,4);//data size
    data_size=to_int32(file_info,4);
    
    data= new char[data_size];//create a buffer to store sound data
    wav_file.read(data,data_size);//retrieve sound data
    
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);
    
    if(alGetError() != AL_NO_ERROR){
        printf("Error: generating source: %d\n",alGetError());
        return 0;
    }
    
    ALuint format=0;
    switch(bit_sample){
        case 8:
            format = channels==1?AL_FORMAT_MONO8:AL_FORMAT_STEREO8;
            break;
        case 16:
            format = channels==1?AL_FORMAT_MONO16:AL_FORMAT_STEREO16;
            break;
        default:
            printf("Unknown bit sample: %d\n",bit_sample);
            return 0;
    }
    

    
    alBufferData(buffer, format, data, data_size, sample_rate);
    if(alGetError() != AL_NO_ERROR){
        printf("error loading buffer: %d\n",alGetError());
        return 0;
    }
    ALfloat source_position[] = {0.0, 0.0, 0.0};
    ALfloat source_velocity[] = {0.0, 0.0, 0.0};
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, 1.0f);
    alSourcefv(source, AL_POSITION, source_position);
    alSourcefv(source, AL_VELOCITY, source_velocity);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    
    alEnable(AL_DISTANCE_MODEL);
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    wav_file.close();
    
    printf("WAV file loaded!\n");
    return 1;
}

void printHelp(char *argv[]){
    printf("Program should be ran as follows:\n\n");
    printf("%s image_to_be_loaded\n\n",argv[0]);
}

int main(int argc, char * argv[]) {
    if (argc < 2){
        printHelp(argv);
        return -1;
    }
    
    if (!initAL()) return -1;
    
    char* filename = argv[1];
    if (!loadWAVFile(filename)) return -1;
    
    int quit = 0;
    char option;
    while(!quit){
        option = getchar();
        switch(option){
            case 'p':
                printf("Playing %s\n",filename);
                alSourceStop(source);
                alSourcePlay(source);
                break;
            case 'q':
                quit = 1;
                alSourceStop(source);
                break;
        }
        
    }
    
    quitAL();
    return 0;
}
