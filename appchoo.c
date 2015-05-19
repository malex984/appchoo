/*
appchoo - Application Chooser with Timeout
Written in 2012 by <Ahmet Inan> <ainan@mathematik.uni-freiburg.de>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <SDL_rotozoom.h>

#include <getopt.h>
#include <string.h>

static TTF_Font* font;

int fit_image(SDL_Surface *image, int w, int h)
{
	if (!image)
		return 0;
	int bytes = image->format->BytesPerPixel;
	if (bytes != 3 && bytes != 4)
		return 0;
	if (w >= image->w && h >= image->h)
		return 1;
	int fx = (image->w + (w-1)) / w;
	int fy = (image->h + (h-1)) / h;
	int f = fx > fy ? fx : fy;
	int pitch = image->pitch;
	image->clip_rect.w = image->w /= f;
	image->clip_rect.h = image->h /= f;
	image->pitch = (image->w * bytes + 3) & (~3);
	uint8_t *pixels = image->pixels;
	for (int y = 0; y < image->h; y++) {
		for (int x = 0; x < image->w; x++) {
			for (int c = 0; c < bytes; c++) {
				uint32_t sum = 0;
				for (int j = 0; j < f; j++)
					for (int i = 0; i < f; i++)
						 sum += pixels[(y*f+j) * pitch + (x*f+i) * bytes + c];
				pixels[y * image->pitch + x * bytes + c] = sum / (f*f);
			}
		}
	}
	return 1;
}

void center_image(SDL_Rect *dest, SDL_Rect *src)
{
	if (dest->w > src->w) {
		dest->x += dest->w / 2 - src->w / 2;
		dest->w = src->w;
	} else {
		src->x += src->w / 2 - dest->w / 2;
		src->w = dest->w;
	}
	if (dest->h > src->h) {
		dest->y += dest->h / 2 - src->h / 2;
		dest->h = src->h;
	} else {
		src->y += src->h / 2 - dest->h / 2;
		src->h = dest->h;
	}
}

int handle_corner(int w, int h, int x, int y, int r2, int c)
{
	int cx = c & 1 ? w - 1 : 0;
	int cy = c & 2 ? h - 1 : 0;
	int d2 = (cx - x) * (cx - x) + (cy - y) * (cy - y);
	return r2 > d2;
}

void handle_events(SDL_Surface *screen, SDL_Rect *rects, char **apps, int num, char corners[4][2048], int r2)
{
	static int mouse_x = 0;
	static int mouse_y = 0;
	int button = 0;
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_q:
						exit(1);
						break;
					case SDLK_ESCAPE:
						exit(1);
						break;
					default:
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				mouse_x = event.motion.x;
				mouse_y = event.motion.y;
				break;
			case SDL_MOUSEBUTTONUP:
				switch (event.button.button) {
					case SDL_BUTTON_LEFT:
						button = 1;
						break;
					default:
						break;
				}
				break;
			case SDL_QUIT:
		                TTF_CloseFont(font);
		   
		                TTF_Quit();
                                SDL_Quit();
		   
				exit(1);
				break;
			default:
				break;
		}
	}
	if (!button)
		return;

	for (int i = 0; i < 4; i++) {
		if (*corners[i] && handle_corner(screen->w, screen->h, mouse_x, mouse_y, r2, i)) {
			fputs(corners[i], stdout);
			exit(0);
		}
	}

	for (int i = 0; i < num; i++) {
		if (rects[i].x <= mouse_x && mouse_x < (rects[i].x + rects[i].w) && rects[i].y <= mouse_y && mouse_y < (rects[i].y + rects[i].h)) {
			fputs(apps[i], stdout);
			exit(0);
		}
	}

}

SDL_Cursor *empty_cursor()
{
	static uint8_t null[32];
	return SDL_CreateCursor(null, null, 16, 16, 0, 0);
}

static int hide_cursor = 0;
static int timeout = 0;
static char *to_cmd = "true";
static char *p = NULL;
char *fn = "FreeMonoBold.ttf"; // default font

void init(int argc, char **argv)
{
	for (;;) {
		switch (getopt(argc, argv, "hct:d:p:f:")) {
			case 'h':
				fprintf(stderr, "usage: %s [-h] (show help) [-c] (hide cursor) [-t NUM] (timeout after NUM seconds) [-p 'Prompt'] [-f 'FontName.ttf'] [-d 'CMD'] (emit 'CMD' instead of 'true' on timeout)\n", argv[0]);
				exit(1);
				break;
			case 'c':
				hide_cursor = 1;
				break;
			case 't':
				timeout = atoi(optarg);
				break;
			case 'd':
				to_cmd = optarg;
				break;
			case 'p':
				p = optarg;
//		                fprintf(stderr, "prompt: %s\n", p);
				break;
			case 'f':
				fn = optarg;
//		                fprintf(stderr, "font file : %s\n", fn);
				break;
			case -1:
				return;
				break;
			default:
				fprintf(stderr, "use '-h' for help.\n");
				exit(1);
				break;
		}
	}
}

#define LENGTH 2048

int check_corner(char *out, char *in, char *which)
{
   if (in != strstr(in, which))
     return 0;
     
   int l = strnlen(which, LENGTH);
   
   char * s = in + l;
   
   while (*s == ' ') 
     s++;
   
   l = strnlen(s, LENGTH);
	   
   if( l == 0 )
     return 0;
	   
   strncpy(out, s, l+1);
// out[strnlen(out, LENGTH) - 1] = 0;
   return 1;
}

int main(int argc, char **argv)
{
	init(argc, argv);
   
	int max = 32;
	int num = 0;
	char imgs[max][LENGTH];
   
	char *apps[max];
	char corners[4][LENGTH];
   
	for (int i = 0; i < 4; i++)
		*corners[i] = 0;

	while ( (num < max) && fgets(imgs[num], LENGTH, stdin) )
        {
	   int l = strnlen(imgs[num], LENGTH);
	   
	   if( l == 0 ) continue;
	   
	   //	        if( l > LENGTH ) l = LENGTH;
	     
	   imgs[num][l - 1] = 0; // ? instead of EOL?
	       
	   char *delim = strchr(imgs[num], '#');
	   
	   if ( delim != NULL )
	     *delim = 0;
	   
	   char *start = imgs[num];
	   
	   while( *start == ' ' ) 
	     start++;

	   l = strnlen(start, l);
	   
	   if( l == 0 ) 
	     continue;
	   
	   if( *start == '@' )
	     {
		
		if (check_corner(corners[0], start, "@NW "))
		  continue;
		
		if (check_corner(corners[1], start, "@NE "))
		  continue;
		
		if (check_corner(corners[2], start, "@SW "))
		  continue;
		
		if (check_corner(corners[3], start, "@SE "))
		  continue;
		
		fprintf(stderr, "Warn: wrong corner in input line %d: '%s'\n", num+1, imgs[num]);
		continue;
	     }
	   
	   delim = strchr(start, ' '); // todo: single space!?
	   
	   while( (delim != NULL) && (*delim == ' ') ) 
	     delim++;
	   
	   if( delim == NULL )
	     {
		fprintf(stderr, "Warn: missing ' ' in the input line %d: '%s'\n", num+1, imgs[num]);
		continue;
	     } 
	   apps[num] = delim;
	   
	   *(delim-1) = 0;
	   
	   l = strnlen(start, l);
	   
	   for( ; delim != start ; delim-- )
	     if ( *delim == '_' )
	       *delim = ' ';
	   
	   strncpy(imgs[num], start, l+1);
	   
	   num++;
	}

	SDL_Init(SDL_INIT_VIDEO);
   
        if(TTF_Init() == -1) 
        {
	   fprintf(stderr, "TTF_Init: %s\n", TTF_GetError()) ;
//	   exit(2);
	}

	const SDL_VideoInfo *const info = SDL_GetVideoInfo();

	int w = info->current_w;
	int h = info->current_h;
	int r2 = (w * w + h * h) / 0x4000;

	SDL_Surface *screen = SDL_SetVideoMode(w, h, 32, SDL_SWSURFACE|SDL_FULLSCREEN);

	if (!screen)
		exit(1);
	if (screen->format->BytesPerPixel != 4)
		exit(1);
	if (screen->w != w || screen->h != h)
		exit(1);

	SDL_WM_SetCaption("Application Chooser", NULL ) ; // "appchoo");

	if (hide_cursor)
		SDL_SetCursor(empty_cursor());

        SDL_FillRect(screen , NULL , 0xFFFFFF);
        font = TTF_OpenFont(fn, 25); // todo ADD an option for font file name!
   
        if(!font) 
        {
	   fprintf(stderr, "TTF_OpenFont (%s): %s\n", fn, TTF_GetError());
	   // handle error
	}
   
  
	int num_x = 1;
	int num_y = 1;

	while (num > (num_x * num_y)) 
	{
		if (num_y < num_x)
			num_y++;
		else
			num_x++;
	}
	
	// max num_x/y : num_x * num_y <= num ?
	// todo: start with num_x = num_y = [sqrt(num)]... ?

	SDL_Rect rects[max];
	
	for (int i = 0; i < num; i++) 
	{
	    if( strnlen( imgs[i], LENGTH ) == 0 ) 
	      continue;
	      
	    SDL_Surface *image = NULL;
	    
	    if( *imgs[i] != '"' ) 
	    {
		image = IMG_Load(imgs[i]);

		if (!image) {
		    fprintf(stderr, "could not load \"%s\"\n", imgs[i]);
//			exit(1);
		}
	    }
		
    	    if(font && !image) 
    	    {
	
	       // get the width and height of a string as it would be rendered in a loaded font
/*	   
	   int ww,hh;
	   if(TTF_SizeText(font,imgs[i],&ww,&hh)) 
	     {
	      // perhaps print the current TTF_GetError(), the string can't be rendered...
	     } else 
	     {
//	        printf("width=%d height=%d\n",ww,hh);
	     }
*/
	   
	   char *start = imgs[i];
	       
	   while( *start == ' ' ) 
		 start++;
	       
	    if( *start == '"' )
            {
	      char *stop = ++start;
	       
	      while ( (*stop != 0) && (*stop != '"') )
		stop++;
	       
	      *stop = 0;
	    }
	       
	    
	      
	   // SDL_Color foregroundColor = { 0, 0, 0 }; SDL_Color backgroundColor = { 0, 0, 255 } ;
	   // TTF_RenderText_Shaded(font, "This is my text.", foregroundColor, backgroundColor);
	      
	   SDL_Color color={0,0,0,0};
	   SDL_Surface* textSurface =  TTF_RenderUTF8_Blended (font, start, color);
	  
	     if(!textSurface) 
	     {
		fprintf(stderr, "TTF_Render?_?: %s\n", TTF_GetError());
	     }
	     else
	     {
//		SDL_BlitSurface(textSurface,NULL,screen,NULL);
		
		// SDL_Surface* textSurface2 =  
		image = rotozoomSurface(textSurface, 45.0, 1.0, SMOOTHING_ON ) ;
		
		// Pass zero for width and height to draw the whole surface
//		SDL_Rect textLocation = { w >> 1, h >> 1, 0, 0 };
//		SDL_BlitSurface(textSurface2, NULL, screen, &textLocation);
        
		//     //perhaps we can reuse it, but I assume not for simplicity.

//		SDL_FreeSurface(textSurface2);
		SDL_FreeSurface(textSurface);
	     }
     }
		
    		

		SDL_Rect dest = screen->clip_rect;

		dest.w /= num_x;
		dest.h /= num_y;

		dest.x += (i % num_x) * dest.w;
		dest.y += ((i / num_x) % num_y) * dest.h;

		if( !image )
		{
		    /* Creating the surface. */
		    image = SDL_CreateRGBSurface(0, dest.w >> 1, dest.h >> 1, 32, 0, 0, 0, 0);
		
    		    /* Filling the surface with red color. */
		    SDL_FillRect(image, NULL, SDL_MapRGB(image->format, 200, 0, 0));
		}

		if (!fit_image(image, dest.w, dest.h))
		{
			
//		    	exit(1); // ???
		}
		

		SDL_Rect src = image->clip_rect;

		center_image(&dest, &src);
		
		SDL_SetAlpha( image, SDL_SRCALPHA, 128 );

		SDL_BlitSurface(image, &src, screen, &dest);
		SDL_FreeSurface(image);
		rects[i] = dest;
	}
	
	if( font && p != NULL )
	{
	   SDL_Color color={255,0,0,0};
	   SDL_Surface* textSurface = TTF_RenderUTF8_Blended (font, p, color);
	  
	     if(!textSurface) 
	     {
		fprintf(stderr, "TTF_Render?_?: %s\n", TTF_GetError());
	     } else
	     {
//                fprintf(stderr, "drawing prompt (%s)\n", p);
//		image = rotozoomSurface(textSurface, 0.0, 1.0, SMOOTHING_ON ) ;
		
		// Pass zero for width and height to draw the whole surface
		SDL_Rect dest = screen->clip_rect;
		SDL_Rect src = textSurface->clip_rect;

		dest.h = 30;
		dest.y = 0;
		
		center_image(&dest, &src);

		SDL_SetAlpha( textSurface, SDL_SRCALPHA, 128 );
		
//		SDL_BlitSurface(textSurface, NULL, screen, NULL ) ;// &src... &dest);
		SDL_BlitSurface(textSurface, &src, screen, &dest);
		
		SDL_FreeSurface(textSurface);
	     }	    
	}

	SDL_Flip(screen);

	for (;;) {
		if (timeout && (int)SDL_GetTicks() > (timeout * 1000)) {
			fputs(to_cmd, stdout);
			exit(0);
		}
		SDL_Delay(100);
		handle_events(screen, rects, apps, num, corners, r2);
	}
	return 0;
}

