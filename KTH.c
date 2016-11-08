#include <stdio.h>
#include <math.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

const float FPS = 60;
const int SCREEN_W = 1000;
const int SCREEN_H = 600;

// ---- Functions
void mvChase (float *hx, float *hy, float cx, float cy) {
	if (cx < *hx)
		*hx -= 0.7;
	else if (cx > *hx)
		*hx += 0.7;
	if (cy < *hy)
		*hy -= 0.7;
	else if (cy > *hy)
		*hy += 0.7;
}

void mvRunTeleport (float *hx, float *hy, float cx, float cy) {
	if (cx < *hx) *hx += 0.7;
	else if (cx > *hx) *hx -= 0.7;
	if (cy < *hy) *hy += 0.7;
	else if (cy > *hy) *hy -= 0.7;
	if (rand()%750 == 1 || *hx > 919 || *hx < 51) *hx = rand()%800; 
	if (rand()%750 == 2 || *hy > 469 || *hy < 51) *hy = rand()%400;
}

void mvProtector (float *hx, float *hy, float cx, float cy, float px, float py, int kh[2]) {
	int i=0;
	if(!kh[2])
		mvChase(hx, hy, cx, cy);
	else if ((cx - *hx < 120 && cx - *hx > 0) && (cy - *hy < 120 && cy - *hy > 0)) {
		*hx += 0.7;
		*hy += 0.7;
		i=1;
	}
	else if ((cx - *hx < 120 && cx - *hx > 0) && (*hy - cy < 120 && *hy - cy > 0)) {
		*hx += 0.7;
		*hy -= 0.7;
		i=1;
	}
	else if ((*hx - cx < 120 && *hx - cx > 0) && (cy - *hy < 120 && cy - *hy > 0)) {
		*hx -= 0.7;
		*hy += 0.7;
		i=1;
	}
	else if ((*hx - cx < 120 && *hx - cx > 0) && (*hy - cy < 120 && *hy - cy > 0)) {
		*hx -= 0.7;
		*hy -= 0.7;
		i=1;
	}
	else if (!i) {
		if (px+10 < *hx) *hx -= 0.7;
		else if (px-10 > *hx) *hx += 0.7;
		if (py+10 < *hy) *hy -= 0.7;
		else if (py-10 > *hy) *hy += 0.7;
	}
}

void Wall (float *px, float *py, float w, float h, float dh) {
	if (*px < 35) *px = 35;
	if (*px > (963 - w)) *px = 963 - w;
	if (*py < dh) *py = dh + 5;
	if (*py > (513 - h)) *py = 513 - h;
}

float xpToUp (float lvl, float *pts) {
	*pts += lvl + 300 * pow(2, lvl/7);
	return *pts/4;
}

void lvlUP (float *chp, int lvl, float *catk) {
	*chp = lvl * 15840 / 99;
	*catk = lvl * 200 / 99;
}

void resetHero (float *h1hp, float *h1bhp) {
	*h1hp = *h1bhp;
}

int Impact (float cx, float cy, float ch, float cw, float px, float py, float ph, float pw) {
	if (((cx + cw) > px && cx < (px + pw)) && ((cy + ch) > py && cy < (py + ph)))
		return 1;
	else
		return 0;
}

float gRnd(int n){
	int rnd=0;
	switch (n){
		case 0: rnd = (rand()%400)+70; break; //y
		case 1:	rnd = (rand()%800)+40; break; //x
		case 2: rnd = (rand()%100)+40; break; //x chest
	}
	return rnd;
}

void genDungeon(float *hero1x, float *hero2x, float *hero3x, float *hero1y, float *hero2y, float *hero3y, float *chestx, float *chesty){
	int n=gRnd(1); while(n>440&&n<550) {n=gRnd(1);} *hero1x = n;
	n=gRnd(0); while(n>450) {n=gRnd(0);} *hero1y = n;
	n=gRnd(1); while(n>440&&n<550) {n=gRnd(1);} *hero2x = n;
	n=gRnd(0); while(n>450) {n=gRnd(0);} *hero2y = n;
	n=gRnd(1); while(n>440&&n<550) {n=gRnd(1);} *hero3x = n;
	n=gRnd(0); while(n>450) {n=gRnd(0);} *hero3y = n;
	*chestx = gRnd(2);
	*chesty = gRnd(0);
}

void gLoot(int n, float *cxp, float *cgld, int *k){
	switch(n) {
		case 0: *cxp+=12; *cgld+=(rand()%5)+15; break; //hero 1
		case 1: *cxp+=15; *cgld+=(rand()%5)+25; break; //hero 2
		case 2: *cxp+=19; *cgld+=(rand()%5)+40; *k=1; break; //boss
		case 3: *cgld+=(rand()%10)+15; break; //chest
	}
}

int dmgCalc(int lvl, int base){
	return lvl*base/99;
}

void heal(float *chp, float basehp, int n){
	switch(n){
		case 1: *chp+=0.2*basehp; break; //potion peq +20%
		case 2: *chp+=0.4*basehp; break; //potion med +40%
		case 3: *chp+=0.85*basehp; break; //potion gra +85%
	}
}

void addInv(int item, int inv[12][3]){
	int i;
	if(item>=0&&item<=2) {
		for(i=9;i<12;i++) {
			if (inv[i][0] == item) {
				inv[i][1]++;
				i=12;
			}
			else if(inv[i][0] == -1) {
				inv[i][0]=item;
				inv[i][1]=1;
				i=12;
			}
		}
	}
	else {
		for(i=0;i<9;i++) {
			if (inv[i][0] == item) inv[i][1]++;
			else if(inv[i][0] == -1) {
				inv[i][0]=item;
				inv[i][1]=1;
				i=9;
			}
		}
	}
}

