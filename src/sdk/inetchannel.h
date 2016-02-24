#pragma once

#include "externfuncs.h"
#include "interface.h"
#include "buf.h"

// ======================= INDEXES =========================== //

#define NET_MESSAGE_BITS 6
#define NETMSG_TYPE_BITS	5

// ======================= CLASS HEADER ======================= //

#include "../utils.h"

class INetMsgHandler;
class INetMessage;
class INetChannel;

class INetChannelInfo
{
public:

	enum {
		GENERIC = 0,	// must be first and is default group
		LOCALPLAYER,	// bytes for local player entity update
		OTHERPLAYERS,	// bytes for other players update
		ENTITIES,		// all other entity bytes
		SOUNDS,			// game sounds
		EVENTS,			// event messages
		USERMESSAGES,	// user messages
		ENTMESSAGES,	// entity messages
		VOICE,			// voice data
		STRINGTABLE,	// a stringtable update
		MOVE,			// client move cmds
		STRINGCMD,		// string command
		SIGNON,			// various signondata
		TOTAL,			// must be last and is not a real group
	};
	//only need that shit
};

class INetMessage
{
public:
	virtual	~INetMessage() {};

	// Use these to setup who can hear whose voice.
	// Pass in client indices (which are their ent indices - 1).

	virtual void	SetNetChannel(INetChannel * netchan) = 0; // netchannel this message is from/for
	virtual void	SetReliable(bool state) = 0;	// set to true if it's a reliable message

	virtual bool	Process(void) = 0; // calles the recently set handler to process this message

	virtual	bool	ReadFromBuffer(bf_read &buffer) = 0; // returns true if parsing was OK
	virtual	bool	WriteToBuffer(bf_write &buffer) = 0;	// returns true if writing was OK

	virtual bool	IsReliable(void) const = 0;  // true, if message needs reliable handling

	virtual int				GetType(void) const = 0; // returns module specific header tag eg svc_serverinfo
	virtual int				GetGroup(void) const = 0;	// returns net message group of this message
	virtual const char		*GetName(void) const = 0;	// returns network message name, eg "svc_serverinfo"
	virtual INetChannel		*GetNetChannel(void) const = 0;
	virtual const char		*ToString(void) const = 0; // returns a human readable string about message content
};

#define MAX_USER_MSG_DATA 255

class netmsghandler;

