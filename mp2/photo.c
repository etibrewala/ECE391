/*									tab:8
 *
 * photo.c - photo display functions
 *
 * "Copyright (c) 2011 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 * 
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO 
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED 
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE 
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE, 
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:	    Steve Lumetta
 * Version:	    3
 * Creation Date:   Fri Sep  9 21:44:10 2011
 * Filename:	    photo.c
 * History:
 *	SL	1	Fri Sep  9 21:44:10 2011
 *		First written (based on mazegame code).
 *	SL	2	Sun Sep 11 14:57:59 2011
 *		Completed initial implementation of functions.
 *	SL	3	Wed Sep 14 21:49:44 2011
 *		Cleaned up code for distribution.
 */


#include <string.h>

#include "assert.h"
#include "modex.h"
#include "photo.h"
#include "photo_headers.h"
#include "world.h"

#define LEVEL4_SIZE			 4096		// size of level 4 on octree
#define LEVEL2_SIZE		  	   64		// size of level 2 on octree
#define COLORS_FROM_LVL4 	  128		// number of colors being used from level 4

#define SHIFT_RED_LVL4	   		4		// num of shift left for 4MSB red
#define SHIFT_GREEN_LVL4   		3		// num of shift left for 4MSB green
#define SHIFT_BLUE_LVL4	   		1		// num of shift left for 4MSB blue

#define SHIFT_RED_COLOR		   10		// bits to shift to get RED
#define SHIFT_GREEN_COLOR		5		// to get GREEN
#define SHIFT_BLUE_COLOR		1		// to get BLUE

#define PALETTE_RED				0		// access color in palette
#define PALETTE_GREEN			1
#define PALETTE_BLUE			2

#define PALETTE_OFFSET_LVL2	  192

#define BITMASKRB		   0x003E		// 5 bit mask from R and B
#define BITMASKG		   0x003F		// 6 bit mask for G

#define RED_BITMASK12  	   0x0F00		// bitmask to get level 4 index
#define GREEN_BITMASK12	   0x00F0
#define BLUE_BITMASK12	   0X000F

// Initialize my array of octree structs for level 2 and level 4
//======================================================
struct octree_t octree_level4[LEVEL4_SIZE];
struct octree_t octree_level2[LEVEL2_SIZE];
//======================================================
/* types local to this file (declared in types.h) */

/* 
 * A room photo.  Note that you must write the code that selects the
 * optimized palette colors and fills in the pixel data using them as 
 * well as the code that sets up the VGA to make use of these colors.
 * Pixel data are stored as one-byte values starting from the upper
 * left and traversing the top row before returning to the left of
 * the second row, and so forth.  No padding should be used.
 */
struct photo_t {
    photo_header_t hdr;			/* defines height and width */
    uint8_t        palette[192][3];     /* optimized palette colors */
    uint8_t*       img;                 /* pixel data               */
};

/* 
 * An object image.  The code for managing these images has been given
 * to you.  The data are simply loaded from a file, where they have 
 * been stored as 2:2:2-bit RGB values (one byte each), including 
 * transparent pixels (value OBJ_CLR_TRANSP).  As with the room photos, 
 * pixel data are stored as one-byte values starting from the upper 
 * left and traversing the top row before returning to the left of the 
 * second row, and so forth.  No padding is used.
 */
struct image_t {
    photo_header_t hdr;			/* defines height and width */
    uint8_t*       img;                 /* pixel data               */
};

/* file-scope variables */

/* 
 * The room currently shown on the screen.  This value is not known to 
 * the mode X code, but is needed when filling buffers in callbacks from 
 * that code (fill_horiz_buffer/fill_vert_buffer).  The value is set 
 * by calling prep_room.
 */
static const room_t* cur_room = NULL; 


