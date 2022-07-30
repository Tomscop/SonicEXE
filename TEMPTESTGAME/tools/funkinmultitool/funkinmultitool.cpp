//A little program to help with psxfunkin ports

#include <iostream>
#include <fstream>

int mode;

using namespace std;

int vramx, vramy, palx, paly, bpp;
char animname[30], curfile[30];
int animlength;
int TextHelper();

string arcdir, arcname, idlename, leftname, downname, upname, rightname, specname;
int lengthi, lengthl, lengthd, lengthu, lengthr, lengthspec;
void PrintAnim(string animname, int dalength);
int GetAnimLength(string animname);
int MakefileHelper();

int spritew, spriteh;
double spritescale;
double calculatedscale;
int SizeHelper();


int main()
{
	cout << "input the program mode (0 = .png.txt maker, 1 = makefile.tim helper, 2 = sprite sheet size calculator)" << endl;
	cin >> mode;

	switch (mode)	
	{
		case 0:
			TextHelper();
		break;
		case 1:
			MakefileHelper();
		break;
		case 2:
			SizeHelper();
		break;
  		default: 
  			cout << "invalid mode!" << endl;
		break;
	}

    return 0;
}

int TextHelper()
{
    ofstream dafile;

	//Get user input
	cout << "disclamer: this program will create a lot of files in the current directory you are in" << endl;
    cout << "input a vram x value" << endl;
    cin >> vramx;
    cout << "input a vram y value" << endl;
    cin >> vramy;
    cout << "input a pallete x value" << endl;
    cin >> palx;
    cout << "input a pallete y value" << endl;
    cin >> paly;
    cout << "input a bpp value (must be 4 or 8)" << endl;
    cin >> bpp;
    cout << "input the animation name" << endl;
    cin >> animname;
    cout << "input the amount of animations (starts from 0)" << endl;
    cin >> animlength;
	    
    if (!(bpp == 4 || bpp == 8))
    {
    	cout << "invalid bpp!" << endl;
    	return 0;
    }

    //Write the files
    for (int i = 0; i <= animlength; i++)
    {
		sprintf(curfile, "%s%d.png.txt", animname, i);
    	dafile.open(curfile); // opens the file
    	dafile << vramx << " " << vramy << " " << palx << " " << paly << " " << bpp << endl; //Place the values in the file
    	if (curfile == NULL)
			cout << "failed to create " << curfile << endl;
    	else
			cout << "created " << curfile << endl;

    	dafile.close();
    }

    return 0;
}

int MakefileHelper()
{
	//Get user input
	cout << "input arc directory (ex: iso/bf/)" << endl;
    cin >> arcdir;
	cout << "input arc name (ex: main.arc)" << endl;
    cin >> arcname;
	cout << "idle anim name (ex: idle) note: if you dont need this just put NULL" << endl;
    cin >> idlename;
	cout << "left anim name (ex: left) note: if you dont need this just put NULL" << endl;
    cin >> leftname;
	cout << "down anim name (ex: down) note: if you dont need this just put NULL" << endl;
    cin >> downname;
	cout << "up anim name (ex: up) note: if you dont need this just put NULL" << endl;
    cin >> upname;
	cout << "right anim name (ex: right) note: if you dont need this just put NULL" << endl;
    cin >> rightname;
	cout << "special anim name (ex: shoot) note: if you dont need this just put NULL" << endl;
    cin >> specname;
    lengthi = GetAnimLength(idlename);
    lengthl = GetAnimLength(leftname);
    lengthd = GetAnimLength(downname);
    lengthu = GetAnimLength(upname);
    lengthr = GetAnimLength(rightname);
    lengthspec = GetAnimLength(specname);

    cout << arcdir << arcname << ": "; //prints like iso/bf/main.arc:

   	PrintAnim(idlename, lengthi);
   	PrintAnim(leftname, lengthl);
   	PrintAnim(downname, lengthd);
   	PrintAnim(upname, lengthu);
   	PrintAnim(rightname, lengthr);
   	PrintAnim(specname, lengthspec);

    return 0;
}

void PrintAnim(string animname, int dalength)
{
	if (animname != "NULL")
	{
		for (int i = 0; i <= dalength; i++)
	    {
	    	if (dalength == 0)
			    cout << arcdir << animname << ".tim "; //prints like iso/bf/down.tim
		    else
			    cout << arcdir << animname << i << ".tim "; //prints like iso/bf/down1.tim
	    }
	}
}

int GetAnimLength(string animname)
{
	int animlength;
	if (animname != "NULL")
	{
		cout << animname << " length (starts from 0)" << endl;
		cin >> animlength;
	}
	return animlength;
}

int SizeHelper()
{
	//Get user input
	std::cout << "Type the sprite scale (example: 0.6)" << std::endl;
	std::cin >> spritescale; 

	std::cout << "Type the sprite width" << std::endl;
	std::cin >> spritew; 

	std::cout << "Type the sprite height" << std::endl;
	std::cin >> spriteh; 

	//Calculate stuff
	spritew *= spritescale / 4;
	spriteh *= spritescale / 4;

	std::cout << "image size is " << spritew << "x" << spriteh << std::endl;

    return 0;
}
