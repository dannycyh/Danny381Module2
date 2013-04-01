#ifndef UI_H_
#define UI_H_
#include "Global.h"

#define SONG_Y_POSITION 4
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

struct Frame{
	struct Frame** elements;
	struct Button** buttons;
	int element_size, button_size;
	int currentPanel; // signify song / list being shown
	struct Image* bg_image;
	struct Frame* mainFrame;


	void (*drawFrame)(struct Frame*);
};

struct Frame* initFrame();
struct Frame* initMainFrame();
struct Frame* initMenuFrame(struct Frame*);
struct Frame* initActionFrame(struct Frame*);
struct Frame* initSongPanel(struct Frame*);
struct Frame* initPlaylistPanel(struct Frame*);

void drawMainFrame(struct Frame*);
void drawMenuFrame(struct Frame*);
void drawActionFrame(struct Frame*);
void drawSongPanel(struct Frame*);
void drawPlaylistPanel(struct Frame*);
void drawScrollFrame(struct Frame*);
void clearSongPanel();
void displayLoadingScreenVGA();

#endif /* FRAME_H_ */