/* 
 * fill_horiz_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the leftmost 
 *                pixel of a line to be drawn on the screen, this routine 
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- leftmost pixel of line to be drawn 
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_horiz_buffer (int x, int y, unsigned char buf[SCROLL_X_DIM])
{
    int            idx;   /* loop index over pixels in the line          */ 
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgx;  /* loop index over pixels in object image      */ 
    int            yoff;  /* y offset into object image                  */ 
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_X_DIM; idx++) {
        buf[idx] = (0 <= x + idx && view->hdr.width > x + idx ?
		    view->img[view->hdr.width * y + x + idx] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
    	 obj = obj_next (obj)) {
	obj_x = obj_get_x (obj);
	obj_y = obj_get_y (obj);
	img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
	if (y < obj_y || y >= obj_y + img->hdr.height ||
	    x + SCROLL_X_DIM <= obj_x || x >= obj_x + img->hdr.width) {
	    continue;
	}

	/* The y offset of drawing is fixed. */
	yoff = (y - obj_y) * img->hdr.width;

	/* 
	 * The x offsets depend on whether the object starts to the left
	 * or to the right of the starting point for the line being drawn.
	 */
	if (x <= obj_x) {
	    idx = obj_x - x;
	    imgx = 0;
	} else {
	    idx = 0;
	    imgx = x - obj_x;
	}

	/* Copy the object's pixel data. */
	for (; SCROLL_X_DIM > idx && img->hdr.width > imgx; idx++, imgx++) {
	    pixel = img->img[yoff + imgx];

	    /* Don't copy transparent pixels. */
	    if (OBJ_CLR_TRANSP != pixel) {
		buf[idx] = pixel;
	    }
	}
    }
}


/* 
 * fill_vert_buffer
 *   DESCRIPTION: Given the (x,y) map pixel coordinate of the top pixel of 
 *                a vertical line to be drawn on the screen, this routine 
 *                produces an image of the line.  Each pixel on the line
 *                is represented as a single byte in the image.
 *
 *                Note that this routine draws both the room photo and
 *                the objects in the room.
 *
 *   INPUTS: (x,y) -- top pixel of line to be drawn 
 *   OUTPUTS: buf -- buffer holding image data for the line
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
fill_vert_buffer (int x, int y, unsigned char buf[SCROLL_Y_DIM])
{
    int            idx;   /* loop index over pixels in the line          */ 
    object_t*      obj;   /* loop index over objects in the current room */
    int            imgy;  /* loop index over pixels in object image      */ 
    int            xoff;  /* x offset into object image                  */ 
    uint8_t        pixel; /* pixel from object image                     */
    const photo_t* view;  /* room photo                                  */
    int32_t        obj_x; /* object x position                           */
    int32_t        obj_y; /* object y position                           */
    const image_t* img;   /* object image                                */

    /* Get pointer to current photo of current room. */
    view = room_photo (cur_room);

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_Y_DIM; idx++) {
        buf[idx] = (0 <= y + idx && view->hdr.height > y + idx ?
		    view->img[view->hdr.width * (y + idx) + x] : 0);
    }

    /* Loop over objects in the current room. */
    for (obj = room_contents_iterate (cur_room); NULL != obj;
    	 obj = obj_next (obj)) {
	obj_x = obj_get_x (obj);
	obj_y = obj_get_y (obj);
	img = obj_image (obj);

        /* Is object outside of the line we're drawing? */
	if (x < obj_x || x >= obj_x + img->hdr.width ||
	    y + SCROLL_Y_DIM <= obj_y || y >= obj_y + img->hdr.height) {
	    continue;
	}

	/* The x offset of drawing is fixed. */
	xoff = x - obj_x;

	/* 
	 * The y offsets depend on whether the object starts below or 
	 * above the starting point for the line being drawn.
	 */
	if (y <= obj_y) {
	    idx = obj_y - y;
	    imgy = 0;
	} else {
	    idx = 0;
	    imgy = y - obj_y;
	}

	/* Copy the object's pixel data. */
	for (; SCROLL_Y_DIM > idx && img->hdr.height > imgy; idx++, imgy++) {
	    pixel = img->img[xoff + img->hdr.width * imgy];

	    /* Don't copy transparent pixels. */
	    if (OBJ_CLR_TRANSP != pixel) {
		buf[idx] = pixel;
	    }
	}
    }
}