void useItem(int inv[12][3], int pos){
	inv[pos][1]--;
	int i, tam=12-pos, j;
	if(inv[pos][1] <= 0) {
		for(i=pos;i<12;i++) {
			for(j=0;j<3;j++) {
				inv[i][j]=inv[i+1][j];
				if(i==11) inv[i][j]=-1;
			}
		}
	}
}

int main(int argc, char **argv){
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
	ALLEGRO_SAMPLE *dgnMsc = NULL;
	ALLEGRO_SAMPLE_INSTANCE *dgnInstance = NULL;
	ALLEGRO_SAMPLE *btlMsc = NULL;
	ALLEGRO_SAMPLE_INSTANCE *btlInstance = NULL;
	ALLEGRO_SAMPLE *goMsc = NULL;
	ALLEGRO_SAMPLE_INSTANCE *goInstance = NULL;
	ALLEGRO_BITMAP *char1 = NULL;
	ALLEGRO_BITMAP *hero[3];
	ALLEGRO_BITMAP *bg = NULL;
	ALLEGRO_BITMAP *gameoverbg = NULL;
	ALLEGRO_BITMAP *map = NULL;
	ALLEGRO_BITMAP *head = NULL;
	ALLEGRO_BITMAP *battlebg = NULL;
	ALLEGRO_BITMAP *inventbg = NULL;
	ALLEGRO_BITMAP *itembg = NULL;
	ALLEGRO_BITMAP *statusbg = NULL;
	ALLEGRO_BITMAP *menubg = NULL;
	ALLEGRO_BITMAP *skull = NULL;
	ALLEGRO_BITMAP *exitbg = NULL;
	ALLEGRO_BITMAP *savepoint = NULL;
	ALLEGRO_BITMAP *chest = NULL;
	ALLEGRO_BITMAP *chestOpen = NULL;
	ALLEGRO_BITMAP *bsword = NULL;
	ALLEGRO_BITMAP *sword = NULL;
	ALLEGRO_BITMAP *shield = NULL;
	ALLEGRO_BITMAP *hppot[3];
	ALLEGRO_BITMAP *axe = NULL;
	ALLEGRO_BITMAP *door = NULL;
	ALLEGRO_BITMAP *dopen = NULL;
    bool redraw = true;
    float herow[3], heroh[3], herox[3], heroy[3], herohp[3], herobhp[3], heromxhp[3], heroatk[3], herogxp[3];
    FILE *entrada, *saida;

// ----- Init
	if(!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
    }
	timer = al_create_timer(1.0 / FPS);
    if(!timer) {
		fprintf(stderr, "failed to create timer!\n");
		return -1;
    }
	display = al_create_display(SCREEN_W, SCREEN_H);
    if(!display) {
		fprintf(stderr, "failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
    }
	
	al_init_image_addon();
	al_install_keyboard();
    al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_install_audio();
	al_init_acodec_addon();

	ALLEGRO_FONT *size_12 = al_load_font("Nordstern.ttf", 12, 1);
	al_reserve_samples(100);
	dgnMsc = al_load_sample("Sounds/Dungeon5.ogg");
	dgnInstance = al_create_sample_instance(dgnMsc);
	al_set_sample_instance_playmode(dgnInstance, ALLEGRO_PLAYMODE_LOOP);
	al_attach_sample_instance_to_mixer(dgnInstance, al_get_default_mixer());
	btlMsc = al_load_sample("Sounds/Battle7.ogg");
	btlInstance = al_create_sample_instance(btlMsc);
	al_set_sample_instance_playmode(btlInstance, ALLEGRO_PLAYMODE_LOOP);
	al_attach_sample_instance_to_mixer(btlInstance, al_get_default_mixer());
	goMsc = al_load_sample("Sounds/Theme2.ogg");
	goInstance = al_create_sample_instance(goMsc);
	al_set_sample_instance_playmode(goInstance, ALLEGRO_PLAYMODE_LOOP);
	al_attach_sample_instance_to_mixer(goInstance, al_get_default_mixer());

// ---- Bitmaps
	//Char: Orc;
    char1 = al_load_bitmap("images/char/char1up.png");
	if(!char1) {
      fprintf(stderr, "failed to create char1 bitmap!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
    }
    float char1_w = al_get_bitmap_width(char1), char1_h = al_get_bitmap_height(char1);
    float char1_x = 484, char1_y = SCREEN_H - 89 - char1_h;
	float char1_hp = 160, char1_atk = 2, char1_xp = 0, char1_gld = 0;
	//.

	//Heroes
	/* ---- Male1 */
	hero[0] = al_load_bitmap("images/char/hero1.png");
	if(!hero[0]) {
      fprintf(stderr, "failed to create hero1 bitmap!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
    }
    herow[0] = al_get_bitmap_width(hero[0]), heroh[0] = al_get_bitmap_height(hero[0]);
    heromxhp[0] = 1980, herohp[0] = 20, herobhp[0] = 20, heroatk[0] = 2;
	
	/* ---- Male2 */
	hero[1] = al_load_bitmap("images/char/hero2.png");
	if(!hero[1]) {
      fprintf(stderr, "failed to create hero2 bitmap!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
    }
    herow[1] = al_get_bitmap_width(hero[1]), heroh[1] = al_get_bitmap_height(hero[1]);
    heromxhp[1] = 2772, herohp[1] = 28, herobhp[1] = 28, heroatk[1] = 3;

	/* ---- Fem Boss1 */
	hero[2] = al_load_bitmap("images/char/hero3.png");
	if(!hero[2]) {
      fprintf(stderr, "failed to create hero3 bitmap!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
    }
    herow[2] = al_get_bitmap_width(hero[2]), heroh[2] = al_get_bitmap_height(hero[2]);
    heromxhp[2] = 5148, herohp[2] = 52, herobhp[2] = 52, heroatk[2] = 5;
	//.

	//Backgrounds
	bg = al_load_bitmap("images/bg.png");
	if(!bg) {
		fprintf(stderr, "failed to create bg bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    gameoverbg = al_load_bitmap("images/GameOver.png");
	if(!gameoverbg) {
		fprintf(stderr, "failed to create gameoverbg bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
	map = al_load_bitmap("images/map.png");
	if(!map) {
		fprintf(stderr, "failed to create map bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
	head = al_load_bitmap("images/head.png");
	if(!head) {
		fprintf(stderr, "failed to create head bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    battlebg = al_load_bitmap("images/battle.png");
	if(!battlebg) {
		fprintf(stderr, "failed to create battlebg bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    inventbg = al_load_bitmap("images/invent.png");
	if(!inventbg) {
		fprintf(stderr, "failed to create invent bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    itembg = al_load_bitmap("images/item.png");
	if(!itembg) {
		fprintf(stderr, "failed to create item bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    statusbg = al_load_bitmap("images/status.png");
	if(!statusbg) {
		fprintf(stderr, "failed to create status bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    menubg = al_load_bitmap("images/menubg.png");
	if(!menubg) {
		fprintf(stderr, "failed to create menu bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    skull = al_load_bitmap("images/skull.png");
	if(!skull) {
		fprintf(stderr, "failed to create menu bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    exitbg = al_load_bitmap("images/exitbg.png");
	if(!exitbg) {
		fprintf(stderr, "failed to create exit bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
	//.

	//Items
	bsword = al_load_bitmap("images/Items/basicsword.png");
    if(!bsword) {
		fprintf(stderr, "failed to create basicsword bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    int bsword_dmg[99];

    sword = al_load_bitmap("images/Items/sword.png");
    if(!sword) {
		fprintf(stderr, "failed to create sword bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    int sword_dmg[99];

    axe = al_load_bitmap("images/Items/axe.png");
    if(!axe) {
		fprintf(stderr, "failed to create axe bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    int axe_dmg[99];

    hppot[0] = al_load_bitmap("images/Items/hppot1.png");
    if(!hppot[0]) {
		fprintf(stderr, "failed to create hp1 bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }

    hppot[1] = al_load_bitmap("images/Items/hppot.png");
    if(!hppot[1]) {
		fprintf(stderr, "failed to create hp2 bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }

    hppot[2] = al_load_bitmap("images/Items/hppot3.png");
    if(!hppot[2]) {
		fprintf(stderr, "failed to create hp3 bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }

    shield = al_load_bitmap("images/Items/shield.png");
    if(!shield) {
		fprintf(stderr, "failed to create shield bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
	//.

	//Scene
	door = al_load_bitmap("images/scene/door.png");
	if(!door) {
		fprintf(stderr, "failed to create door bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
	float door_w = al_get_bitmap_width(door), door_h = al_get_bitmap_height(door);
    float door_x = 484, door_y = door_h;
    dopen = al_load_bitmap("images/scene/dopen.png");
	if(!dopen) {
		fprintf(stderr, "failed to create dopen bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }

	savepoint = al_load_bitmap("images/scene/save.png");
	if(!savepoint) {
		fprintf(stderr, "failed to create savepoint bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }

    chest = al_load_bitmap("images/scene/chest.png");
	if(!chest) {
		fprintf(stderr, "failed to create chest bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
	float chest_w = al_get_bitmap_width(chest), chest_h = al_get_bitmap_height(chest);
    float chest_x, chest_y;
	chestOpen = al_load_bitmap("images/scene/chestOpen.png");
	if(!chestOpen) {
		fprintf(stderr, "failed to create chestOpen bitmap!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }
    //.

// ---- Event	
	event_queue = al_create_event_queue();
    if(!event_queue) {
		fprintf(stderr, "failed to create event_queue!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());  
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_flip_display();
	al_start_timer(timer);

// ---- Variáveis de base	
	char hp[15], gold[10], XP[15], atkEn[10], hpEn[10], nvl[10], atk[10], qtd[6], desc[20], desc2[20], bt[20], bt2[20];
	bool e_bsword=false, e_sword=false, e_axe=false;
	float points=0, basehp = char1_hp;
	int gchest = 1, key = 0, i, j, weapLvl=0, aux, aux2;
	int invOpen=0, stsOpen=0, itemOpen = -1, mapOpen = 0, headPos = 2, txtPos, menuOpen = 0, itemMenu = 1, exitOpen = 0;
	int battle = 0, enImp, turn_pl = 0, turn_en = 1, kh[3];
	int gameOver = 0, playing = 1, lvl = 1, mvW = 0, mvA = 0, mvS = 0, mvD = 0;
	int msgSave=0;
	int inv[12][3]; int invPosx[12]; int invPosy[12]; 

	// seta todos os herois como vivos (kh = 1)
	for(i=0;i<3;i++) kh[i]=1;
	// gera o primeiro max xp
	int mxpc = 1;
	float maxXP;
	if (mxpc) { 
		maxXP = xpToUp((lvl+1), &points);
		mxpc++;
	}
	// gera primeira dungeon
	srand((unsigned)time(NULL));
	genDungeon(&herox[0], &herox[1], &herox[2], &heroy[0], &heroy[1], &heroy[2], &chest_x, &chest_y);
	int porcHP = 227*char1_hp/basehp, porcXP = 227*char1_xp/maxXP;
	// preenche vetores de itens, invent e invent posição
	for(i=1;i<100;i++){
		bsword_dmg[i]=dmgCalc(i, 200);
		sword_dmg[i]=dmgCalc(i, 300);
		axe_dmg[i]=dmgCalc(i, 400);
	}
	for(i=0;i<12;i++){
		for(j=0;j<3;j++){ inv[i][j] = -1; }
		if(i>=0&&i<=2||i==9) invPosy[i]=234;
		else if(i>=3&&i<=5||i==10) invPosy[i]=313;
		else invPosy[i]=392;
		if(i==0||i==3||i==6) invPosx[i]=368;
		else if(i==1||i==4||i==7) invPosx[i]=451;
		else if(i==2||i==5||i==8) invPosx[i]=527;
		else invPosx[i]=606;
	}
	for(i=3;i<6;i++)
		addInv(i, inv);
	for(i=0;i<3;i++) {
		inv[i][2]=1;
		addInv(i, inv);
	}
	//....
	// arquivo
	entrada = fopen("save.txt", "r");
	if(entrada){
		fscanf(entrada, "%f,", &char1_x);
		fscanf(entrada, "%f,", &char1_y);
		fscanf(entrada, "%f,", &char1_hp);
		fscanf(entrada, "%f,", &char1_xp);
		fscanf(entrada, "%f,", &char1_gld);
		fscanf(entrada, "%f,", &basehp);
		for(i=0;i<3;i++) {
			fscanf(entrada, "%f,", &herox[i]);
			fscanf(entrada, "%f,", &heroy[i]);
			fscanf(entrada, "%d,", &kh[i]);
		}
		for(i=0;i<12;i++) {
			for(j=0;j<3;j++) {
				fscanf(entrada, "%d,", &inv[i][j]);
			}
		}
		fscanf(entrada, "%f,", &chest_x);
		fscanf(entrada, "%f,", &chest_y);
		fscanf(entrada, "%d,", &lvl);
		fscanf(entrada, "%d,", &gchest);
		fscanf(entrada, "%d,", &key);
		
	}
	fclose(entrada);
	// musica
	al_play_sample_instance(dgnInstance);
	//.

// ---- Jogo
		while (playing) {
			ALLEGRO_EVENT ev;
			al_wait_for_event(event_queue, &ev);
			// eventos allegro
			if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				exitOpen=1;
			}
			else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
				if (exitOpen) {
					switch(ev.keyboard.keycode) {
						case ALLEGRO_KEY_ENTER:
								playing=0;

							break;
						case ALLEGRO_KEY_ESCAPE:
							exitOpen=0;
							break;
					}
				}
				switch(ev.keyboard.keycode) {
					case ALLEGRO_KEY_ESCAPE:
						if(itemOpen>0 && itemOpen<=12) itemOpen=-1;
						else if(invOpen) invOpen=0;
						else if(stsOpen) stsOpen=0;
						else if(mapOpen) {
							mapOpen=0;
							headPos=2;
						}
						else if (menuOpen) {
							menuOpen=0;
							itemMenu=1;
						}
						break;

					// abrir menu
					case ALLEGRO_KEY_Q:
						if (menuOpen) {
							menuOpen=0;
							itemMenu=1;
						}
						else menuOpen=1;
						stsOpen=0;
						mapOpen=0;
						invOpen=0;
						break;
					//.

					// abrir inventário
					case ALLEGRO_KEY_I:
						if (invOpen) invOpen=0;
						else invOpen=1;
						stsOpen=0;
						mapOpen=0;
						menuOpen=0;
						break;
					//.

					// abrir status
					case ALLEGRO_KEY_C:
						if(stsOpen) stsOpen=0;
						else stsOpen=1;
						invOpen=0;
						mapOpen=0;
						menuOpen=0;
						break;
					//.

					// abrir mapa
					case ALLEGRO_KEY_M:
						if(mapOpen) {
							mapOpen=0;
							headPos=2;
						}
						else mapOpen=1;
						invOpen=0;
						stsOpen=0;
						menuOpen=0;
						break;
					//.
				}

				if (menuOpen) {
					switch(ev.keyboard.keycode) {
						case ALLEGRO_KEY_UP:
							if(itemMenu>1)
								itemMenu--;
							break;

						case ALLEGRO_KEY_DOWN:
							if(itemMenu<3)
								itemMenu++;
							break;

						case ALLEGRO_KEY_ENTER:
							switch(itemMenu) {
								case 1: menuOpen=0; break;
								case 2: 
								case 3: exitOpen=1; break;
							}
							break;
					}
				}

				if (mapOpen) {
					switch(ev.keyboard.keycode) {
						case ALLEGRO_KEY_RIGHT:
							if(headPos<3)
								headPos++;
							break;

						case ALLEGRO_KEY_LEFT:
							if(headPos>1)
								headPos--;
							break;
					}
				}

				if (invOpen) {
					if(itemOpen<1||itemOpen>12) {
						switch(ev.keyboard.keycode) {
							case ALLEGRO_KEY_1: itemOpen=1; break;
							case ALLEGRO_KEY_2: itemOpen=2; break;
							case ALLEGRO_KEY_3: itemOpen=3; break;
							case ALLEGRO_KEY_4: itemOpen=4; break;
							case ALLEGRO_KEY_5: itemOpen=5; break;
							case ALLEGRO_KEY_6: itemOpen=6; break;
							case ALLEGRO_KEY_7: itemOpen=7; break;
							case ALLEGRO_KEY_8: itemOpen=8; break;
							case ALLEGRO_KEY_9: itemOpen=9; break;
							case ALLEGRO_KEY_A: itemOpen=10; break;
							case ALLEGRO_KEY_B: itemOpen=11; break;
							case ALLEGRO_KEY_C: itemOpen=12; break;
						}
					}
					else if (itemOpen>0 && itemOpen<=12){
						switch(ev.keyboard.keycode) {
							case ALLEGRO_KEY_ENTER:
								if(inv[itemOpen-1][0]>=0&&inv[itemOpen-1][0]<3) {
									useItem(inv, itemOpen-1);
									heal(&char1_hp, basehp, (inv[itemOpen-1][0]+1));
									itemOpen=-1;
								}
								else if(inv[itemOpen-1][0]>=3&&inv[itemOpen-1][0]<7) {
									switch(inv[itemOpen-1][0]) {
										case 3:
											if(e_bsword) e_bsword = false;
											else {
												e_bsword = true;
												e_sword = false;
												e_axe = false;
											}
											break;
										case 4:
											if(e_sword) e_sword=false;
											else {
												e_bsword = false;
												e_sword = true;
												e_axe = false;
											}
											break;
										case 5:
											if(e_axe) e_axe=false;
											else {
												e_bsword = false;
												e_sword = false;
												e_axe = true;
											}
											break;
									}
									weapLvl=inv[itemOpen-1][2];
									itemOpen=-1;
								}
							break;
						}
					}

				}
				if (!battle && !invOpen && !stsOpen && !mapOpen && !menuOpen && !exitOpen) {
					switch(ev.keyboard.keycode) {
						// declaração do começo do movimento do char
						case ALLEGRO_KEY_W:
						case ALLEGRO_KEY_UP:
							mvW=1;
							break;
							
						case ALLEGRO_KEY_S:
						case ALLEGRO_KEY_DOWN:
							mvS=1;
							break;
							
						case ALLEGRO_KEY_A:
						case ALLEGRO_KEY_LEFT:
							mvA=1;
							break;
							
						case ALLEGRO_KEY_D:
						case ALLEGRO_KEY_RIGHT:
							mvD=1;
							break;
						//.
						// save
						case ALLEGRO_KEY_P:
							if(Impact(char1_x, char1_y, char1_h, char1_w, 900, 50, 55, 100) == 1) {
								saida = fopen("save.txt", "w");
								fprintf(saida, "%f,", char1_x);
								fprintf(saida, "%f,", char1_y);
								fprintf(saida, "%f,", char1_hp);
								fprintf(saida, "%f,", char1_xp);
								fprintf(saida, "%f,", char1_gld);
								fprintf(saida, "%f,", basehp);
								for(i=0;i<3;i++) {
									fprintf(saida, "%f,", herox[i]);
									fprintf(saida, "%f,", heroy[i]);
									fprintf(saida, "%d,", kh[i]);
								}
								for(i=0;i<12;i++) {
									for(j=0;j<3;j++) {
										fprintf(saida, "%d,", inv[i][j]);
									}
								}
								fprintf(saida, "%f,", chest_x);
								fprintf(saida, "%f,", chest_y);
								fprintf(saida, "%d,", lvl);
								fprintf(saida, "%d,", gchest);
								fprintf(saida, "%d,", key);

								fclose(saida);
								msgSave=100;
							}
						//.
					}
				}
				// turn do char com opções possíveis em cada tecla
				if (turn_pl && !invOpen) {
					switch(ev.keyboard.keycode) {
					case ALLEGRO_KEY_1: //soco
						herohp[enImp] -= char1_atk;
						turn_en = 1;
						turn_pl = 0;
						break;

					case ALLEGRO_KEY_2:
						if(e_bsword) { //espada básica
							herohp[enImp] -= (rand()%2)+bsword_dmg[lvl];
							turn_en = 1;
							turn_pl = 0;
						}
						else if(e_sword) { //espada
							herohp[enImp] -= (rand()%2)+sword_dmg[lvl];
							turn_en = 1;
							turn_pl = 0;
						}
						else if(e_axe) { //machado
							herohp[enImp] -= (rand()%2)+axe_dmg[lvl];
							turn_en = 1;
							turn_pl = 0;
						}
						break;
					}
				}
				//.

				// evento de abrir a chest
				if (gchest && Impact(char1_x, char1_y, char1_h, char1_w, chest_x, chest_y, chest_h, chest_w) == 1) {
					switch(ev.keyboard.keycode) {
					case ALLEGRO_KEY_E:
						gLoot(3, &char1_xp, &char1_gld, &key);
						if(rand()%35==1) addInv(2, inv);
						else if(rand()%15==1) addInv(1, inv);
						else if(rand()%5==1) addInv(0, inv);
						gchest = 0;
						break;
					}
				}
				//.
				redraw = true;
			}
			
			else if (ev.type == ALLEGRO_EVENT_KEY_UP){
				// declaração do fim dos movimentos do char
				switch(ev.keyboard.keycode) {
					case ALLEGRO_KEY_W:
					case ALLEGRO_KEY_UP:
						mvW=0;
						break;
						
					case ALLEGRO_KEY_S:
					case ALLEGRO_KEY_DOWN:
						mvS=0;
						break;
						
					case ALLEGRO_KEY_A:
					case ALLEGRO_KEY_LEFT:
						mvA=0;
						break;
						
					case ALLEGRO_KEY_D:
					case ALLEGRO_KEY_RIGHT:
						mvD=0;
						break;
				}
				//.
			}

			else if (ev.type == ALLEGRO_EVENT_TIMER) {
				// movimentos de inimigos
				if (!battle && !invOpen && !stsOpen && !mapOpen && !menuOpen && !exitOpen) {
					mvChase (&herox[0], &heroy[0], char1_x, char1_y);
					mvRunTeleport (&herox[2], &heroy[2], char1_x, char1_y);
					mvProtector (&herox[1], &heroy[1], char1_x, char1_y, herox[2], heroy[2], kh);
				}
				//.

				// cálculo das porcentagens de hp e xp pra preencher as barras de status
					porcHP = 227*char1_hp/basehp; porcXP = 227*char1_xp/maxXP;
				//.

				// decrementa o contador das mensagens
					msgSave--;
				// .
	        }
	        //.
			
			// movimentação do char
			if(!battle && !invOpen && !stsOpen && !mapOpen && !menuOpen && !exitOpen) {
	        	if (mvW) { 
	        		char1_y -= 1.2;
	        		al_destroy_bitmap(char1);
	        		char1 = al_load_bitmap("images/char/char1up.png");
	        	}
	       		if (mvA) {
	       			char1_x -= 1.2;
	       			al_destroy_bitmap(char1);
	       			char1 = al_load_bitmap("images/char/char1l.png");
	       		}
	        	if (mvS) {
	        		char1_y += 1.2;
	        		al_destroy_bitmap(char1);
	        		char1 = al_load_bitmap("images/char/char1.png");
	        	}
	        	if (mvD) {
	        		char1_x += 1.2;
	        		al_destroy_bitmap(char1);
	        		char1 = al_load_bitmap("images/char/char1r.png");
	        	}
	        }	        //.

			// paredes da dungeon
			Wall (&char1_x, &char1_y, char1_w, char1_h, door_h);
			Wall (&herox[0], &heroy[0], herow[0], heroh[0], door_h);
			Wall (&herox[1], &heroy[1], herow[1], heroh[1], door_h);
			Wall (&herox[2], &heroy[2], herow[2], heroh[2], door_h);
			// porta/mudança de dungeon
			if ((char1_x > 480 && char1_x < (door_x + char1_w)) && (char1_y < (door_y + door_h)) && key) {
				genDungeon(&herox[0], &herox[1], &herox[2], &heroy[0], &heroy[1], &heroy[2], &chest_x, &chest_y);
				for(i=0;i<3;i++) {
					resetHero (&herohp[i], &herobhp[i]);
					kh[i] = 1;
				}
				char1_y = 484;
				key=0;
				gchest = 1;
			}
			//..

			// upar level
			if (char1_xp >= maxXP) {
				lvl++;
				lvlUP(&char1_hp, lvl, &char1_atk);
				for(i=0;i<3;i++) 
					lvlUP(&herohp[i], lvl, &heroatk[i]);
				maxXP = xpToUp(lvl, &points);
				basehp=char1_hp;
			}
			//.

			// teste de impacto com cada um dos heroes. não há impacto com mais de um hero por vez
			if(!battle) {
				for (i=0;i<3;i++) {
					if(Impact(char1_x, char1_y, char1_h, char1_w, herox[i], heroy[i], heroh[i], herow[i]) && !battle && kh[i]){
						enImp = i;
						battle = 1;
					}
				}
			}
			//.
			
			if (battle) {
				al_stop_sample_instance(dgnInstance);
				al_play_sample_instance(btlInstance);
				// turn do hero
				if (turn_en) {
					char1_hp -= heroatk[enImp];
					turn_en = 0;
					turn_pl = 1;
					if (char1_hp <= 0) { playing = 0; battle = 0; gameOver = 1; }
				}
				// morte do hero
				if (herohp[enImp] <= 0) {
					al_stop_sample_instance(btlInstance);
					al_play_sample_instance(dgnInstance);
					battle = 0;
					kh[enImp] = 0;
					gLoot(enImp, &char1_xp, &char1_gld, &key);
				}
				//..
			}
			if (redraw && al_is_event_queue_empty(event_queue)) {
				al_clear_to_color(al_map_rgb(0,0,0));
				al_draw_bitmap(bg, 0, 0, 0);
				if(key) al_draw_bitmap(dopen, door_x, door_y, 0);
				else al_draw_bitmap(door, door_x, door_y, 0);
				sprintf(hp, "%.f / %.f", char1_hp, basehp);
				al_draw_text(size_12, al_map_rgb(0, 0, 0), 60, 9, 0, hp);
				sprintf(gold, "%.f", char1_gld);
				al_draw_text(size_12, al_map_rgb(0, 0, 0), 208, 9, 0, gold);
				sprintf(XP, "%.f / %.f", char1_xp, maxXP);
				al_draw_text(size_12, al_map_rgb(0, 0, 0), 355, 9, 0, XP);

				if(gchest) al_draw_bitmap(chest, chest_x, chest_y, 0);
				else al_draw_bitmap(chestOpen, chest_x, chest_y, 0);
				if (Impact(char1_x, char1_y, char1_h, char1_w, chest_x, chest_y, chest_h, chest_w) == 1)
					al_draw_text(size_12, al_map_rgb(180, 0, 0), char1_x-19, char1_y-20, 0, "pressione [e]");
				al_draw_bitmap(savepoint, 900, 50, 0);
				if(Impact(char1_x, char1_y, char1_h, char1_w, 900, 50, 55, 100) == 1) {
					al_draw_text(size_12, al_map_rgb(180, 0, 0), char1_x, char1_y-20, 0, "salvar");
				}

				al_draw_bitmap(char1, char1_x, char1_y, 0);
				for(i=0;i<3;i++) {
					if (kh[i])
						al_draw_bitmap(hero[i], herox[i], heroy[i], 0);
				}

				if (msgSave>0)
					al_draw_text(size_12, al_map_rgb(180, 0, 0), 460, 550, 0, "** Jogo Salvo! **");

				if (battle) {
					al_draw_bitmap(battlebg, 0, 0, 0);
					al_draw_bitmap(hero[enImp], (SCREEN_W/2-heroh[enImp]/2), 255, 0);
					sprintf(hpEn, "%.f", herohp[enImp]);
					al_draw_text(size_12, al_map_rgb(0, 0, 0), 430, 306, 0, hpEn);
					sprintf(atkEn, "%.f", heroatk[enImp]);
					al_draw_text(size_12, al_map_rgb(0, 0, 0), 553, 306, 0, atkEn);
					sprintf(bt, "[1] soco (%.f de dano)", char1_atk);
					al_draw_text(size_12, al_map_rgb(255, 255, 255), 400, 368, 0, bt);
					if(e_bsword)
						sprintf(bt2, "[2] arma equipada (-%dhp)", bsword_dmg[lvl]);
					else if(e_sword)
						sprintf(bt2, "[2] arma equipada (-%dhp)", sword_dmg[lvl]);
					else if(e_axe)
						sprintf(bt2, "[2] arma equipada (-%dhp)", axe_dmg[lvl]);
					else
						sprintf(bt2, "[2] nada equipado (-0hp)");
					al_draw_text(size_12, al_map_rgb(255, 255, 255), 400, 395, 0, bt2);
				}

				if(invOpen) {
					al_draw_bitmap(inventbg, 0, 0, 0);
					for(i=0;i<12;i++) {
						switch(inv[i][0]) {
							case 0:
								al_draw_bitmap(hppot[0], invPosx[i], invPosy[i], 0);
								sprintf(qtd, "x%d", inv[i][1]);
								al_draw_text(size_12, al_map_rgb(180, 0, 0), invPosx[i]+20, invPosy[i]+30, 0, qtd);
								break;
							case 1:
								al_draw_bitmap(hppot[1], invPosx[i], invPosy[i], 0);
								sprintf(qtd, "x%d", inv[i][1]);
								al_draw_text(size_12, al_map_rgb(180, 0, 0), invPosx[i]+20, invPosy[i]+30, 0, qtd);
								break;
							case 2:
								al_draw_bitmap(hppot[2], invPosx[i], invPosy[i], 0);
								sprintf(qtd, "x%d", inv[i][1]);
								al_draw_text(size_12, al_map_rgb(180, 0, 0), invPosx[i]+20, invPosy[i]+30, 0, qtd);
								break;
							case 3:
								al_draw_bitmap(bsword, invPosx[i], invPosy[i], 0);
								sprintf(qtd, "x%d", inv[i][1]);
								al_draw_text(size_12, al_map_rgb(180, 0, 0), invPosx[i]+20, invPosy[i]+30, 0, qtd);
								break;
							case 4:
								al_draw_bitmap(sword, invPosx[i], invPosy[i], 0);
								sprintf(qtd, "x%d", inv[i][1]);
								al_draw_text(size_12, al_map_rgb(180, 0, 0), invPosx[i]+20, invPosy[i]+30, 0, qtd);
								break;
							case 5:
								al_draw_bitmap(axe, invPosx[i], invPosy[i], 0);
								sprintf(qtd, "x%d", inv[i][1]);
								al_draw_text(size_12, al_map_rgb(180, 0, 0), invPosx[i]+20, invPosy[i]+30, 0, qtd);
								break;
							case 6:
								al_draw_bitmap(shield, invPosx[i], invPosy[i], 0);
								sprintf(qtd, "x%d", inv[i][1]);
								al_draw_text(size_12, al_map_rgb(180, 0, 0), invPosx[i]+20, invPosy[i]+30, 0, qtd);
								break;
							default: break;
						}
					}
					if(itemOpen>=1&&itemOpen<=12){
						if(inv[itemOpen-1][0]>=0 && inv[itemOpen-1][0] <=6)
							al_draw_bitmap(itembg, 0, 0, 0);
						else
							itemOpen=-1;
						switch(inv[itemOpen-1][0]) {
							case 0:
								al_draw_bitmap(hppot[0], 406, 284, 0);
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 420, 238, 0, "Potion de HP Pequena");
								sprintf(desc, "Aumenta o HP em %.f", (0.2*basehp));
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 442, 282, 0, desc);
								sprintf(desc2, "Qt.: %d", inv[itemOpen-1][1]);
								al_draw_text(size_12, al_map_rgb(128, 0, 0), 442, 296, 0, desc2);
								al_draw_text(size_12, al_map_rgb(69, 69, 37), 415, 350, 0, "[ENTER] usar | [ESC] sair");
								break;
							case 1: al_draw_bitmap(hppot[1], 405, 282, 0);
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 433, 238, 0, "Potion de HP Media");
								sprintf(desc, "Aumenta o HP em %.f", (0.4*basehp));
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 440, 280, 0, desc);
								sprintf(desc2, "Qt.: %d", inv[itemOpen-1][1]);
								al_draw_text(size_12, al_map_rgb(128, 0, 0), 440, 294, 0, desc2);
								al_draw_text(size_12, al_map_rgb(69, 69, 37), 415, 350, 0, "[ENTER] usar | [ESC] sair");
								break;
							case 2: al_draw_bitmap(hppot[2], 406, 283, 0);
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 433, 238, 0, "Potion de HP Grande");
								sprintf(desc, "Aumenta o HP em %.f", (0.85*basehp));
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 438, 281, 0, desc);
								sprintf(desc2, "Qt.: %d", inv[itemOpen-1][1]);
								al_draw_text(size_12, al_map_rgb(128, 0, 0), 438, 295, 0, desc2);
								al_draw_text(size_12, al_map_rgb(69, 69, 37), 415, 350, 0, "[ENTER] usar | [ESC] sair");
								break;
							case 3: al_draw_bitmap(bsword, 408, 286, 0);
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 398, 238, 0, "Espada de Aprendiz - uma mao");
								sprintf(desc, "Nivel %d", inv[itemOpen-1][2]);
								al_draw_text(size_12, al_map_rgb(128, 0, 0), 443, 280, 0, desc);
								sprintf(desc2, "Da %d de dano", bsword_dmg[inv[itemOpen-1][2]]);
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 443, 294, 0, desc2);
								if(e_bsword) {
									al_draw_text(size_12, al_map_rgb(140, 0, 0), 443, 312, 0, "* equipado *");	
									al_draw_text(size_12, al_map_rgb(69, 69, 37), 395, 350, 0, "[ENTER] desequipar | [ESC] sair");	
								}
								else
									al_draw_text(size_12, al_map_rgb(69, 69, 37), 408, 350, 0, "[ENTER] equipar | [ESC] sair");
								break;
							case 4: al_draw_bitmap(sword, 403, 285, 0);
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 404, 238, 0, "Espada Superior - uma mao");
								sprintf(desc, "Nivel %d", inv[itemOpen-1][2]);
								al_draw_text(size_12, al_map_rgb(128, 0, 0), 438, 280, 0, desc);
								sprintf(desc2, "Da %d de dano", sword_dmg[inv[itemOpen-1][2]]);
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 438, 294, 0, desc2);
								if(e_sword) {
									al_draw_text(size_12, al_map_rgb(140, 0, 0), 443, 312, 0, "* equipado *");	
									al_draw_text(size_12, al_map_rgb(69, 69, 37), 395, 350, 0, "[ENTER] desequipar | [ESC] sair");	
								}
								else
									al_draw_text(size_12, al_map_rgb(69, 69, 37), 408, 350, 0, "[ENTER] equipar | [ESC] sair");
								break;
							case 5: al_draw_bitmap(axe, 403, 283, 0);
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 403, 238, 0, "Machado de Orc - duas maos");
								sprintf(desc, "Nivel %d", inv[itemOpen-1][2]);
								al_draw_text(size_12, al_map_rgb(128, 0, 0), 438, 280, 0, desc);
								sprintf(desc2, "Da %d de dano", axe_dmg[inv[itemOpen-1][2]]);
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 438, 294, 0, desc2);
								if(e_axe) {
									al_draw_text(size_12, al_map_rgb(140, 0, 0), 443, 312, 0, "* equipado *");	
									al_draw_text(size_12, al_map_rgb(69, 69, 37), 395, 350, 0, "[ENTER] desequipar | [ESC] sair");	
								}
								else
									al_draw_text(size_12, al_map_rgb(69, 69, 37), 408, 350, 0, "[ENTER] equipar | [ESC] sair");
								break;
							case 6: al_draw_bitmap(shield, 401, 283, 0);
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 398, 238, 0, "Escudo de Carvalho - uma mao");
								sprintf(desc, "Nivel %d", inv[itemOpen-1][2]);
								al_draw_text(size_12, al_map_rgb(128, 0, 0), 438, 280, 0, desc);
								sprintf(desc2, "Aumenta a def em x");
								al_draw_text(size_12, al_map_rgb(12, 32, 9), 438, 294, 0, desc2);
								al_draw_text(size_12, al_map_rgb(69, 69, 37), 408, 350, 0, "[ENTER] equipar | [ESC] sair");
								break;
							default: break;
						}
					}
				}

				if(stsOpen) {
					al_draw_bitmap(statusbg, 0, 0, 0);
					sprintf(nvl, "nível %d", lvl);
					al_draw_text(size_12, al_map_rgb(0, 0, 0), 451, 201, 0, nvl);
					al_draw_filled_rectangle(442, 237, 442+porcHP, 256, al_map_rgb(185,25,25));
					sprintf(hp, "%.f / %.f", char1_hp, basehp);
					al_draw_text(size_12, al_map_rgb(0, 0, 0), 565, 240, 0, hp);
					al_draw_filled_rectangle(442, 278, 442+porcXP, 297, al_map_rgb(25,160,180));
					sprintf(XP, "%.f / %.f", char1_xp, maxXP);
					al_draw_text(size_12, al_map_rgb(0, 0, 0), 565, 282, 0, XP);
					sprintf(atk, "%.f", char1_atk);
					al_draw_text(size_12, al_map_rgb(0, 0, 0), 441, 322, 0, atk);
				}

				if(mapOpen) {
					al_draw_bitmap(map, 0, 0, 0);
					switch(headPos) {
						case 1:
							al_draw_bitmap(head, 407, 340, 0);
							sprintf(desc, "Mercado da Cidade");
							txtPos = 435;
							break;
						case 2:
							al_draw_bitmap(head, 500, 212, 0);
							sprintf(desc, "Caminho para o campo de Batalha");
							txtPos = 385;
							break;
						case 3:
							al_draw_bitmap(head, 562, 274, 0);
							sprintf(desc, "Centro de Resistencia");
							txtPos = 425;
							break;
					}
					al_draw_text(size_12, al_map_rgb(180, 0, 0), txtPos, 420, 0, desc);
				}

				if(menuOpen) {
					al_draw_bitmap(menubg, 0, 0, 0);
					switch (itemMenu) {
						case 1: al_draw_bitmap(skull, 222, 258, 0); break;
						case 2: al_draw_bitmap(skull, 222, 360, 0); break;
						case 3: al_draw_bitmap(skull, 222, 475, 0); break;
					}
				}

				if(exitOpen) {
					al_draw_bitmap(exitbg, 0, 0, 0);
				}
				al_flip_display();
			}
		} //end_of_while

	al_stop_sample_instance(dgnInstance);
	al_stop_sample_instance(btlInstance);
	
	if(gameOver) {
		al_play_sample_instance(goInstance);
		al_clear_to_color(al_map_rgb(0,0,0));
		al_draw_bitmap(gameoverbg, 0, 0, 0);
		al_flip_display();
		al_rest(6);
	}
	al_stop_sample_instance(goInstance);
	int a=0;
    al_destroy_bitmap(char1);
    for(a=0;a<3;a++)
    	al_destroy_bitmap(hero[a]);
    al_destroy_bitmap(bg);
    al_destroy_bitmap(gameoverbg);
    al_destroy_bitmap(map);
    al_destroy_bitmap(head);
    al_destroy_bitmap(battlebg);
    al_destroy_bitmap(inventbg);
    al_destroy_bitmap(itembg);
    al_destroy_bitmap(statusbg);
    al_destroy_bitmap(menubg);
    al_destroy_bitmap(skull);
    al_destroy_bitmap(exitbg);
    al_destroy_bitmap(savepoint);
    al_destroy_bitmap(chest);
    al_destroy_bitmap(chestOpen);
    al_destroy_bitmap(door);
    al_destroy_bitmap(bsword);
    al_destroy_bitmap(sword);
    al_destroy_bitmap(shield);
    al_destroy_bitmap(axe);
    al_destroy_bitmap(dopen);  
	al_destroy_sample(dgnMsc);
	al_destroy_sample_instance(dgnInstance);
	al_destroy_sample(btlMsc);
	al_destroy_sample_instance(btlInstance);
	al_destroy_sample(goMsc);
	al_destroy_sample_instance(goInstance);
	al_destroy_display(display);
    al_destroy_event_queue(event_queue);
	al_destroy_timer(timer);
	
	return 0;
}