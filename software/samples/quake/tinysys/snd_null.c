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
    /*Con_Printf("Sound init\n");

    Cvar_RegisterVariable(&volume);*/
}

void S_AmbientOff (void)
{
}

void S_AmbientOn (void)
{
}

void S_Shutdown (void)
{
    /*Con_Printf("Sound shutdown\n");*/

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
    /*char buf[64];
    sprintf(buf, "sound/%s", sfx->name);
    
    sfx_t *tmp = S_FindcacheSound(buf);
    if(tmp)
        goto done;

    tmp = S_CacheSound(sfx->name);

done:
    sfxcache.data = tmp->cache.data;
    sfxcache.length = tmp->cache_data_size;

    return &sfxcache;*/
	return NULL;
}

void S_StartSound (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol,  float attenuation)
{
    /*if(!sfx)
        return;

    sfxcache_t *sfxcache = S_LoadSound(sfx);
	
	// PLAY
	*/
}

void S_StopSound (int entnum, int entchannel)
{
}

static sfx_t *S_FindcacheSound (char *sample)
{
    /*for(int i = 0; i < precache_sound_idx; i++){
        if(!strcmp(precache_sound[i]->name, sample))
            return precache_sound[i];
    }*/

    return NULL;
}

extern int len_for_emu;
static sfx_t *S_CacheSound(char *sample)
{ 
    /*sfx_t *sfx;
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

    return sfx;*/
	return NULL;
}

sfx_t *S_PrecacheSound (char *sample)
{
    /*char buf[64];
    sprintf(buf, "sound/%s", sample);

    sfx_t *sfx = S_FindcacheSound(buf);
    if(sfx)
        return sfx;
    
    return S_CacheSound(buf);*/
	return NULL;
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
    /*sfx_t *sfx = S_PrecacheSound(s);
    if(!sfx){
        Con_Printf("S_LocalSound: cannot cache %s\n", s);
        return;
    }

    S_StartSound(-1, 0, sfx, vec3_origin, 1, 1);*/
}

