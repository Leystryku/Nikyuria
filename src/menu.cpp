#pragma once

#include "menu.h"
#include "menu_panels\elabel.h" // done
#include "menu_panels\epanel.h" // done
#include "menu_panels\eframe.h" // done
#include "menu_panels\ebutton.h" // done
#include "menu_panels\echeckbox.h" // done
#include "menu_panels\eslider.h" // done
#include "menu_panels\etextbox.h"
//todo: textbox, combobox

m_menu *menu;

element::element(element*dad)
{
	if (!menu)
		return;

	color.SetColor(39, 39, 39, 255);
	bordercolor.SetColor(255, 255, 255, 255);
	textcol.SetColor(255, 255, 255, 255);

	bordersize = 1;

	x, y, w, h = 1;
	parent = dad;

	name = "";

	visible = true;
	
	menu->elements[menu->num_elements++] = this;
	x_padding = 0;
	y_padding = 0;
	font = 0;

};

void element::Paint()
{
	int posx, posy;
	GetPos(posx, posy, true);

	d3d->DrawOutlinedRectangle(posx, posy, w, h, bordersize, D3DCOLOR_RGBA(color.r(), color.g(), color.b(), color.a()), D3DCOLOR_RGBA(bordercolor.r(), bordercolor.g(), bordercolor.b(), bordercolor.a()));

	PaintOver();

}

void element::PaintOver()
{

}

void element::SetPos(int to_x, int to_y)
{

	x = to_x;
	y = to_y;
}

void element::SetPadding(int to_x, int to_y)
{
	x_padding = to_x;
	y_padding = to_y;
}

void element::GetPadding(int &f_x, int &f_y)
{
	f_x = x_padding;
	f_y = y_padding;
}

void element::SetSize(int new_w, int new_h)
{
	if (parent)
	{

		if (new_h > parent->h - 20)
			new_h = parent->h - 20;

	}

	w = new_w;
	h = new_h;


}

void element::GetPos(int &ix, int &iy, bool breal)
{

	if (breal&&parent)
	{
		int parentx, parenty;
		parent->GetPos(parentx, parenty, true);

		if (!parentx || !parenty)
			return GetPos(x, y, false);

		int truex = parentx + parent->x_padding + x;
		int truey = parenty + parent->y_padding + y;
		


		ix = truex;
		iy = truey;
		return;
	}

	ix = x;
	iy = y;



}

void element::GetSize(int &iw, int &ih)
{
	iw = w;
	ih = h;
}

void element::Center()
{

	int sizew=0, sizeh=0;
	GetSize(sizew, sizeh);

	if (parent)
	{
		int parentsizew, parentsizeh;
		parent->GetSize(parentsizew, parentsizeh);

		int x = (parentsizew / 2) - (sizew / 2);
		int y = (parentsizeh / 2) - (sizeh / 2);

		SetPos(x, y);

		return;
	}
	
	
	
	
	int x = (menu->ScrW / 2) - (sizew / 2);
	int y = (menu->ScrH / 2) - (sizeh / 2);

	SetPos(x, y);
}

void element::SetName(std::string newname)
{
	name = newname;
}

void element::SetFont(void* newfont)
{
	font = newfont;
}

void* element::GetFont()
{
	if (font)
		return font;

	return d3d->font2;
}

std::string element::GetName()
{
	return name;
}

void element::SetColor(Color to_col)
{
	color = to_col;
}

void element::SetBorderColor(Color to_col)
{
	bordercolor = to_col;
}

void element::SetBorderSize(int to_size)
{
	bordersize = to_size;
}

Color element::GetColor()
{
	return color;
}

Color element::GetBorderColor()
{
	return bordercolor;
}

Color element::GetTextColor()
{
	return textcol;
}

int element::GetBorderSize()
{
	return bordersize;
}

void element::SetVisible(bool visibility)
{
	visible = visibility;
}

bool element::GetVisible()
{

	if (parent && !parent->GetVisible())
		return false;

	return visible;
}

