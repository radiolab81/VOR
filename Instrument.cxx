#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include <errno.h>
#include <signal.h>
#include <string>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include <unistd.h>

using namespace std;

#define SKALE_HOEHE 488
#define SKALE_BREITE 488

#define BILDER_PRO_SEKUNDE 50

volatile float needle_rotation = 0;
volatile unsigned int heading = 0;

#define FROM false
#define TO   true
volatile bool b_from_to = TO;

volatile float frequency=110450000; // VOR-Frequenz in Hz

void set_receiver_freq() {
   char cmd[32] = "./xmlrpc_client.py ";
   //strcat(cmd,(std::to_string(frequency)).c_str());
   char buf[16];
   sprintf(buf,"%d", (int)frequency);
   strcat(cmd,buf);
   system(cmd);
}

void next_freq() {
       if (frequency < 117950000)
         frequency+=50000;

       set_receiver_freq();
}


void prev_freq() {
       if (frequency >= 108050000)
	 frequency-=50000;

       set_receiver_freq();
}


static void *socket_thread_vordata(void *arg) {
	int port = 1234;
	int r, n;
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	unsigned char buffer[512];
	struct sockaddr_in serv_addr, cli_addr;
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		perror("ERROR opening socket");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,  sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
	}

	bzero(buffer,5);

	fprintf (stderr, "Main socket started on UDP/%d \n", port);

        int i=0;
        float phasenversatz, temp = 0;
	while((n = read(sockfd,buffer,4)) != 0) {
		unsigned char b[] = {buffer[0], buffer[1], buffer[2], buffer[3]};
		memcpy(&phasenversatz, &b, sizeof(float));
		i++;
		temp=temp+phasenversatz;
		cout << phasenversatz << "\n";
		if (i==4) {
		  //needle_rotation=temp/i;
		  // run smoothly
		  if (needle_rotation < (temp/i+1))
			needle_rotation++;

		  if (needle_rotation > (temp/i-1))
			needle_rotation--;

		  temp=0;
	          i=0;
		}
	        fflush(stdout);

	}

	close(sockfd);
	return 0;
}


static void *socket_thread_control_instrument(void *arg) {
	int port = 1235;
	int r, n;
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	unsigned char buffer[512];
	struct sockaddr_in serv_addr, cli_addr;
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		perror("ERROR opening socket");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,  sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
	}

	bzero(buffer,5);

	fprintf (stderr, "Main socket started on UDP/%d \n", port);

	while((n = read(sockfd,buffer,4)) != 0) {
		printf("%s \n",buffer);
		fflush(stdout);

		if (strcmp((const char *)buffer,(const char *) "hdg+") == 0 ) {
			fprintf(stderr,"increment heading.\n");
		        if (heading<359)
			   heading++;
			else
			   heading=0;
		}

		if (strcmp((const char *)buffer,(const char *) "hdg-") == 0 ) {
			fprintf(stderr,"decrement heading.\n");
		        if (heading>0)
			   heading--;
			else
			   heading=359;
		}

		if (strcmp((const char *)buffer,(const char *) "tofr") == 0 ) {
			fprintf(stderr,"toggle to/from.\n");
		        b_from_to=!b_from_to;
		}

		if (strcmp((const char *)buffer,(const char *) "frq+") == 0 ) {
			fprintf(stderr,"frequency++.\n");
		        next_freq();
		}

		if (strcmp((const char *)buffer,(const char *) "frq-") == 0 ) {
			fprintf(stderr,"frequency--.\n");
		        prev_freq();
		}
	}

	close(sockfd);
	return 0;
}


int main(void) {
        system("fuser -k -n udp 1234");
        system("fuser -k -n udp 1235");
        system("fuser -k -n udp 7777");

// Interprozesskom herstellen
	pthread_t process_vordata;
	pthread_create(&process_vordata, NULL, socket_thread_vordata, NULL);
	pthread_detach(process_vordata);

	pthread_t process_control_instrument;
	pthread_create(&process_control_instrument, NULL, socket_thread_control_instrument, NULL);
	pthread_detach(process_control_instrument);

// SDL - Teil
	SDL_Event event;
	SDL_Renderer *renderer = NULL;
	SDL_Window *window = NULL;

   	SDL_Texture *textur_background = NULL;
    	SDL_Texture *textur_rosette = NULL;
        SDL_Texture *textur_needle = NULL;
	SDL_Texture *textur_aircraft = NULL;
	SDL_Texture *textur_marker = NULL;
	SDL_Texture *textur_to_indicator = NULL;
	SDL_Texture *textur_from_indicator = NULL;

	SDL_Rect rect_background;
	rect_background.x=0; rect_background.y=0;
	rect_background.w=488; rect_background.h=488;

	TTF_Font *font = NULL;
	TTF_Font *font2 = NULL;
	TTF_Font *font3 = NULL;

	SDL_Surface *surface_freq = NULL;
        SDL_Texture *textur_freq = NULL;
	SDL_Rect freq_rect;
	freq_rect.x = rect_background.w/4; freq_rect.y= rect_background.h/2-(rect_background.h*0.1);
	freq_rect.w = 80; freq_rect.h = 30;
	char freq[16];


	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Konnte SDL nicht initialisieren: %s", SDL_GetError());
		return 3;
	}

	if (SDL_CreateWindowAndRenderer(SKALE_BREITE, SKALE_HOEHE,  0, &window, &renderer)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window und Renderer konnten nicht erzeugt werden: %s", SDL_GetError());
		return 3;
	}


       // load support for the PNG image formats
        int flags=IMG_INIT_PNG;
        if(IMG_Init(flags)&flags != flags) {
           SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "PNG Unterstuetzung konnte nicht initialisiert werden: %s", IMG_GetError());
	   return 3;
        }

        textur_background = IMG_LoadTexture(renderer, "background.png");
        textur_rosette = IMG_LoadTexture(renderer, "rosette.png");
        textur_needle = IMG_LoadTexture(renderer, "needle.png");
	textur_aircraft = IMG_LoadTexture(renderer,"aircraft.png");
	textur_marker = IMG_LoadTexture(renderer, "marker.png");
	textur_to_indicator = IMG_LoadTexture(renderer, "to_indicator.png");
	textur_from_indicator = IMG_LoadTexture(renderer, "from_indicator.png");

        if ((textur_background == NULL) || (textur_rosette == NULL) || (textur_needle == NULL) ||
	    (textur_aircraft == NULL) || (textur_marker == NULL ) || (textur_to_indicator == NULL) ||
            (textur_from_indicator == NULL) ) {
    	   SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fehler beim Laden der Skalen und Zeiger Texturen: %s", IMG_GetError());
	   return 3;
        }

	if(TTF_Init()==-1) {
	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fehler in TTF_Init: %s", TTF_GetError());
		return 3;
	}

	font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 20);
	if(!font) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fehler in TTF_OpenFont: %s", TTF_GetError());
		return 3;
	}

	font2 = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 36);
	if(!font2) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fehler in TTF_OpenFont: %s", TTF_GetError());
		return 3;
	}

	font3 = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 72);
	if(!font3) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fehler in TTF_OpenFont: %s", TTF_GetError());
		return 3;
	}

	SDL_Color color = { 0, 0, 0 };
	SDL_Color color_white = { 255, 255, 255 };
	SDL_Color color_blue = { 72,61,139 };

