#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{FL_Menu, XA_LENGTH(6119)}, //XA_GettinFreaky
	{FL_Menu, XA_LENGTH(1918)},  //XA_GameOver
	//WEEK1A.XA
	{FL_TooSlow, XA_LENGTH(18830)}, //XA_Bopeebo
	{FL_TooSlow, XA_LENGTH(18216)}, //XA_Fresh
	//WEEK1B.XA
	{FL_YouCantRun, XA_LENGTH(15943)}, //XA_Dadbattle
	{FL_YouCantRun, XA_LENGTH(20900)}, //XA_Tutorial
	//WEEK1A.XA
	{FL_TT_FE, XA_LENGTH(50710)}, //XA_Bopeebo
	{FL_TT_FE, XA_LENGTH(22400)}, //XA_Fresh
	//WEEK1B.XA
	{FL_Cycles, XA_LENGTH(15231)}, //XA_Dadbattle
	{FL_Cycles, XA_LENGTH(15540)}, //XA_Tutorial
	//WEEK1B.XA
	{FL_FOF, XA_LENGTH(22000)}, //XA_Dadbattle
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\TOOSLOW.XA;1", //XA_Week1A
	"\\MUSIC\\YCR.XA;1", //XA_Week1B
	"\\MUSIC\\TT-FE.XA;1", //XA_Week1B
	"\\MUSIC\\CYCLES.XA;1", //XA_Week1B
	"\\MUSIC\\FOF.XA;1", //XA_Week1B
	NULL,
};

typedef struct
{
	const char *name;
	boolean vocal;
} XA_Mp3;

static const XA_Mp3 xa_mp3s[] = {
	//MENU.XA
	{"freakymenu", false},   //XA_GettinFreaky
	{"gameover", false}, //XA_GameOver
	//WEEK1A.XA
	{"tooslow", true},
	{"tooslowencore", true},
	//WEEK1B.XA
	{"youcantrun", true},
	{"youcantrunencore", true},
	
	{"tripletrouble", true},
	{"finalescape", true},
	
	{"cycles", true},
	{"cyclesencore", true},
	
	{NULL, false}
};
