# ECE 391 MP2 Bug Log

## MP2.1

### Bug Description:

Random pixels where drawing to my status bar. The background was correctly displaying the color I wanted, but I was getting weird lines of my foreground color that were not actual ascii characters on the status bar.

### Bug Resolution:

I realized that my buffer indexing equation was very messed up, and I was not actually populating the buffer the way I wanted. After taking a closer look at my equation, I noticed that the buffer was currently being populated in such a way that each letter in my `buffer_message` was essentially getting stacked in the buffer. So the first 128 values in my buffer were all one character, the second 128 were the next ascii character, and so on. However the buffer was supposed to be loaded row by row, so the first 320 values of the buffer would include the first row of each of my ascii characters of my message and so on. Once I corrected the indexing equation, my buffer functioned properly and I could see actual ascii characters being populated into my buffer.

### Bug Description:

My typed text was not being displayed to the status bar, but I could see my room name on the left hand side of my status bar. I could also see the status message when there was an update, and I could perform actions such as loading my inventory. So my code was properly recognizing the typed commands, it just was not correctly displaying to the buffer.

### Bug Resolution:

I realized that in my for loops to load my buffer name, I was directly comparing my iterator to the return value of the `strlen()` function. While this worked for some of my for loops, it did not work when I was simultaneously loading in both ends of my buffer message on two concurrent for loops. It only wrote my room name, which was being loaded in with my first for loop. However once I assigned the `strlen(typed_command)` to a temp variable and then used that variable as the bounds of my for loop, my buffer was populated correctly and began to show both the typed command on the right side of the status bar and the room name on the left hand side of the status bar.  

### Bug Description:

My logic for accessing my palette was wrong when implementing my octrees. I initially made it such that I would add the RGB values to both my level two and my level four, then go back and iterate through my level four and remove the same colors from level two. This caused a lot of problems because logically it made sense, but I was not implementing it properly so my color palette was assigned all wrong and I was getting weird colors to show on screen.

### Bug Resolution:

I solved this problem by changing my logic. I learned that it was easier to first fill my entire 4th level of my octree, sort to get the most common 128 colors, and then use the rest to fill my second layer. Implementing this logic was much simpler and a lot easier to debug, and so I was able to properly fill my palette after I implemented my octree levels differently.

### Bug Description:

When drawing to my image, I was checking if the pixel color of the room was equal to the RGB value of the palette, then assigning the pixel color to the palette. I initally did this because I assumed that all the colors in the palette where what we were going to assign to all the pixels in the picture, so when we are looking for the right pixel to draw, we directly compare to the palette. This caused my screen to appear all green in most rooms, and every once in a while I would have random pixels of other colors. 

### Bug Resolution:

I realized that the palette itself was being assigned by me earlier in the code, and the color values were based on the average of the RGB colors for a given image. Also each palette was actually different for each room, so my comparing the pixel of one room with the averages of the pixels of the same room would saturate the image and lead to the same color being shown everywhere. The palette is simply for us to access by an offset to fill the colors in the same area with a similar value, so my directly comparing it to the palette values we were actually comparing with the wrong values. Instead, I used the colors directly in my octrees, by first iterating through the 4th level, and then the 2nd, to find the color that best fit the location of the pixel.

### Bug Description:

When testing my emulator, my buttons were working correctly, but they were extremely delayed or when I was trying to move the screen, it would move all the way to the end. So for example if I pressed down, the screen would immediately jump to the bottom. This behaviour would happen in all directions, and the ABC buttons were extremely delayed or not working at all.

### Bug Resolution:

I was checking if the button pressed was CMD_NONE or not when I was trying to get my tux command, but that logic was causing the signals to get messed up. So when though I had the right conditions to see if my buttons were pressed or not, they game would do weird things when I used the tux. However when I created a random temp variable to check whether or not my button was pressed, the problem was resolved. I made essentially made this a boolean variable by making it an integer that toggles between 1 or 0 based on the button press. After implementing this my code worked seamlessly.