// common_fps_init();

// Event-Schleife & Daemonbetrieb
	while (1) {

                SDL_SetRenderDrawColor(renderer,0,0,0,255);
                SDL_RenderClear(renderer);

		sprintf(freq, "%.2f MHz", frequency/1000000.0);
		surface_freq = TTF_RenderText_Solid(font3, freq, color_white);
		if(!surface_freq) {
		    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fehler in TTF_RenderText_Solid: %s", TTF_GetError());
		    return 3;
		}

	        textur_freq = SDL_CreateTextureFromSurface(renderer,surface_freq);
		if (textur_freq == NULL) {
		    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Fehler in CreateTextureFromSurface: %s", SDL_GetError());
		    return 3;
    		}

		SDL_RenderCopy(renderer, textur_background, NULL, &rect_background);
                SDL_RenderCopyEx(renderer, textur_rosette, NULL, &rect_background,heading,NULL,SDL_FLIP_NONE);
		SDL_RenderCopy(renderer, textur_marker, NULL, &rect_background);

		SDL_RenderCopy(renderer, textur_freq, NULL, &freq_rect);

		if (b_from_to == FROM) {
                  SDL_RenderCopy(renderer, textur_from_indicator, NULL, &rect_background);
		  SDL_RenderCopyEx(renderer, textur_needle, NULL, &rect_background,needle_rotation+heading,NULL,SDL_FLIP_NONE);
		} else {
		  SDL_RenderCopy(renderer, textur_to_indicator, NULL, &rect_background);
		  SDL_RenderCopyEx(renderer, textur_needle, NULL, &rect_background,needle_rotation+180.0+heading,NULL,SDL_FLIP_NONE);
		} //if (b_from_to) {


		SDL_RenderCopy(renderer, textur_aircraft, NULL, &rect_background);

// Aktualisiere Fenster
		SDL_RenderPresent(renderer);

// Eventbehandlung
		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
		break;
		else if (event.type == SDL_KEYDOWN) {
		  cout << "Tastendruck entdeckt:" << event.key.keysym.scancode << "\n";
		  if (event.key.keysym.scancode == SDL_SCANCODE_A)
		    if (heading<359)
		      heading++;
		    else
		      heading=0;

		  if (event.key.keysym.scancode == SDL_SCANCODE_S)
		    if (heading>0)
		      heading--;
		    else
		      heading=359;


		  if (event.key.keysym.scancode == SDL_SCANCODE_P)
		    next_freq();

 		  if (event.key.keysym.scancode == SDL_SCANCODE_O)
		    prev_freq();

		  if (event.key.keysym.scancode == SDL_SCANCODE_T)
		    b_from_to=!b_from_to;
		}
// Limit max. xxx fps
		SDL_Delay(1000/BILDER_PRO_SEKUNDE);

// Aufraeumarbeiten pro Frame
        if (textur_freq)
           SDL_DestroyTexture(textur_freq);

        if (surface_freq)
           SDL_FreeSurface(surface_freq);

	} // while(1)

// Aufraeumen und Ressourcen freigeben
        if (textur_background)
    	    SDL_DestroyTexture(textur_background);

        if (textur_rosette)
   	   SDL_DestroyTexture(textur_rosette);

        if (textur_needle)
           SDL_DestroyTexture(textur_needle);

	if (textur_aircraft)
	   SDL_DestroyTexture(textur_aircraft);

	if (textur_to_indicator)
	   SDL_DestroyTexture(textur_to_indicator);

	if (textur_from_indicator)
	   SDL_DestroyTexture(textur_from_indicator);

        if (textur_freq)
	   SDL_DestroyTexture(textur_freq);

	if (surface_freq)
	   SDL_FreeSurface(surface_freq);

	if (font)
		TTF_CloseFont(font);
	if (font2)
		TTF_CloseFont(font2);
	if (font3)
		TTF_CloseFont(font3);

	IMG_Quit();

	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);

	TTF_Quit();
	SDL_Quit();
	return EXIT_SUCCESS;
}