bool element::GetToggleKeyState(int vk_key)
{
	return utils::keys::GetToggleKeyState(vk_key);
}

char curChar;
char element::GetCurChar()
{
	char ret = curChar;
	curChar = 0;
	return ret;
}

element *element::GetParent()
{
	return parent;
}
void element::SetParent(element *toparent)
{
	parent = toparent;
}


int element::MouseOver(int x1, int y1, int x2, int y2)
{

	

	int xx, yy;
	GetPos(xx, yy, true);

	if (GetParent())
		yy = yy;

	int xx2 = w;
	int yy2 = h;

	if (x1 && y1 && x2 && y2)
	{
		xx = x1;
		yy = y1;
		xx2 = x2;
		yy2 = y2;
	}

	POINT Cur;
	GetCursorPos(&Cur);

	if (Cur.x > xx && Cur.x < xx+xx2 && Cur.y > yy && Cur.y < yy+yy2)
	{

		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
			return 2;

		if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
			return 3;

		return 1;
	}
	return 0;
}
void element::Think()
{

}

m_menu::m_menu()
{
	num_elements = 0;

	for (int i = 1; i < 666; i++)
		elements[i] = NULL;

	is_open = true;

	//hack stuff
	bot_aim = false;
	bot_trigger = false;
	bot_aim_silent = false;
	bot_aim_silent_p = false;
	bot_teamkill = false;
	bot_killplayer = false;
	bot_killnpc = false;
	bot_teamshoot = false;
	bot_canshoot = false;
	bot_delayselection = false;

	bot_aim_smoothing = 0;
	bot_aim_fov = 0.f;
	bot_aim_dist = 0;
	bot_aim_targetmode = 0;
	bot_aim_prediction = 0;
	bot_hitgroup = 0;
	bot_spawnprotection = 0;

	wep_autopistol = false;
	wep_autopistol_alt = false;
	wep_autoreload = false;
	wep_nospread = false;
	wep_nospread_seed = false;
	wep_norecoil = false;
	wep_norecoil_punch = false;
	wep_noshake = false;

	esp_hitbox = false;
	esp_name = false;
	esp_health = false;
	esp_players = false;
	esp_npcs = false;
	esp_weapons = false;
	esp_view = false;

	misc_safemode = false;
	misc_namechange = false;
	misc_invisname = false;
	misc_bhop = false;
	misc_autostrafe = false;
	misc_freeze = false;
	misc_antiaim = false;
	misc_antiaimproxy = false;
	misc_flashlightspam = false;
	misc_fakecrouch = false;
	misc_spechack = false;
	misc_text_team = false;
	misc_text_imitator = false;
	misc_voice_imitator = false;
	misc_text_spam = false;
	misc_text_jam = false;


	misc_speedhack = 0;
	misc_fakelag = 0;
	misc_fakelag2 = 0;

	esp_customesp = "";
	misc_text_spam_txt = "";

#ifdef _GMOD
	misc_text_ulxspam = false;
	misc_namechange_darkrp = false;
	
	misc_block_net = 0;

	misc_text_lua = "";
#endif

}


#ifdef _GMOD



extern const char* currentCode;

void DoLuaRun(element*e)
{



	const char *code = menu->misc_text_lua.c_str();

	Msg("Running: %s\n", code);

	currentCode = code;





}

#endif
int dolel = 0;
char *g_pMicInputFileData = 0;
int g_nMicInputFileBytes = 0;
int g_CurMicInputFileByte = 0;
double g_MicStartTime;

void DoLel(element*e)
{
	if (dolel)
	{
		Msg("Done lel!\n");
		dolel = 0;
		g_pMicInputFileData = 0;
		g_nMicInputFileBytes = 0;
		g_CurMicInputFileByte = 0;
		g_MicStartTime;

	}else{
		Msg("Doing lel...\n");
		dolel = 1;
	}
		


}

