	#include "../stdafx.h"

#define MAX_PLAYER_NAME_LENGTH 32

#ifdef _CSGO
#undef MAX_PLAYER_NAME_LENGTH
#define MAX_PLAYER_NAME_LENGTH 128
#endif

typedef struct player_info_s
{
#ifdef _CSGO
	unsigned __int64 unknown;
	// network xuid
	unsigned __int64 xuid;
#endif
	char			name[MAX_PLAYER_NAME_LENGTH];
	int				userID;
	char			guid[32 + 1];
	unsigned int	friendsID;
	char			friendsName[MAX_PLAYER_NAME_LENGTH];
	bool			fakeplayer;
	bool			ishltv;
	unsigned long	customFiles[4];
	unsigned char	filesDownloaded;
} player_info;