#define DECLARE_NET_MESSAGE( name )						\
	public:													\
		bool			ReadFromBuffer( bf_read &buffer ) { return true; };	\
		bool			WriteToBuffer( bf_write &buffer ) { return true; };	\
		const char		*ToString() const {return #name;};					\
		int				GetType() const { return name; } \
		const char		*GetName() const { return #name;}\
		netmsghandler *m_pMessageHandler;	\
		bool Process() { return true; }\


#define DECLARE_VNET_MESSAGE( name )						\
	public:													\
		bool			ReadFromBuffer( bf_read &buffer ) { return true; };	\
		bool			WriteToBuffer( bf_write &buffer ) { buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS); m_nLength = m_DataOut.GetNumBitsWritten(); buffer.WriteWord(m_nLength); return buffer.WriteBits(m_DataOut.GetBasePointer(), m_nLength); };	\
		const char		*ToString() const {	sprintf_s(s_text, "%s: %i bytes", GetName(), Bits2Bytes(m_nLength)); return s_text; };					\
		int				GetType() const { return name; } \
		const char		*GetName() const { return #name;}\
		netmsghandler *m_pMessageHandler;	\
		bool Process() { return true; }\

class CNetMessage : public INetMessage
{
public:
	CNetMessage() {
		m_bReliable = true;
		m_NetChannel = NULL;
	}

	virtual ~CNetMessage() {};

	virtual int		GetGroup() const { return INetChannelInfo::GENERIC; }
	INetChannel		*GetNetChannel() const { return m_NetChannel; }

	virtual void	SetReliable(bool state) { m_bReliable = state; };
	virtual bool	IsReliable() const { return m_bReliable; };
	virtual void    SetNetChannel(INetChannel * netchan) { m_NetChannel = netchan; }
	virtual bool	Process() {  return false; };	// no handler set

protected:
	bool				m_bReliable;	// true if message should be send reliable
	INetChannel			*m_NetChannel;	// netchannel this message is from/for
};

#define Bits2Bytes(b) ((b+7)>>3)

#define clc_VoiceData 10

#define svc_VoiceData 15
#define svc_UserMessage 23
#define	svc_SetPause		11

static char s_text[1024];

class SVC_SetPause : public CNetMessage
{
	DECLARE_NET_MESSAGE(svc_SetPause);

	SVC_SetPause() {}
	SVC_SetPause(bool state) { m_bPaused = state; }

public:
	bool		m_bPaused;		// true or false, what else
};

class CLC_VoiceData : public CNetMessage
{
	DECLARE_VNET_MESSAGE(clc_VoiceData);

	CLC_VoiceData() { m_bReliable = false; };

	int	GetGroup() const { return INetChannelInfo::VOICE; }

public:
	int				m_nLength;
	bf_read			m_DataIn;
	bf_write		m_DataOut;
	uint64			m_xuid;
};

#define net_SignonState 16

class NET_SignonState : public CNetMessage
{
	DECLARE_NET_MESSAGE(net_SignonState);

	NET_SignonState() { m_bReliable = false; }

	int	GetGroup() const { return INetChannelInfo::SIGNON; }
	
	NET_SignonState(int tits) {};
	NET_SignonState(int state, int spawncount) { m_nSignonState = state; m_nSpawnCount = spawncount; };

public:
	int			m_nSignonState;			// See SIGNONSTATE_ defines
	int			m_nSpawnCount;			// server spawn count (session number)
};

#define svc_VoiceInit		14	

class SVC_VoiceInit : public CNetMessage
{
	DECLARE_NET_MESSAGE(svc_VoiceInit);

	int	GetGroup() const { return INetChannelInfo::SIGNON; }


	SVC_VoiceInit() {}
	SVC_VoiceInit(const char * codec, int quality) { m_szVoiceCodec = codec; m_nQuality = quality; }

public:
	const char 	*m_szVoiceCodec;	// used voice codec .DLL
	int			m_nQuality;	// custom quality seeting

public:
	char 		m_szVoiceCodecBuffer[260];	// used voice codec .DLL
};


class SVC_VoiceData : public CNetMessage
{
	DECLARE_NET_MESSAGE(svc_VoiceData);

	SVC_VoiceData() { m_bReliable = false; }

	int	GetGroup() const { return INetChannelInfo::VOICE; }

public:
	int				m_nFromClient;	// client who has spoken
	bool			m_bProximity;
	int				m_nLength;		// data length in bits
	uint64			m_xuid;			// X360 player ID

	bf_read			m_DataIn;
	void			*m_DataOut;
};

class SVC_UserMessage : public CNetMessage
{
	DECLARE_NET_MESSAGE(svc_UserMessage);

	SVC_UserMessage() { m_bReliable = false; }

	int	GetGroup() const { return INetChannelInfo::USERMESSAGES; }

public:
	int			m_nMsgType;
	int			m_nLength;	// data length in bits
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

#define KillNetMessage(thename) char aba[1024]; thename->SetReliable(false); bf_read tmap(aba, sizeof(aba)); thename->ReadFromBuffer(tmap);

typedef struct netpacket_s netpacket_t;

#ifndef NET_PACKET_ST_DEFINED
typedef enum
{
	NA_NULL = 0,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
} netadrtype_t;
typedef struct netadr_s
{
public:
	netadr_s() { SetIP(0); SetPort(0); SetType(NA_IP); }
	netadr_s(int unIP) { }
	netadr_s(const char *pch) { SetFromString(pch); }
	void	Clear();	// invalids Address

	void	SetType(netadrtype_t type);
	void	SetPort(unsigned short port);
	bool	SetFromSockadr(const struct sockaddr *s);
	void	SetIP(uint8 b1, uint8 b2, uint8 b3, uint8 b4);
	void	SetIP(int unIP);									// Sets IP.  unIP is in host order (little-endian)
	void    SetIPAndPort(int unIP, unsigned short usPort) { SetIP(unIP); SetPort(usPort); }
	void	SetFromString(const char *pch, bool bUseDNS = false); // if bUseDNS is true then do a DNS lookup if needed

	bool	CompareAdr(const netadr_s &a, bool onlyBase = false) const;
	bool	CompareClassBAdr(const netadr_s &a) const;
	bool	CompareClassCAdr(const netadr_s &a) const;

	netadrtype_t	GetType() const;
	unsigned short	GetPort() const;
	const char*		ToString(bool onlyBase = false) const; // returns xxx.xxx.xxx.xxx:ppppp
	void			ToSockadr(struct sockaddr *s) const;
	unsigned int	GetIP() const;

	bool	IsLocalhost() const; // true, if this is the localhost IP 
	bool	IsLoopback() const;	// true if engine loopback buffers are used
	bool	IsReservedAdr() const; // true, if this is a private LAN IP
	bool	IsValid() const;	// ip & port != 0
	bool	IsBaseAdrValid() const;	// ip != 0

	void    SetFromSocket(int hSocket);

	// These function names are decorated because the Xbox360 defines macros for ntohl and htonl
	unsigned long addr_ntohl() const;
	unsigned long addr_htonl() const;
	bool operator==(const netadr_s &netadr) const { return (CompareAdr(netadr)); }
	bool operator<(const netadr_s &netadr) const;

public:	// members are public to avoid to much changes

	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned short	port;
} netadr_t;
#define NET_PACKET_ST_DEFINED
typedef struct netpacket_s
{
	netadr_t		from;		// sender IP
	int				source;		// received source 
	double			received;	// received time
	unsigned char	*data;		// pointer to raw packet data
	bf_read			message;	// easy bitbuf data access
	int				size;		// size in bytes
	int				wiresize;   // size in bytes before decompression
	bool			stream;		// was send as stream
	struct netpacket_s *pNext;	// for internal use, should be NULL in public
} netpacket_t;
#endif // NET_PACKET_ST_DEFINED

SETUP_INTERFACE3(INetChannel)
	
/*
	void SendFile(const char *filename, unsigned int transferid = 90)
	{
		static auto OriginalFn = (void(__thiscall *)(void *, const char*filename, unsigned int transferid))utils::memory::FindSubStart(utils::memory::FindString(GetModuleHandleA("engine.dll"), "SendFile: %s(ID %i)\n"));
		return OriginalFn(this, filename, transferid);
	}
	*/
	SetupInterfaceFunc2(SendFile, void, 42, (const char *filename, unsigned int transferid), (void* __this, const char*filename, unsigned int transferid), (this, filename, transferid));
	SetupInterfaceFunc2(RequestFile, unsigned int, 62, (const char *filename), (void* __this, const char*filename), (this, filename));


	void SendData(bf_write &msg, bool bReliable = true)
	{
		static auto OriginalFn = (void(__thiscall *)(void *, bf_write &msg,  bool bReliable))utils::memory::FindSubStart(utils::memory::FindString(GetModuleHandleA("engine.dll"), "ERROR! SendData reliabe data too big (%i)"));
		return OriginalFn(this, msg, bReliable);
	}

	int SendNetMsg(INetMessage &msg, bool bForceReliable, bool bVoice)
	{
		static auto OriginalFn = (int(__thiscall *)(void *, INetMessage &msg, bool bForceReliable, bool bVoice))utils::memory::FindSubStart(utils::memory::FindString(GetModuleHandleA("engine.dll"), "NetMsg"));
		return OriginalFn(this, msg, bForceReliable, bVoice);
	}


	

	void StringCommand(const char* cmd, bool breliable = false)
	{
		char pckBuf[5024];
		bf_write pck(pckBuf, sizeof(pckBuf));


		pck.WriteUBitLong(4, NET_MESSAGE_BITS);
		pck.WriteString(cmd);

		this->SendData(pck, breliable);
	}

	void Disconnect(const char* reason, bool breliable=false)
	{
		char pckBuf[5024];
		bf_write pck(pckBuf, sizeof(pckBuf));


		pck.WriteUBitLong(1, NET_MESSAGE_BITS);
		pck.WriteString(reason);

		this->SendData(pck, false);
	}
	void SetName(const char*name)
	{
		char pckBuf[5024];
		bf_write pck(pckBuf, sizeof(pckBuf));

		pck.WriteUBitLong(5, NET_MESSAGE_BITS);
		pck.WriteByte(1);
		pck.WriteString("name");
		pck.WriteString(name);

		this->SendData(pck, false);

	}

	void DoLel(int type, int cnt = -1)
	{
		char pckBuf[1024];
		bf_write pck(pckBuf, sizeof(pckBuf));
		pck.WriteUBitLong(6, NET_MESSAGE_BITS);
		pck.WriteByte(type);
		pck.WriteLong(cnt);

		this->SendData(pck);
	}


};