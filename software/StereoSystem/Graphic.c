/*
 * Graphic.c
 *
 *  Created on: 2013-03-21
 *      Author: danny
 */
#include "Graphic.h"

alt_up_pixel_buffer_dma_dev* pixel_buffer;
alt_up_char_buffer_dev* char_buffer;
/*
* this function clear the screen and set up pixel buffers for graphics
*/
void initVGA() {
	// Use the name of your pixel buffer DMA core
	pixel_buffer =alt_up_pixel_buffer_dma_open_dev("/dev/pixel_buffer_dma_0");

	//text on screen initialization
	char_buffer = alt_up_char_buffer_open_dev("/dev/char_drawer");
	alt_up_char_buffer_init(char_buffer);

	// Set the background buffer address � Although we don�t use thebackground,
	// they only provide a function to change the background buffer address, so
	// we must set that, and then swap it to the foreground.
	unsigned int pixel_buffer_addr1 = PIXEL_BUFFER_BASE;
	unsigned int pixel_buffer_addr2 = PIXEL_BUFFER_BASE + (320*240*2);

	alt_up_pixel_buffer_dma_change_back_buffer_address(pixel_buffer,pixel_buffer_addr1);
	// Swap background and foreground buffers
	alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
	// Wait for the swap to complete
	while(alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));

	alt_up_pixel_buffer_dma_change_back_buffer_address(pixel_buffer,pixel_buffer_addr2);
	// Clear the screen
	alt_up_pixel_buffer_dma_clear_screen(pixel_buffer, 0);
	alt_up_pixel_buffer_dma_clear_screen(pixel_buffer, 1);

	//Swap background and foreground buffers
	alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);
	//Wait for the swap to complete
	while(alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));

}

/*
* Load bitmap image from SD card.
* file name is required to be upper-case
* return Image struct if success; null otherwise
*/

struct Image* loadSDImage(char* filename) {
	int i, j, bytes = 0, offset = 0, byte = 0;
	int a, b;
	int height = 0, width = 0;
	int size = 0, count = 0;
	char bmpId[3];
	int* result = NULL;
	memset(bmpId, 0, 3);
	short int file_pointer = alt_up_sd_card_fopen(filename, false);
	if(file_pointer < 0) {
		alt_up_sd_card_fclose(file_pointer); //close the file
		return NULL;
	}
	//Start reading the bitmap header
	while(bytes < 2) {
		if((bmpId[bytes++] = alt_up_sd_card_read(file_pointer)) < 0) {
			alt_up_sd_card_fclose(file_pointer);
			printf("fail reading %s\n", filename);
			return NULL;
		}
	}
	if(strcmp(bmpId, "BM") != 0) {
		alt_up_sd_card_fclose(file_pointer);
		printf("fail reading %s at ID %s\n", filename, bmpId);
		return NULL;
	}
	short int temp;
	while(bytes < 10) {
		if((temp = alt_up_sd_card_read(file_pointer)) < 0) {
			alt_up_sd_card_fclose(file_pointer);
			printf("fail reading %s\n", filename);
			return NULL;
		}
		bytes++;
	}

	if((offset = alt_up_sd_card_read(file_pointer))< 0) {
		alt_up_sd_card_fclose(file_pointer);
		return NULL;
	} printf("offset: %x\n", offset);
	bytes++;
	while(bytes < offset){
		if(bytes == 18) {
			if((width = alt_up_sd_card_read(file_pointer))< 0) {
				alt_up_sd_card_fclose(file_pointer);
				return false;
			} printf("width: %d, ", width);
		} else if(bytes == 22) {
			if((height = alt_up_sd_card_read(file_pointer))< 0) {
				alt_up_sd_card_fclose(file_pointer);
				return false;
			} printf("height: %d\n", height);
		} else if(bytes == 34 || bytes == 35 || bytes == 36 || bytes == 37) {
			if((temp = alt_up_sd_card_read(file_pointer))< 0) {
				alt_up_sd_card_fclose(file_pointer);
				return false;
			}  size += (int)(temp << 8*(count++));
			if(bytes == 37) {
				printf("data size: %d\n", size/2);
				result = (int*)malloc(sizeof(int)*(size/2));
			}
		} else if((temp = alt_up_sd_card_read(file_pointer)) < 0) {
			alt_up_sd_card_fclose(file_pointer);
			return NULL;
		} //printf("%d %x\n", bytes, temp);
		bytes++;
	}
	//Start reading the pixel data
	for(j = height-1; j >= 0; j--) {
		for(i = 0; i < size/2/height; i++) {
			a = alt_up_sd_card_read(file_pointer);
			b = alt_up_sd_card_read(file_pointer);
			if(a < 0 || b < 0) {
				printf("%s invalid at pixel[%d, %d]!\n", filename, i, j);
				free(result);
				result = NULL;
				alt_up_sd_card_fclose(file_pointer);
				return NULL;
			}
			byte = getColor555(b*256+a);
			*(result + j*(width)+i) = byte;
		}
	}
	alt_up_sd_card_fclose(file_pointer);
	return initImage(result, 0, width, height);
}
/*
 * This function draw the image and set the previous position for the image
 */
