# PSXFunkin
Friday Night Funkin' on the PSX LOL

## Code samples
```
// Note shake : 
// Add this code in stage.c, case StageState_Play:

if (stage.song_step >= "your step") // Replace "your step" with the step when you want noteshake to start
    noteshake = 1;
else
    noteshake = 0;

// Sprite angle changing :

fixed_t fx, fy;
	
//Draw a rotated texture
fx = stage.camera.x;
fy = stage.camera.y;
	
u8 sprite_angle = 0; // Angle of the sprite, dont put this in a function and dont make it more than 255
RECT tex_src = {0, 0, 255, 255};
RECT_FIXED tex_dst = {
	FIXED_DEC(0,1) - fx, // X position of the sprite
	FIXED_DEC(0,1) - fy, // Y position of the sprite
	FIXED_DEC(256,1), // Width of the sprite
	FIXED_DEC(256,1)  // Height of the sprite
};

// Change sprite angle when L2 or R2 is pressed
if (pad_state.held & PAD_L2)	
   	sprite_angle --;
if (pad_state.held & PAD_R2)
   	sprite_angle ++;
	
Stage_DrawTexRotate(&this->tex_back1, &tex_src, &tex_dst, stage.camera.bzoom, sprite_angle);
```

To use the funkinmultitool do ```./tools/funkinmultitool/funkinmultitool``` in the source code folder

## Compilation
Refer to [COMPILE.md](/COMPILE.md) here

## Disclaimer
This project is not endorsed by the original Friday Night Funkin' devs, this is just an unofficial fan project because I was bored.
