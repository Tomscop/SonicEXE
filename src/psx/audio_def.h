#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{FL_Menu, XA_LENGTH(6119)}, //XA_GettinFreaky
	{FL_Menu, XA_LENGTH(1920)},  //XA_GameOver
	{FL_Menu, XA_LENGTH(7540)}, //XA_GettinFreaky
	//TOOSLOW.XA
	{FL_TooSlow, XA_LENGTH(18830)}, //XA_TooSlow
	{FL_TooSlow, XA_LENGTH(18216)}, //XA_TooSlowEncore
	//YCR.XA
	{FL_YouCantRun, XA_LENGTH(15942)}, //XA_YouCantRun
	//TRIPLETROUB.XA
	{FL_TripleTrouble, XA_LENGTH(50710)}, //XA_TripleTrouble
	//ENDLESS.XA
	{FL_Endless, XA_LENGTH(16954)}, //XA_Endless
	//ENDLESSOG.XA
	{FL_EndlessOG, XA_LENGTH(17032)}, //XA_EndlessOG
	//CYC-EXEC.XA
	{FL_Cyc_Exec, XA_LENGTH(15230)}, //XA_Cycles
	{FL_Cyc_Exec, XA_LENGTH(12158)}, //XA_Execution
	//SUN-SOUL.XA
	{FL_Sun_Soul, XA_LENGTH(18342)}, //XA_Sunshine
	{FL_Sun_Soul, XA_LENGTH(19800)}, //XA_Soulless
	//CHAOS.XA
	{FL_Chaos, XA_LENGTH(15751)}, //XA_Chaos
	//PRY-FOF.XA
	{FL_Pry_FOF, XA_LENGTH(29045)}, //XA_Prey
	{FL_Pry_FOF, XA_LENGTH(21990)}, //XA_Fight_Or_Flight
	//RAB.XA
	{FL_RAB, XA_LENGTH(16401)}, //XA_Round_A_Bout
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //FL_Menu
	
	"\\MUSIC\\TOOSLOW.XA;1", //FL_TooSlow
	"\\MUSIC\\YCR.XA;1", //FL_YouCantRun
	"\\MUSIC\\TRIPLETRO.XA;1", //FL_TripleTrouble
	
	"\\MUSIC\\ENDLESS.XA;1", //FL_Endless
	"\\MUSIC\\ENDLESSOG.XA;1", //FL_EndlessOG
	
	"\\MUSIC\\CYC-EXEC.XA;1", //FL_Cyc_Exec
	
	"\\MUSIC\\SUN-SOUL.XA;1", //FL_Sun_Soul
	
	"\\MUSIC\\CHAOS.XA;1", //FL_Chaos
	
	"\\MUSIC\\PRY-FOF.XA;1", //FL_Pry_FOF
	
	"\\MUSIC\\RAB.XA;1", //FL_RAB
	
	NULL,
};

typedef struct
{
	const char *name;
	boolean vocal;
} XA_Mp3;

static const XA_Mp3 xa_mp3s[] = {
	//MENU.XA
	{"freakymenu", false},
	{"gameover", false},
	{"storymode", false},
	//TOOSLOW.XA
	{"tooslow", true},
	{"tooslowencore", true},
	//YCR.XA
	{"youcantrun", true},
	//TRIPLETROUB.XA
	{"tripletrouble", true},
	//ENDLESS.XA
	{"endless", true},
	{"endlessencore", true},
	//ENDLESSOG.XA
	{"endlessog", true},
	//CYC-EXEC.XA
	{"cycles", true},
	{"execution", true},
	//SUN-SOUL.XA
	{"sunshine", true},
	{"soulless", true},
	//CHAOS.XA
	{"chaos", true},
	//PRY-FOF.XA
	{"prey", true},
	{"fight or flight", true},
	//RAB.XA
	{"round a bout", true},
	
	{NULL, false}
};
