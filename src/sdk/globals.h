struct CGlobalVars
{
	float	real_time;
	int		frame_cout;
	float	absolute_frame_time;
#ifdef _CSGO
	float absoluteframestarttimesdev;
#endif
	float	curtime;
	float	frame_time;
	int		max_clients;
	int		tick_count;
	float	interval_per_tick;
	float	interpolation_amount;
	int		sim_ticks_this_frame;
	int		network_protocol;
};

extern CGlobalVars *g_pGlobals;