/* 
 * image_height
 *   DESCRIPTION: Get height of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
image_height (const image_t* im)
{
    return im->hdr.height;
}


/* 
 * image_width
 *   DESCRIPTION: Get width of object image in pixels.
 *   INPUTS: im -- object image pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of object image im in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
image_width (const image_t* im)
{
    return im->hdr.width;
}

/* 
 * photo_height
 *   DESCRIPTION: Get height of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: height of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
photo_height (const photo_t* p)
{
    return p->hdr.height;
}


/* 
 * photo_width
 *   DESCRIPTION: Get width of room photo in pixels.
 *   INPUTS: p -- room photo pointer
 *   OUTPUTS: none
 *   RETURN VALUE: width of room photo p in pixels
 *   SIDE EFFECTS: none
 */
uint32_t 
photo_width (const photo_t* p)
{
    return p->hdr.width;
}


/* 
 * prep_room
 *   DESCRIPTION: Prepare a new room for display.  You might want to set
 *                up the VGA palette registers according to the color
 *                palette that you chose for this room.
 *   INPUTS: r -- pointer to the new room
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes recorded cur_room for this file
 */
void
prep_room (const room_t* r)
{
    /* Record the current room. */
    cur_room = r;
	photo_t *photo = room_photo(r);			//initialize current room to a photo
	unsigned char* palette_RGB = (unsigned char *)photo->palette;
	fill_octree_palette(palette_RGB);	//send palette for current photo to ports
}

/* 
 * read_obj_image
 *   DESCRIPTION: Read size and pixel data in 2:2:2 RGB format from a
 *                photo file and create an image structure from it.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the image
 */
image_t*
read_obj_image (const char* fname)
{
    FILE*    in;		/* input file               */
    image_t* img = NULL;	/* image structure          */
    uint16_t x;			/* index over image columns */
    uint16_t y;			/* index over image rows    */
    uint8_t  pixel;		/* one pixel from the file  */

    /* 
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the image pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
	NULL == (img = malloc (sizeof (*img))) ||
	NULL != (img->img = NULL) || /* false clause for initialization */
	1 != fread (&img->hdr, sizeof (img->hdr), 1, in) ||
	MAX_OBJECT_WIDTH < img->hdr.width ||
	MAX_OBJECT_HEIGHT < img->hdr.height ||
	NULL == (img->img = malloc 
		 (img->hdr.width * img->hdr.height * sizeof (img->img[0])))) {
	if (NULL != img) {
	    if (NULL != img->img) {
	        free (img->img);
	    }
	    free (img);
	}
	if (NULL != in) {
	    (void)fclose (in);
	}
	return NULL;
    }

    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */
    for (y = img->hdr.height; y-- > 0; ) {

	/* Loop over columns from left to right. */
	for (x = 0; img->hdr.width > x; x++) {

	    /* 
	     * Try to read one 8-bit pixel.  On failure, clean up and 
	     * return NULL.
	     */
	    if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
		free (img->img);
		free (img);
	        (void)fclose (in);
		return NULL;
	    }

	    /* Store the pixel in the image data. */
	    img->img[img->hdr.width * y + x] = pixel;
	}
    }

    /* All done.  Return success. */
    (void)fclose (in);
    return img;
}


/* 
 * read_photo
 *   DESCRIPTION: Read size and pixel data in 5:6:5 RGB format from a
 *                photo file and create a photo structure from it.
 *                Code provided simply maps to 2:2:2 RGB.  You must
 *                replace this code with palette color selection, and
 *                must map the image pixels into the palette colors that
 *                you have defined.
 *   INPUTS: fname -- file name for input
 *   OUTPUTS: none
 *   RETURN VALUE: pointer to newly allocated photo on success, or NULL
 *                 on failure
 *   SIDE EFFECTS: dynamically allocates memory for the photo
 */
