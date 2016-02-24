#pragma once

#include "stdafx.h"
#include "d3d.h"
#include "sdk/color.h"

class element
{
private:

	bool visible;
	int x,y,w,h, x_padding, y_padding;
	element *parent;
	std::string name;
	
	Color color;
	Color bordercolor;
	Color textcol;
	int bordersize;
	void* font;

public:

	element(element* parent);
	element() : element(NULL) { }

	virtual void SetName(std::string);
	virtual void SetFont(void* newfont);
	virtual void SetPos(int x, int y);
	virtual void SetSize(int w, int h);
	virtual void SetVisible(bool visibility);
	virtual void GetSize(int &w, int &h);
	virtual void GetPos(int &x, int &y, bool breal);
	virtual void* GetFont();
	virtual bool GetVisible();
	virtual char GetCurChar();
	virtual bool GetToggleKeyState(int vk_key);
	virtual std::string GetName();
	virtual Color GetColor();
	virtual Color GetTextColor();
	virtual Color GetBorderColor();
	virtual int GetBorderSize();
	virtual void SetColor(Color tocolor);
	virtual void SetBorderColor(Color tocolor);
	virtual void SetBorderSize(int tosize);
	virtual void Center();
	virtual void SetParent(element* newparent);
	virtual void SetPadding(int x_padding, int y_padding);
	virtual void GetPadding(int &f_x, int &f_y);
	virtual element *GetParent();

	virtual void Paint();

	virtual void PaintOver();

	virtual int MouseOver(int x1,int y1, int x2, int y2);

	virtual void Think();

};



class m_menu
{
public:

	m_menu();

	void Initialize( int scrw, int scrh );
	void Resize(int to_scrw, int to_scrh);

	void *elements[666];
	int num_elements;
	int ScrW;
	int ScrH;
	
	bool is_open;

	POINT Cur;
	//hack stuff

	//settings

	bool bot_aim;
	bool bot_trigger;
	bool bot_aim_silent;
	bool bot_aim_silent_p;
	bool bot_teamkill;
	bool bot_killplayer;
	bool bot_killnpc;
	bool bot_hitscan;
	bool bot_teamshoot;
	bool bot_canshoot;
	bool bot_delayselection;

	int bot_aim_smoothing;
	float bot_aim_fov;
	int bot_aim_dist;
	int bot_aim_targetmode;
	int bot_aim_prediction;
	int bot_hitgroup;
	int bot_spawnprotection;

	bool wep_autopistol;
	bool wep_autopistol_alt;
	bool wep_autoreload;
	bool wep_nospread;
	bool wep_nospread_seed;
	bool wep_norecoil;
	bool wep_norecoil_punch;
	bool wep_noshake;

	bool esp_hitbox;
	bool esp_name;
	bool esp_health;
	bool esp_players;
	bool esp_npcs;
	bool esp_weapons;
	bool esp_view;

	
	bool misc_safemode;
	bool misc_namechange;

	bool misc_invisname;
	bool misc_bhop;
	bool misc_autostrafe;
	bool misc_freeze;
	bool misc_antiaim;
	bool misc_antiaimproxy;
	bool misc_flashlightspam;
	bool misc_fakecrouch;
	bool misc_spechack;
	bool misc_text_team;
	bool misc_text_imitator;
	bool misc_text_spam;
	bool misc_text_jam;
	bool misc_voice_imitator;

	int misc_speedhack;
	int misc_fakelag;
	int misc_fakelag2;

	std::string esp_customesp;
	std::string misc_text_spam_txt;

#ifdef _GMOD

	bool misc_text_ulxspam;
	bool misc_namechange_darkrp;

	int misc_block_net;

	std::string misc_text_lua;
#endif


};

extern m_menu *menu;