void draw(int pos_x, int pos_y, struct Image* this) {
	if(this == NULL || this->buffer == NULL) return;
	if(pos_x < 0 || pos_y < 0 || pos_x + this->width>= 320 || pos_y + this->height>= 240) {
		printf("draw image out of boundary\n");
		return;
	}
	setImagePos(this, pos_x, pos_y);
	int i, j;
	for(i = 0; i < this->width; i++) {
		for(j = 0; j < this->height; j++) {
			if(*(this->buffer+ j*this->width+i) != BACKGROUND) {
				IOWR_16DIRECT(pixel_buffer->buffer_start_address, ((pos_y+j)*320+pos_x+i)<<1, *(this->buffer + j*this->width+i));
			}
		}
	}
}
void draw_notransparent(int pos_x, int pos_y, struct Image* this) {
	if(this == NULL || this->buffer == NULL) return;
	if(pos_x < 0 || pos_y < 0 || pos_x + this->width>= 320 || pos_y + this->height>= 240) {
		printf("draw image out of boundary\n");
		return;
	}
	setImagePos(this, pos_x, pos_y);
	int i, j;
	for(i = 0; i < this->width; i++) {
		for(j = 0; j < this->height; j++) {
			//if(*(this->buffer+ j*this->width+i) != 0) {
			IOWR_16DIRECT(pixel_buffer->buffer_start_address, ((pos_y+j)*320+pos_x+i)<<1, *(this->buffer + j*this->width+i));
			//}
		}
	}
}
/*helper function to convert 32 bit color code to 16 bit color*/

int getColor(int red, int green, int blue) {
	return (int)(((red>>3)<<11) + ((green>>2)<<5) + (blue>>3));
}
int getColor555(int color555) {
	int color = color555&0x7FFF;
	return (color&0x7C00)*2+(color&0x03E0)*2+(color&0x1F);
}

/*
 * Constructor of Animation; start == 1 if this animation contains the first image
 * in the sequence;
 */
struct Image* initImage(int* img, int start, int width, int height) {
	struct Image* a;
	while((a = (struct Image*)malloc(sizeof(struct Image)))==NULL) {printf("memory allocation with aniimation\n");}
	a->width = width;
	a->height = height;
	a->x = a->y = a->prev_x = a->prev_y = 0;
	a->buffer = img;
	a->prev = a;
	a->next = a;
	a->end = a;
	if(start > 0)
		a->first = a;
	else
		a->first = NULL;
	return a;
}

/*
 * Destructor;
 */
void killImage(struct Image* this) {
	if(this == NULL) return;
	if(this->prev != NULL) {
		this->prev = NULL;
	}
	free(this->buffer);
	this->buffer = NULL;
	if(this->end != this)
		killImage(this->next);

	this->next = NULL;
	this->first = NULL;
	this->end = NULL;
	free(this);
	this = NULL;
}
/*
 * add an image to the animation
 */
void addImage(struct Image *curr, struct Image* n) {
	n->prev = curr->end;
	curr->end->next = n;
	curr->end = n;
	curr->first->prev = n;
	n->first = curr->first;
	n->next = curr->first;
}
/*
 * set the position of the image
 */
void setImagePos(struct Image* this, int pos_x, int pos_y) {
	this->prev_x = this->x;
	this->prev_y = this->y;
	this->x = pos_x;
	this->y = pos_y;
}
