/*
Copyright (C) 2023 Chin Yik Ming

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/* snd.c -- include this instead of all the other snd_* files to have sound */

#include "quakedef.h"

cvar_t bgmvolume = {"bgmvolume", "1", true};
cvar_t volume = {"volume", "0.7", true};

#define MAX_PRECACHE_SOUND 512
static sfx_t *precache_sound[MAX_PRECACHE_SOUND] = {0};
static int precache_sound_idx = 0;

/* sound-related request type */
enum {
    INIT_AUDIO,
    SHUTDOWN_AUDIO,
    PLAY_MUSIC,       /* not used since pak0.pak does not contain music */
    PLAY_SFX,
    SET_MUSIC_VOLUME, /* not used since pak0.pak does not contain music */
    STOP_MUSIC,       /* not used since pak0.pak does not contain music */
};
 
void S_Init (void)
{
    Con_Printf("Sound init\n");

    Cvar_RegisterVariable(&volume);

    /*int request_type = INIT_AUDIO;

    register int a0 asm("a0") = request_type;
    register int a7 asm("a7") = 0xBABE;

    asm volatile("scall" : "+r"(a0) : "r"(a7));*/
}

void S_AmbientOff (void)
{
}

void S_AmbientOn (void)
{
}

void S_Shutdown (void)
{
    Con_Printf("Sound shutdown\n");

    /*int request_type = SHUTDOWN_AUDIO;

    register int a0 asm("a0") = request_type;
    register int a7 asm("a7") = 0xBABE;

    asm volatile("scall" : "+r"(a0) : "r"(a7));*/

}

void S_TouchSound (char *sample)
{
}

void S_ClearBuffer (void)
{
}

void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
}

static sfx_t *S_FindcacheSound (char *sample);
static sfx_t *S_CacheSound(char *sample);

sfxcache_t sfxcache;
sfxcache_t *S_LoadSound (sfx_t *sfx)
{
    char buf[64];
    sprintf(buf, "sound/%s", sfx->name);
    
    sfx_t *tmp = S_FindcacheSound(buf);
    if(tmp)
        goto done;

    tmp = S_CacheSound(sfx->name);

done:
    sfxcache.data = tmp->cache.data;
    sfxcache.length = tmp->cache_data_size;

    return &sfxcache;
}

void S_StartSound (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol,  float attenuation)
{
    if(!sfx)
        return;

    sfxcache_t *sfxcache = S_LoadSound(sfx);

    /*int request_type = PLAY_SFX;

    register int a0 asm("a0") = request_type;
    register int a1 asm("a1") = (uintptr_t) sfxcache;
    register int a2 asm("a2") = (int) (volume.value * 255);
    register int a7 asm("a7") = 0xD00D;

    asm volatile("scall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7));*/
}

void S_StopSound (int entnum, int entchannel)
{
}

static sfx_t *S_FindcacheSound (char *sample)
{
    for(int i = 0; i < precache_sound_idx; i++){
        if(!strcmp(precache_sound[i]->name, sample))
            return precache_sound[i];
    }

    return NULL;
}

extern int len_for_emu;
static sfx_t *S_CacheSound(char *sample)
{ 
    sfx_t *sfx;
    byte *data;

    sfx = malloc(sizeof(sfx_t));
    if(!sfx){
        Con_Printf("S_CacheSound malloc failed\n");
        return NULL;
    }
    strcpy(sfx->name, sample);

    data = COM_LoadTempFile(sample);
    sfx->cache.data = data;
    sfx->cache_data_size = len_for_emu;

    precache_sound[precache_sound_idx++] = sfx;

    return sfx;
}

sfx_t *S_PrecacheSound (char *sample)
{
    char buf[64];
    sprintf(buf, "sound/%s", sample);

    sfx_t *sfx = S_FindcacheSound(buf);
    if(sfx)
        return sfx;
    
    return S_CacheSound(buf);
}

void S_ClearPrecache (void)
{
}

void S_Update (vec3_t origin, vec3_t v_forward, vec3_t v_right, vec3_t v_up)
{    
}

void S_StopAllSounds (qboolean clear)
{
}

void S_BeginPrecaching (void)
{
}

void S_EndPrecaching (void)
{
}

void S_ExtraUpdate (void)
{
}

void S_LocalSound (char *s)
{
    sfx_t *sfx = S_PrecacheSound(s);
    if(!sfx){
        Con_Printf("S_LocalSound: cannot cache %s\n", s);
        return;
    }

    S_StartSound(-1, 0, sfx, vec3_origin, 1, 1);
}