photo_t*
read_photo (const char* fname)
{
    FILE*    in;	/* input file               */
    photo_t* p = NULL;	/* photo structure          */
    uint16_t x;		/* index over image columns */
    uint16_t y;		/* index over image rows    */
    uint16_t pixel;	/* one pixel from the file  */


	int init_oct2;												// iterator to initialize 2nd level octree
	for(init_oct2 = 0; init_oct2 < LEVEL2_SIZE; init_oct2++){	// initialize array of octrees for level 2
		octree_level2[init_oct2].red = 0;						// set all the RGB vals to zero
		octree_level2[init_oct2].green = 0;
		octree_level2[init_oct2].blue = 0;
		octree_level2[init_oct2].pixel_count = 0;				// so that we can keep track of how many pixels map to color
	}

	int init_oct4;												// iterator to initialize 4th level of octree
	for(init_oct4 = 0; init_oct4 < LEVEL4_SIZE; init_oct4++){	// initialize array of octrees for level 4
		octree_level4[init_oct4].red = 0;						// set all the RGB vals to zero
		octree_level4[init_oct4].green = 0;
		octree_level4[init_oct4].blue = 0;
		octree_level4[init_oct4].pixel_count = 0;				// keep track of how many pixels map to color
	}

    /* 
     * Open the file, allocate the structure, read the header, do some
     * sanity checks on it, and allocate space to hold the photo pixels.
     * If anything fails, clean up as necessary and return NULL.
     */
    if (NULL == (in = fopen (fname, "r+b")) ||
	NULL == (p = malloc (sizeof (*p))) ||
	NULL != (p->img = NULL) || /* false clause for initialization */
	1 != fread (&p->hdr, sizeof (p->hdr), 1, in) ||
	MAX_PHOTO_WIDTH < p->hdr.width ||
	MAX_PHOTO_HEIGHT < p->hdr.height ||
	NULL == (p->img = malloc 
		 (p->hdr.width * p->hdr.height * sizeof (p->img[0])))) {
	if (NULL != p) {
	    if (NULL != p->img) {
	        free (p->img);
	    }
	    free (p);
	}
	if (NULL != in) {
	    (void)fclose (in);
	}
	return NULL;
    }

	//===========================================================
	//Variables used in pixel loop
	//===========================================================
	int nodes_lvl4;
	int red_lvl4, green_lvl4, blue_lvl4;
	int red_color, green_color, blue_color;


    /* 
     * Loop over rows from bottom to top.  Note that the file is stored
     * in this order, whereas in memory we store the data in the reverse
     * order (top to bottom).
     */

    for (y = p->hdr.height; y-- > 0; ) {

	/* Loop over columns from left to right. */
		for (x = 0; p->hdr.width > x; x++) {

			/* 
			* Try to read one 16-bit pixel.  On failure, clean up and 
			* return NULL.
			*/
			if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
			free (p->img);
			free (p);
				(void)fclose (in);
			return NULL;

			}
			/* 
			* 16-bit pixel is coded as 5:6:5 RGB (5 bits red, 6 bits green,
			* and 6 bits blue).  We change to 2:2:2, which we've set for the
			* game objects.  You need to use the other 192 palette colors
			* to specialize the appearance of each photo.
			*
			* In this code, you need to calculate the p->palette values,
			* which encode 6-bit RGB as arrays of three uint8_t's.  When
			* the game puts up a photo, you should then change the palette 
			* to match the colors needed for that photo.
			*/
			// p->img[p->hdr.width * y + x] = (((pixel >> 14) << 4) |
			// 				(((pixel >> 9) & 0x3) << 2) |
			// 				((pixel >> 3) & 0x3));

			// assign color index in level 4 octree
			red_lvl4 = (pixel >> SHIFT_RED_LVL4) & RED_BITMASK12;							    // shift to get 4 MSB of red
			green_lvl4 = (pixel >> SHIFT_GREEN_LVL4) & GREEN_BITMASK12;							// shift to get 4 MSB of green
			blue_lvl4 = (pixel >> SHIFT_BLUE_LVL4) & BLUE_BITMASK12;							// shift to get 4 MSB of blue
			nodes_lvl4 = red_lvl4 | green_lvl4 | blue_lvl4;				// concatanate to get index into stuct array

			red_color = (pixel >> SHIFT_RED_COLOR) & BITMASKRB;			// get the 6-bit RGB colors from pixel
			green_color = (pixel >> SHIFT_GREEN_COLOR) & BITMASKG;
			blue_color = (pixel << SHIFT_BLUE_COLOR) & BITMASKRB;

			octree_level4[nodes_lvl4].red += red_color;					// assign RGB pixel colors to struct array for level 4
			octree_level4[nodes_lvl4].green += green_color;
			octree_level4[nodes_lvl4].blue += blue_color;
			octree_level4[nodes_lvl4].pixel_count+=1;
			octree_level4[nodes_lvl4].color_idx = nodes_lvl4;			// save level 4 index for palette clean up later
		}
    }
	
	/*call qsort() to sort level 4 array to find first 128 color values*/
	qsort(octree_level4,LEVEL4_SIZE,sizeof(struct octree_t),comparator); // void* changed

	int iter;
	for(iter = COLORS_FROM_LVL4; iter < LEVEL4_SIZE; iter++){		// store unused level 4 colors in level 2
		int lvl4_idx = octree_level4[iter].color_idx;

		// get level 2 index to store rest of colors
		int lvl2_idx = ((((lvl4_idx>>10) & 0x03) << 4) | (((lvl4_idx>>6) & 0x03) << 2) | ((lvl4_idx >> 2) & 0x03));
	
		octree_level2[lvl2_idx].red += octree_level4[iter].red;
		octree_level2[lvl2_idx].green += octree_level4[iter].green;
		octree_level2[lvl2_idx].blue += octree_level4[iter].blue;
		octree_level2[lvl2_idx].pixel_count += octree_level4[iter].pixel_count;
		octree_level2[lvl2_idx].color_idx = lvl2_idx;						// don't need to save a color idx
	}

	int i;

	for(i=0; i < COLORS_FROM_LVL4; i++){
	if(octree_level4[i].pixel_count > 0){
		p->palette[i][PALETTE_RED] = octree_level4[i].red / octree_level4[i].pixel_count;
		p->palette[i][PALETTE_GREEN] = octree_level4[i].green / octree_level4[i].pixel_count;
		p->palette[i][PALETTE_BLUE] = octree_level4[i].blue / octree_level4[i].pixel_count;
		}
	}

	for(i = 0; i < LEVEL2_SIZE; i++){
		if(octree_level2[i].pixel_count > 0){
			p->palette[i + COLORS_FROM_LVL4][PALETTE_RED] = octree_level2[i].red / octree_level2[i].pixel_count;
			p->palette[i + COLORS_FROM_LVL4][PALETTE_GREEN] = octree_level2[i].green / octree_level2[i].pixel_count;
			p->palette[i + COLORS_FROM_LVL4][PALETTE_BLUE] = octree_level2[i].blue / octree_level2[i].pixel_count;
		}	
	}
	

	fseek(in, sizeof(p->hdr),SEEK_SET);

    for (y = p->hdr.height; y-- > 0; ) {

	/* Loop over columns from left to right. */
		for (x = 0; p->hdr.width > x; x++) {
			
			if (1 != fread (&pixel, sizeof (pixel), 1, in)) {
			free (p->img);
			free (p);
				(void)fclose (in);
			return NULL;

			}

			draw_pixels(pixel,p,x,y);
		}
	}	

    /* All done.  Return success. */
    (void)fclose (in);
    return p;
}


