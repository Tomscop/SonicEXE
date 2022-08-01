#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{XA_Menu, XA_LENGTH(6119)}, //XA_GettinFreaky
	{XA_Menu, XA_LENGTH(1918)},  //XA_GameOver
	//WEEK1A.XA
	{XA_Week1A, XA_LENGTH(18830)}, //XA_Bopeebo
	{XA_Week1A, XA_LENGTH(18216)}, //XA_Fresh
	//WEEK1B.XA
	{XA_Week1B, XA_LENGTH(8700)}, //XA_Dadbattle
	{XA_Week1B, XA_LENGTH(6800)}, //XA_Tutorial
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\WEEK1A.XA;1", //XA_Week1A
	"\\MUSIC\\WEEK1B.XA;1", //XA_Week1B
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
	{"bopeebo", true}, //XA_Bopeebo
	{"fresh", true},   //XA_Fresh
	//WEEK1B.XA
	{"dadbattle", true}, //XA_Dadbattle
	{"tutorial", false}, //XA_Tutorial
	
	{NULL, false}
};