EPanel *Bots;
EPanel *Weapon;
EPanel *ESP;
EPanel *Misc;
EPanel *Settings;
void switch_tab1(element*e)
{
	Bots->SetVisible(true);
	Weapon->SetVisible(false);
	ESP->SetVisible(false);
	Misc->SetVisible(false);
	Settings->SetVisible(false);
}

void switch_tab2(element*e)
{
	Bots->SetVisible(false);
	Weapon->SetVisible(true);
	ESP->SetVisible(false);
	Misc->SetVisible(false);
	Settings->SetVisible(false);
}
void switch_tab3(element*e)
{
	Bots->SetVisible(false);
	Weapon->SetVisible(false);
	ESP->SetVisible(true);
	Misc->SetVisible(false);
	Settings->SetVisible(false);
}
void switch_tab4(element*e)
{
	Bots->SetVisible(false);
	Weapon->SetVisible(false);
	ESP->SetVisible(false);
	Misc->SetVisible(true);
	Settings->SetVisible(false);
}
void switch_tab5(element*e)
{
	Bots->SetVisible(false);
	Weapon->SetVisible(false);
	ESP->SetVisible(false);
	Misc->SetVisible(false);
	Settings->SetVisible(true);
}

EFrame *MainFrame;
void m_menu::Initialize(int scrw, int scrh)
{
	ScrW = scrw;
	ScrH = scrh;

	is_open = true;

	MainFrame = new EFrame;
	MainFrame->SetSize(405, 370);
	MainFrame->Center();
	MainFrame->SetName("Nikyuria");

	Bots = new EPanel(MainFrame);
	Bots->SetSize(395, 295);
	Bots->SetPos(5, 50);
	Bots->SetName("Aim");

	Weapon = new EPanel(MainFrame);
	Weapon->SetSize(395, 295);
	Weapon->SetPos(5, 50);
	Weapon->SetName("Weapon");

	ESP = new EPanel(MainFrame);
	ESP->SetSize(395, 295);
	ESP->SetPos(5, 50);
	ESP->SetName("ESP");

	Misc = new EPanel(MainFrame);
	Misc->SetSize(395, 295);
	Misc->SetPos(5, 50);
	Misc->SetName("Misc");

	Settings = new EPanel(MainFrame);
	Settings->SetSize(395, 295);
	Settings->SetPos(5, 50);
	Settings->SetName("Settings");

	Weapon->SetVisible(false);
	ESP->SetVisible(false);
	Misc->SetVisible(false);
	Settings->SetVisible(false);


	EButton *SwitchToTab = new EButton(MainFrame);
	SwitchToTab->SetSize(75, 30);
	SwitchToTab->SetPos(5 + (80 * 0), 10);
	SwitchToTab->SetName("Aim");
	//SwitchToTab->SetFont(d3d->font3);
	SwitchToTab->DoLeftClick = &switch_tab1;

	EButton *SwitchToTab2 = new EButton(MainFrame);
	SwitchToTab2->SetSize(75, 30);
	SwitchToTab2->SetPos(5 + (80 * 1), 10);
	SwitchToTab2->SetName("Gun");
	//SwitchToTab2->SetFont(d3d->font3);
	SwitchToTab2->DoLeftClick = &switch_tab2;

	EButton *SwitchToTab3 = new EButton(MainFrame);
	SwitchToTab3->SetSize(75, 30);
	SwitchToTab3->SetPos(5 + (80 * 2), 10);
	SwitchToTab3->SetName("ESP");
	//SwitchToTab3->SetFont(d3d->font3);
	SwitchToTab3->DoLeftClick = &switch_tab3;

	EButton *SwitchToTab4 = new EButton(MainFrame);
	SwitchToTab4->SetSize(75, 30);
	SwitchToTab4->SetPos(5 + (80 * 3), 10);
	SwitchToTab4->SetName("Misc");
	//SwitchToTab4->SetFont(d3d->font3);
	SwitchToTab4->DoLeftClick = &switch_tab4;

	EButton *SwitchToTab5 = new EButton(MainFrame);
	SwitchToTab5->SetSize(75, 30);
	SwitchToTab5->SetPos(5 + (80 * 4), 10);
	SwitchToTab5->SetName("Settings");
	//SwitchToTab5->SetFont(d3d->font3);
	SwitchToTab5->DoLeftClick = &switch_tab5;

	//aim
	//checkboxes
	int y_add = 0;
	int y_base = 27;

	ECheckBox *Aimbot = new ECheckBox(Bots);
	Aimbot->SetSize(15, 15);
	Aimbot->SetPos(15, 3);
	Aimbot->SetName("Aimbot");
	Aimbot->SetToggle(&bot_aim);
	(y_add++);

	ECheckBox *AimTrigger = new ECheckBox(Bots);
	AimTrigger->SetSize(15, 15);
	AimTrigger->SetPos(15, y_base * (y_add++));
	AimTrigger->SetName("Trigger");
	AimTrigger->SetToggle(&bot_trigger);

	ECheckBox *SilentAim = new ECheckBox(Bots);
	SilentAim->SetSize(15, 15);
	SilentAim->SetPos(15, y_base * (y_add++));
	SilentAim->SetName("Silent Aim");
	SilentAim->SetToggle(&bot_aim_silent);

	ECheckBox *SilentAim_p = new ECheckBox(Bots);
	SilentAim_p->SetSize(15, 15);
	SilentAim_p->SetPos(15, y_base * (y_add++));
	SilentAim_p->SetName("PSilent Aim");
	SilentAim_p->SetToggle(&bot_aim_silent_p);

	ECheckBox *Teamkill = new ECheckBox(Bots);
	Teamkill->SetSize(15, 15);
	Teamkill->SetPos(15, y_base * (y_add++));
	Teamkill->SetName("Teamkill");
	Teamkill->SetToggle(&bot_teamkill);

	ECheckBox *KillPlayer = new ECheckBox(Bots);
	KillPlayer->SetSize(15, 15);
	KillPlayer->SetPos(15, y_base * (y_add++));
	KillPlayer->SetName("Kill Player");
	KillPlayer->SetToggle(&bot_killplayer);

	ECheckBox *KillNPC = new ECheckBox(Bots);
	KillNPC->SetSize(15, 15);
	KillNPC->SetPos(15, y_base * (y_add++));
	KillNPC->SetName("Kill NPC");
	KillNPC->SetToggle(&bot_killnpc);

	ECheckBox *TeamShoot = new ECheckBox(Bots);
	TeamShoot->SetSize(15, 15);
	TeamShoot->SetPos(15, y_base * (y_add++));
	TeamShoot->SetName("Teamshoot");
	TeamShoot->SetToggle(&bot_teamshoot);

	ECheckBox *IgnoreCanShoot = new ECheckBox(Bots);
	IgnoreCanShoot->SetSize(15, 15);
	IgnoreCanShoot->SetPos(15, y_base * (y_add++));
	IgnoreCanShoot->SetName("Ignore Shottime");
	IgnoreCanShoot->SetToggle(&bot_canshoot);

	ECheckBox *DelaySelection = new ECheckBox(Bots);
	DelaySelection->SetSize(15, 15);
	DelaySelection->SetPos(15, y_base * (y_add++));
	DelaySelection->SetName("Delay Selection");
	DelaySelection->SetToggle(&bot_delayselection);

	//sliders
	y_add = 0;
	y_base = 37;

	ESlider *AimSmoothing = new ESlider(Bots);
	AimSmoothing->SetSize(200, 15);
	AimSmoothing->SetPos(170, 1);
	AimSmoothing->SetName("Smoothing");
	AimSmoothing->SetSlideMax(44);
	AimSmoothing->SetSlider(&bot_aim_smoothing);

	(y_add++);

	ESlider *AimFOV = new ESlider(Bots);
	AimFOV->SetSize(200, 15);
	AimFOV->SetPos(170, y_base * (y_add++));
	AimFOV->SetName("FOV");
	AimFOV->SetSlideMax(360);
	AimFOV->SetSlider(&bot_aim_fov);

	ESlider *AimHitgroup = new ESlider(Bots);
	AimHitgroup->SetSize(200, 15);
	AimHitgroup->SetPos(170, y_base * (y_add++));
	AimHitgroup->SetName("Hitgroup");
	AimHitgroup->SetSlideMax(8);
	AimHitgroup->SetSlider(&bot_hitgroup);
	AimHitgroup->SetSlideText(0, "Hitscan");
	AimHitgroup->SetSlideText(8, "Random");

	ESlider *AimDist = new ESlider(Bots);
	AimDist->SetSize(200, 15);
	AimDist->SetPos(170, y_base * (y_add++));
	AimDist->SetName("Distance");
	AimDist->SetSlideMax(1024);
	AimDist->SetSlider(&bot_aim_dist);

	ESlider *AimSpawnprotect = new ESlider(Bots);
	AimSpawnprotect->SetSize(200, 15);
	AimSpawnprotect->SetPos(170, y_base * (y_add++));
	AimSpawnprotect->SetName("Spawn Protection");
	AimSpawnprotect->SetSlideMax(4);
	AimSpawnprotect->SetSlider(&bot_spawnprotection);
	AimSpawnprotect->SetSlideText(0, "None");
	AimSpawnprotect->SetSlideText(1, "Color A");
	AimSpawnprotect->SetSlideText(2, "Color B");
	AimSpawnprotect->SetSlideText(3, "Material");
	AimSpawnprotect->SetSlideText(4, "Model");

	ESlider *AimTargetmode = new ESlider(Bots);
	AimTargetmode->SetSize(200, 15);
	AimTargetmode->SetPos(170, y_base * (y_add++));
	AimTargetmode->SetName("Target Mode");
	AimTargetmode->SetSlideMax(3);
	AimTargetmode->SetSlider(&bot_aim_targetmode);
	AimTargetmode->SetSlideText(0, "Distance");
	AimTargetmode->SetSlideText(1, "Health");
	AimTargetmode->SetSlideText(2, "Eye Contact");
	AimTargetmode->SetSlideText(3, "Danger Level");

	ESlider *AimPrediction = new ESlider(Bots);
	AimPrediction->SetSize(200, 15);
	AimPrediction->SetPos(170, y_base * (y_add++));
	AimPrediction->SetName("Prediction");
	AimPrediction->SetSlideMax(2);
	AimPrediction->SetSlider(&bot_aim_prediction);
	AimPrediction->SetSlideText(0, "None");
	AimPrediction->SetSlideText(1, "RunCommand");
	AimPrediction->SetSlideText(2, "Movement");

	// stop aim


	// wep
	y_add = 0;
	y_base = 27;

	ECheckBox *AutoPistol = new ECheckBox(Weapon);
	AutoPistol->SetSize(15, 15);
	AutoPistol->SetPos(15, 3);
	AutoPistol->SetName("Auto Pistol");
	AutoPistol->SetToggle(&wep_autopistol);
	(y_add++);

	ECheckBox *AutoPistol_alt = new ECheckBox(Weapon);
	AutoPistol_alt->SetSize(15, 15);
	AutoPistol_alt->SetPos(15, y_base * (y_add++));
	AutoPistol_alt->SetName("Alt Auto Pistol");
	AutoPistol_alt->SetToggle(&wep_autopistol_alt);

	ECheckBox *AutoReload = new ECheckBox(Weapon);
	AutoReload->SetSize(15, 15);
	AutoReload->SetPos(15, y_base * (y_add++));
	AutoReload->SetName("Auto Reload");
	AutoReload->SetToggle(&wep_autoreload);

	ECheckBox *NoSpread = new ECheckBox(Weapon);
	NoSpread->SetSize(15, 15);
	NoSpread->SetPos(15, y_base * (y_add++));
	NoSpread->SetName("Nospread");
	NoSpread->SetToggle(&wep_nospread);

#if defined(_GENERIC) || defined(_GMOD)
	ECheckBox *NoSpread_Seed = new ECheckBox(Weapon);
	NoSpread_Seed->SetSize(15, 15);
	NoSpread_Seed->SetPos(15, y_base * (y_add++));
	NoSpread_Seed->SetName("Seed-Nospread");
	NoSpread_Seed->SetToggle(&wep_nospread_seed);
#endif

	ECheckBox *NoRecoil = new ECheckBox(Weapon);
	NoRecoil->SetSize(15, 15);
	NoRecoil->SetPos(15, y_base * (y_add++));
	NoRecoil->SetName("No Recoil");
	NoRecoil->SetToggle(&wep_norecoil);

	ECheckBox *NoPunchRecoil = new ECheckBox(Weapon);
	NoPunchRecoil->SetSize(15, 15);
	NoPunchRecoil->SetPos(15, y_base * (y_add++));
	NoPunchRecoil->SetName("No Punch Recoil");
	NoPunchRecoil->SetToggle(&wep_norecoil_punch);

	ECheckBox *NoShake = new ECheckBox(Weapon);
	NoShake->SetSize(15, 15);
	NoShake->SetPos(15, y_base * (y_add++));
	NoShake->SetName("No Shake");
	NoShake->SetToggle(&wep_noshake);

	//stop wep

	//esp
	y_add = 0;
	y_base = 27;

	ECheckBox *HitBoxESP = new ECheckBox(ESP);
	HitBoxESP->SetSize(15, 15);
	HitBoxESP->SetPos(15, 3);
	HitBoxESP->SetName("Hitbox ESP");
	HitBoxESP->SetToggle(&esp_hitbox);
	(y_add++);

	ECheckBox *NameESP = new ECheckBox(ESP);
	NameESP->SetSize(15, 15);
	NameESP->SetPos(15, y_base * (y_add++));
	NameESP->SetName("Name ESP");
	NameESP->SetToggle(&esp_name);

	ECheckBox *HealthESP = new ECheckBox(ESP);
	HealthESP->SetSize(15, 15);
	HealthESP->SetPos(15, y_base * (y_add++));
	HealthESP->SetName("Health ESP");
	HealthESP->SetToggle(&esp_health);

	ECheckBox *PlayerESP = new ECheckBox(ESP);
	PlayerESP->SetSize(15, 15);
	PlayerESP->SetPos(15, y_base * (y_add++));
	PlayerESP->SetName("Player ESP");
	PlayerESP->SetToggle(&esp_players);

	ECheckBox *NPCESP = new ECheckBox(ESP);
	NPCESP->SetSize(15, 15);
	NPCESP->SetPos(15, y_base * (y_add++));
	NPCESP->SetName("NPC ESP");
	NPCESP->SetToggle(&esp_npcs);

	ECheckBox *WeaponESP = new ECheckBox(ESP);
	WeaponESP->SetSize(15, 15);
	WeaponESP->SetPos(15, y_base * (y_add++));
	WeaponESP->SetName("Weapon ESP");
	WeaponESP->SetToggle(&esp_weapons);

	ECheckBox *ViewESP = new ECheckBox(ESP);
	ViewESP->SetSize(15, 15);
	ViewESP->SetPos(15, y_base * (y_add++));
	ViewESP->SetName("View ESP");
	ViewESP->SetToggle(&esp_view);


	//stop esp

	//start misc
	//bools
	y_add = 0;
	y_base = 27;

	ECheckBox *SafeMode = new ECheckBox(Misc);
	SafeMode->SetSize(15, 15);
	SafeMode->SetPos(15, 3);
	SafeMode->SetName("Safe Mode");
	SafeMode->SetToggle(&misc_safemode);
	(y_add++);

	ECheckBox *NameChange = new ECheckBox(Misc);
	NameChange->SetSize(15, 15);
	NameChange->SetPos(15, y_base * (y_add++));
	NameChange->SetName("Name Change");
	NameChange->SetToggle(&misc_namechange);

	ECheckBox *InvisName = new ECheckBox(Misc);
	InvisName->SetSize(15, 15);
	InvisName->SetPos(15, y_base * (y_add++));
	InvisName->SetName("Invisname");
	InvisName->SetToggle(&misc_invisname);

	ECheckBox *Bhop = new ECheckBox(Misc);
	Bhop->SetSize(15, 15);
	Bhop->SetPos(15, y_base * (y_add++));
	Bhop->SetName("Bunnyhop");
	Bhop->SetToggle(&misc_bhop);

	ECheckBox *AutoStrafe = new ECheckBox(Misc);
	AutoStrafe->SetSize(15, 15);
	AutoStrafe->SetPos(15, y_base * (y_add++));
	AutoStrafe->SetName("Auto Strafe");
	AutoStrafe->SetToggle(&misc_autostrafe);

	ECheckBox *Freeze = new ECheckBox(Misc);
	Freeze->SetSize(15, 15);
	Freeze->SetPos(15, y_base * (y_add++));
	Freeze->SetName("Freeze Hack");
	Freeze->SetToggle(&misc_freeze);

	ECheckBox *AntiAim = new ECheckBox(Misc);
	AntiAim->SetSize(15, 15);
	AntiAim->SetPos(15, y_base * (y_add++));
	AntiAim->SetName("Anti Aim");
	AntiAim->SetToggle(&misc_antiaim);

	ECheckBox *AntiAimProxy = new ECheckBox(Misc);
	AntiAimProxy->SetSize(15, 15);
	AntiAimProxy->SetPos(15, y_base * (y_add++));
	AntiAimProxy->SetName("Anti Aim Proxy");
	AntiAimProxy->SetToggle(&misc_antiaimproxy);

	ECheckBox *FlashlightSpam = new ECheckBox(Misc);
	FlashlightSpam->SetSize(15, 15);
	FlashlightSpam->SetPos(15, y_base * (y_add++));
	FlashlightSpam->SetName("Flashspam");
	FlashlightSpam->SetToggle(&misc_flashlightspam);

	ECheckBox *FakeCrouch = new ECheckBox(Misc);
	FakeCrouch->SetSize(15, 15);
	FakeCrouch->SetPos(15, y_base * (y_add++));
	FakeCrouch->SetName("Fakecrouch");
	FakeCrouch->SetToggle(&misc_fakecrouch);

	y_add = 0;
	y_base = 27;

	ECheckBox *Spectate = new ECheckBox(Misc);
	Spectate->SetSize(15, 15);
	Spectate->SetPos(150, 3);
	Spectate->SetName("Spectate");
	Spectate->SetToggle(&misc_spechack);
	(y_add++);

	ECheckBox *TextTeam = new ECheckBox(Misc);
	TextTeam->SetSize(15, 15);
	TextTeam->SetPos(150, y_base * (y_add++));
	TextTeam->SetName("Team Text");
	TextTeam->SetToggle(&misc_text_team);

	ECheckBox *TextImitator = new ECheckBox(Misc);
	TextImitator->SetSize(15, 15);
	TextImitator->SetPos(150, y_base * (y_add++));
	TextImitator->SetName("Text Imitator");
	TextImitator->SetToggle(&misc_text_imitator);

	ECheckBox *VoiceImitator = new ECheckBox(Misc);
	VoiceImitator->SetSize(15, 15);
	VoiceImitator->SetPos(150, y_base * (y_add++));
	VoiceImitator->SetName("Voice Imitator");
	VoiceImitator->SetToggle(&misc_voice_imitator);

	
	ECheckBox *TextSpam = new ECheckBox(Misc);
	TextSpam->SetSize(15, 15);
	TextSpam->SetPos(150, y_base * (y_add++));
	TextSpam->SetName("Text Spammer");
	TextSpam->SetToggle(&misc_text_spam);

	ECheckBox *TextJam = new ECheckBox(Misc);
	TextJam->SetSize(15, 15);
	TextJam->SetPos(150, y_base * (y_add++));
	TextJam->SetName("Text Jammer");
	TextJam->SetToggle(&misc_text_jam);

#ifdef _GMOD


	ECheckBox *DarkrpName = new ECheckBox(Misc);
	DarkrpName->SetSize(15, 15);
	DarkrpName->SetPos(150, y_base * (y_add++));
	DarkrpName->SetName("[GM] DRP Name");
	DarkrpName->SetToggle(&misc_namechange_darkrp);

	ECheckBox *ULXSpam = new ECheckBox(Misc);
	ULXSpam->SetSize(15, 15);
	ULXSpam->SetPos(150, y_base * (y_add++));
	ULXSpam->SetName("[GM] ULXSpam");
	ULXSpam->SetToggle(&misc_text_ulxspam);

	EButton *LelButt = new EButton(Misc);
	LelButt->SetSize(100, 25);
	LelButt->SetPos(150, 233);
	LelButt->SetName("Lel");
	LelButt->DoLeftClick = &DoLel;

	EButton *LuaRunBut = new EButton(Misc);
	LuaRunBut->SetSize(100, 25);
	LuaRunBut->SetPos(150, 260);
	LuaRunBut->SetName("Run Lua");
	LuaRunBut->DoLeftClick = &DoLuaRun;

#endif
	//sliders

	ESlider *Speedhack = new ESlider(Misc);
	Speedhack->SetSize(100, 15);
	Speedhack->SetPos(280, 190);
	Speedhack->SetName("Speedhack");
	Speedhack->SetSlideMax(12);
	Speedhack->SetSlider(&misc_speedhack);

	ESlider *FakeLag = new ESlider(Misc);
	FakeLag->SetSize(100, 15);
	FakeLag->SetPos(280, 240);
	FakeLag->SetName("Fake Lag F");
	FakeLag->SetSlideMax(300);
	FakeLag->SetSlider(&misc_fakelag2);

	ESlider *FakeLagF = new ESlider(Misc);

	FakeLagF->SetSize(100, 30);
	FakeLagF->SetPos(280, 15);
	FakeLagF->SetName("Fake Lag");
	FakeLagF->SetSlideMax(300);
	FakeLagF->SetSlider(&misc_fakelag);
	//textboxes

#ifdef _GMOD

	ESlider *BlockNet = new ESlider(Misc);
	BlockNet->SetSize(100, 30);
	BlockNet->SetPos(280, 45);
	BlockNet->SetName("[GM] Nets");
	BlockNet->SetSlider(&misc_block_net);
	BlockNet->SetSlideMax(4);
	BlockNet->SetSlideText(0, "Off");
	BlockNet->SetSlideText(1, "Out");
	BlockNet->SetSlideText(2, "Rec");
	BlockNet->SetSlideText(3, "All");
	BlockNet->SetSlideText(4, "Log");

	ETextBox *LuaRun = new ETextBox(Misc);
	LuaRun->SetSize(100, 30);
	LuaRun->SetPos(280, 90);
	LuaRun->SetName("To Run");
	LuaRun->SetTextBuf(&misc_text_lua);
#endif

	ETextBox *TextSpam_txt = new ETextBox(Misc);
	TextSpam_txt->SetSize(100, 30);
	TextSpam_txt->SetPos(280, 150);
	TextSpam_txt->SetName("Text spammenz");
	TextSpam_txt->SetTextBuf(&misc_text_spam_txt);

	//end misc

	//start settings

	//end settings
}

void m_menu::Resize(int to_scrw, int to_scrh)
{
	menu->ScrW = to_scrw;
	menu->ScrH = to_scrh;
	MainFrame->Center();
}