int comparator(const void* a, const void* b){
	int node1 = ((struct octree_t*)a)->pixel_count;
	int node2 = ((struct octree_t*)b)->pixel_count;
	return(node2-node1);
}


void draw_pixels(uint16_t pixel, photo_t* p, uint16_t x, uint16_t y){

	int lvl4_iter;
	int red_lvl4, green_lvl4, blue_lvl4;

	red_lvl4 = (pixel >> SHIFT_RED_LVL4) & RED_BITMASK12;							// shift to get 4 MSB of red
	green_lvl4 = (pixel >> SHIFT_GREEN_LVL4) & GREEN_BITMASK12;							// shift to get 4 MSB of green
	blue_lvl4 = (pixel >> SHIFT_BLUE_LVL4) & BLUE_BITMASK12;							// shift to get 4 MSB of blue
	lvl4_iter = red_lvl4 | green_lvl4 | blue_lvl4;
	
	int lvl2_iter = ((((lvl4_iter>>10) & 0x03) << 4) | (((lvl4_iter>>6) & 0x03) << 2) | ((lvl4_iter>>2) & 0x03));



	int i;

	for(i = 0; i < COLORS_FROM_LVL4; i++){
		if(lvl4_iter == octree_level4[i].color_idx){
			p->img[p->hdr.width * y + x] = i+LEVEL2_SIZE;
			break;
		}
		else{
			p->img[p->hdr.width * y + x] = PALETTE_OFFSET_LVL2 + lvl2_iter;
		}
	}
}
